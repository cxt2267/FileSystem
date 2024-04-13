#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "options.h"

int main(int argc, char**argv) {

    int8_t quit = 0;

    while(quit == 0) {

        char* cmd_str = malloc(MAX_COMMAND_SIZE);

        char* token[MAX_COMMAND_ARGUMENTS];

        int j;
        for(j=0; j<MAX_COMMAND_ARGUMENTS; j++) {
            token[j] = NULL;
        }

        printf("mfs> ");
        fgets(cmd_str,MAX_COMMAND_SIZE,stdin);
        cmd_str = strtok(cmd_str,"\n");

        token[0] = strtok(cmd_str," ");

        if(cmd_str == NULL || token[0] == NULL) {
            printf("please enter a valid command or enter quit to leave.\n");
            continue;
        }

        if(!strcmp(cmd_str,"quit")) {
            quit = 1;
            printf("exiting...\n");
            exit(0);
        }

        int i = 1;
        // getting command line arguments 
        while((token[i] = strtok(NULL," "))) {
            if(i == 5) {
                printf("error: too many command line arguments entered!\n");
                continue;
            }
            i++;
        }

        if(!strcmp(token[0],"createfs")) {
            if(token[1] != NULL) {
                if(token[2] != NULL) {
                    printf("createfs: too many arguments for create command!\n");
                    continue;
                }
                else {
                    if(!create(token[1])) {
                        continue;
                    }
                }
            }
            else {
                printf("createfs: Filename not provided!\n");
                continue;
            }
        }
        else if(!strcmp(token[0],"savefs")) {
            if(token[1] != NULL) {
                printf("savefs: too many arguments for savefs command!\n");
                continue;
            }
            else {
                if(!save()) {
                    continue;
                }
            }
        }
        else if(!strcmp(token[0],"open")) {
            if(token[1] != NULL) {
                if(token[2] != NULL) {
                    printf("open: too many arguments for open command!\n");
                    continue;
                }
                else {
                    if(!strcmp(file_image,token[1])) {
                        printf("%s is already open.\n",file_image);
                        continue;
                    }

                    if(!open(token[1])) {
                        continue;
                    }
                }
            }
            else {
                printf("open: Filename not provided!\n");
                continue;
            }
        }
        else if(!strcmp(token[0],"close")) {
            if(token[1] != NULL) {
                printf("close: too many arguments for close command!\n");
                continue;
            }
            else {
                if(!close()) {
                    continue;
                }
            }
        }
        else if(!strcmp(token[0],"insert")) {
            if(token[1] != NULL) {
                if(strlen(token[1]) > MAX_FILE_NAME) {
                    printf("insert error: File name too long.\n");
                    continue;
                }
                if(token[2] != NULL) {
                    printf("insert: too many arguments for insert command!\n");
                    continue;
                }
                else {
                    if(!insert(token[1])) {
                        continue;
                    }
                }
            }
            else {
                printf("insert: Filename not provided!\n");
                continue;
            }
        }
        else if(!strcmp(token[0],"delete")) {
            if(token[1] != NULL) {
                if(token[2] != NULL) {
                    printf("delete: too many arguments for delete command!\n");
                    continue;
                }
                else {
                    if(!delete_file(token[1])) {
                        continue;
                    }
                }
            }
            else {
                printf("delete: Filename not provided!\n");
                continue;
            }
        }
        else if(!strcmp(token[0],"undel")) {
            if(token[1] != NULL) {
                if(token[2] != NULL) {
                    printf("undelete: too many arguments for undel command!\n");
                    continue;
                }
                else {
                    if(!undelete(token[1])) {
                        continue;
                    }
                }
            }
            else {
                printf("undelete: Filename not provided!\n");
                continue;
            }
        }
        else if(!strcmp(token[0],"df")) {
            if(token[1] != NULL) {
                printf("df: too many arguments for df command!\n");
                continue;
            }
            else {
                int bytes_free = df();
                if(bytes_free == -1) {
                    printf("df: no image is currently open.\n");
                    continue;
                }
                else {
                    printf("%d bytes available for use.\n",df());
                    continue;
                }
            }
        }
        else if(!strcmp(token[0],"retrieve")) {
            if(token[1] == NULL) {
                printf("retrieve: Filename not provided!\n");
                continue;
            }
            else {
                if(token[3] != NULL) {
                    printf("retrieve: too many arguments for retrieve command!\n");
                    continue;
                }
                else {
                    if(!retrieve(token[1],token[2])) {
                        continue;
                    }
                }
            }
        }
        else if(!strcmp(token[0],"read")) {
            if(token[1] == NULL || token[2] == NULL || token[3] == NULL) {
                printf("read: not enough arguments for read command!\n");
                continue;
            }

            if(token[4] != NULL) {
                printf("read: too many arguments for read command!\n");
                continue;
            }

            if(!read(token[1],atoi(token[2]),atoi(token[3]))) {
                continue;
            }
        }
        else if(!strcmp(token[0],"attrib")) {
            if(token[3] != NULL) {
                printf("attrib: too many arguments for attrib command!\n");
                continue;
            }

            if(token[1] == NULL || token[2] == NULL) {
                printf("attrib: not enough arguments for attrib command!\n");
                continue;
            }

            if(!attrib(token[1],token[2])) {
                continue;
            }
        }
        else if(!strcmp(token[0],"list")) {
            if(token[3] != NULL) {
                printf("list: too many arguments for list command!\n");
                continue;
            }
            
            if(!list(token[1],token[2])) {
                continue;
            }
        }
        else if(!strcmp(token[0],"encrypt")) {
            if(token[3] != NULL) {
                printf("encrypt: too many arguments for encrypt command!\n");
                continue;
            }

            if(token[1] == NULL || token[2] == NULL) {
                printf("encrypt: not enough arguments for encrypt command!\n");
                continue;
            }
            
            if(!encrypt_decrypt(token[1],atoi(token[2]))) {
                continue;
            }
        }
        else if(!strcmp(token[0],"decrypt")) {
            if(token[3] != NULL) {
                printf("decrypt: too many arguments for decrypt command!\n");
                continue;
            }
            if(token[1] == NULL || token[2] == NULL) {
                printf("decrypt: not enough arguments for decrypt command!\n");
                continue;
            }
            if(!encrypt_decrypt(token[1],atoi(token[2]))) {
                continue;
            }
        }
        else if(!strcmp(token[0],"help")) {
            if(token[1] != NULL) {
                printf("help: too many arguments for help command!\n");
                continue;
            }
            else {
                help();
                continue;
            }
        }
        else {
            printf("error: invalid command entered -- to see valid commands, enter 'help'.\n");
        }
    }
}
