#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/stat.h>

// Parameters of the cameras and their command pipes
#define CAMERA_MAX_NO      5
#define CAMERA_PIPE_PREFIX "/tmp/camera_"
#define CAMERA_PIPE_SUFFIX "000"
#define CAMERA_PIPE_MAXCMD 128

// Commands available
void sync_command()   { sync(); puts("Syncing all filesystems... Done."); fflush(stdout); }
void blowup_command() { puts("Master asked us to blow up to avoid killing citizens. BOOM"); fflush(stdout); exit(1); }
void joke_command()   { puts("Trump for president!"); fflush(stdout); }
void help_command();
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define COMMAND(x) { #x, x ## _command }
struct command {
    char *name;
    void (*function)(void);
};
struct command commands[] = {
    COMMAND(sync),
    COMMAND(blowup),
    COMMAND(joke),
    COMMAND(help),
};
void help_command()   {
    int i;
    puts("Available commands:");
    for(i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
        puts(commands[i].name);
    }
    fflush(stdout);
}


// Command-line params usage for this daemon
void usage(char *argv[])
{
    fprintf(
        stderr,
        "Usage: %s <camera_id>\nwhere <camera_id> can be 1,2,..." STR(CAMERA_MAX_NO) "\n",
        argv[0]);
    exit(1);
}

int main(int argc, char *argv[])
{
    int camera_id;
    FILE *fpPipe;

    if (argc != 2)
        usage(argv);
    camera_id = atoi(argv[1]);
    if (camera_id < 1 || camera_id > CAMERA_MAX_NO)
        usage(argv);

    char pipeFilename[] = CAMERA_PIPE_PREFIX CAMERA_PIPE_SUFFIX;
    snprintf(
        &pipeFilename[sizeof(CAMERA_PIPE_PREFIX)]-1,
        sizeof(pipeFilename)-sizeof(CAMERA_PIPE_SUFFIX),
        "%03d",
        atoi(argv[1]));
    if (0 != mknod(pipeFilename, S_IFIFO|0666, 0)) {
        unlink(pipeFilename);
        if (0 != mknod(pipeFilename, S_IFIFO|0666, 0)) {
            fprintf(stderr, "Failed to create pipe \"%s\"...\n", pipeFilename);
            exit(1);
        }
    }

    printf("Receiving commands for camera %d from %s.\n\nWaiting for command...\n\n", camera_id, pipeFilename);
    while(1) {
        fpPipe = fopen(pipeFilename, "r");
        while(!feof(fpPipe)) {
            char cmd[CAMERA_PIPE_MAXCMD];
            if (fgets(cmd, sizeof(cmd), fpPipe) != NULL) {
                int i;
                int found = 0;
                time_t timeStamp = time(NULL);
                cmd[strlen(cmd)-1] = 0;
                printf("\nReceived command: '%s' at timestamp %s", cmd, ctime(&timeStamp));
                for(i=0; i<sizeof(commands)/sizeof(commands[0]); i++) {
                    if (!strcmp(commands[i].name, cmd)) {
                        found = 1;
                        (*commands[i].function)();
                    }
                }
                if (!found) {
                    printf("Telecommand '%s' is not understood...\n", cmd);
                }
                fflush(stdout);
            } else
                break;
        }
        fclose(fpPipe);
        sleep(1);
    }
    return 0;
}
