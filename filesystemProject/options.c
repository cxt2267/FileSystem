#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include "options.h"

void init() {
    // setting up where data structures are going to be located
    dir = (struct dir_entry*)&data[0][0];
    free_node = (struct free_inode*)&data[19][0];
    node = (struct inode*)&data[20][0]; 
    free_blk = (struct free_block*)&data[1157][0]; 
    blk = (struct block*)&data[1415][0];

    // initializing all values to avoid seg faults
    int i;
    for(i=0;i<MAX_FILE_COUNT;i++) {
        
        dir[i].in_use = 0;
        dir[i].inode = -1;
        free_node[i].inode = i;
        int k;
        for(k=0; k<MAX_FILE_NAME; k++) {
            dir[i].filename[k] = 0;
        }

        node[i].in_use = 0;
        node[i].attribute = 0;
        node[i].filesize = -1;
        memset(node[i].time,0,32);

        int j;
        for(j=0; j<MAX_BLOCKS_PER_FILE; j++) {
            node[i].blocks[j] = -1;
        }
    }

    int32_t m;
    for(m=0; m<DATA_BLOCKS; m++) {
        free_blk[m].block = m;
    }

    memset(blk,0,DATA_BLOCKS*BLOCK_SIZE);

    opened = 0;
}

int create(char* image_file) {
    // verifying that an image file is not already open before creating another one
    if(opened) {
        printf("open: an image is already open. ");
        printf("Please close image before opening or creating another one!\n");
        return 0;
    }

    // making sure the file does not already exist in current directory
    if((fp = fopen(image_file,"r"))) {
        printf("createfs: image not created -- file already exists in directory!\n");
        fclose(fp);
        return 0;
    }

    if(!(fp = fopen(image_file,"w+"))) {
        printf("createfs: could not create image.\n");
        return 0;
    }

    // setting the file's name as a global so data can be saved to it
    strcpy(file_image, image_file);

    // initializing data block every time an image is created
    init();

    fclose(fp);
    opened = 1;

    return 1;
}

int save() {
    if(!opened) {
        printf("save: no image currently opened to save data.\n");
        return 0;
    }

    if(!(fp = fopen(file_image,"r+"))) {
        printf("save: could not open image for writing!\n");
        return 0;
    }

    // transfering the data from the data block to the image file
    fwrite(&data[0][0],BLOCK_SIZE,BLOCK_COUNT,fp);

    fclose(fp);
    return 1;
}

int open(char* image_file) {
    if(opened) {
        printf("open: an image is already open. ");
        printf("Please close image before opening or creating another one!\n");
        return 0;
    }

    // making sure image file exists in directory
    if(!(fp = fopen(image_file,"r"))) {
        printf("open: File not found!\n");
        return 0;
    }
    else {
        fclose(fp);
    }

    if(!(fp = fopen(image_file,"r+"))) {
        printf("open: could not read from image!\n");
        return 0;
    }

    strcpy(file_image, image_file);

    // making sure to initialize data to avoid seg faults
    init();

    // getting the image file's data and storing it on the data block for future manipulation 
    fread(&data[0][0],BLOCK_SIZE,BLOCK_COUNT,fp);

    fclose(fp);
    opened = 1;
    return 1;
}

int close() {
    if(!opened) {
        printf("close: File not open!\n");
        return 0;
    }

    // making sure the file image's name is no longer globally accessible
    memset(file_image,0,MAX_FILE_NAME);
    opened = 0;
    return 1;
}

int* find_free_blks(int blks_needed) {
    int arr[blks_needed];

    int j = 0;
    int i;

    // getting the data blocks that will be used by the new file
    for(i = 0; i < DATA_BLOCKS; i++) {
        if(j == blks_needed) {
            break;
        }
        if(free_blk[i].block != -1) {
            arr[j] = free_blk[i].block;
            free_blk[i].block = -1;
            j++;
        }
    }

    // putting blocks back if there are not enough to fit the new file's size
    if(j < blks_needed) {
        int m = 0;
        int k;
        for(k = 0; k < DATA_BLOCKS; k++) {
            if(m == j) {
                break;
            }
            if(free_blk[k].block == -1) {
                free_blk[k].block = arr[m];
                m++;
            }
        }
        return NULL;
    }

    // storing available blocks in global variable so they can be used by insert() function
    memcpy(needed_blks,arr,blks_needed*sizeof(int32_t));

    int* blks = arr; 

    return blks;
}

int find_free_inode() {
    int i;

    // finding an inode that is available for the new file to use
    for(i=0;i<MAX_FILE_COUNT;i++) {
        if(free_node[i].inode != -1) {
            int free_nd = free_node[i].inode;
            free_node[i].inode = -1;
            return free_nd;
        }
    }

    return -1;
}

int insert(char* filename) {
    if(!opened) {
        printf("insert: no image currently open!\n");
        return 0;
    } 

    if(!(fp = fopen(filename,"r"))) {
        printf("insert: File not found!\n");
        return 0;
    }
    else {
        fclose(fp);
    }

    if(!(fp = fopen(filename,"r+"))) {
        printf("insert: could not read from file!\n");
        return 0;
    }

    // getting the file's size
    fseek(fp,0L,SEEK_END);
    long int size = ftell(fp);
    fclose(fp);

    // verifying that the size is within bounds
    if(size > MAX_FILE_SIZE) {
        printf("insert error: file is too large! Must be less than 2^20 bytes.\n");
        return 0;
    }

    // verifying that the size is within bounds
    if(size > df()) {
        printf("insert error: Not enough disk space!\n");
        return 0;
    }

    if(!(fp = fopen(filename,"r+"))) {
        printf("insert: could not read from file!\n");
        return 0;
    }

    // finding the amount of blocks needed to store the new file
    int blks_needed;
    if(size % 1024 == 0) {
        if(size == 0) {
            blks_needed = 1;
        }
        else {
            blks_needed = (float)size/(float)1024;
        }
    }
    else {
        blks_needed = ((float)size/(float)1024) + 1;
    }

    int i;
    for(i=0;i<MAX_FILE_COUNT;i++) {

        //making sure directory entry is free to use
        if(dir[i].in_use == 0) {
            int* blks = find_free_blks(blks_needed);

            if(blks == NULL) {
                printf("insert error: not enough data blocks available to insert data!\n");
                return 0;
            }

            int num_blocks[blks_needed];
            memcpy(num_blocks,needed_blks,sizeof(int32_t)*blks_needed);

            int free_nd = find_free_inode();
            if(free_nd == -1) {
                printf("insert error: no inodes available for the file!\n");
                int m;
                int k = 0;
                for(m=0; m<DATA_BLOCKS; m++) {
                    if(k == blks_needed) {
                        break;
                    }

                    // putting blocks back if no inode is free
                    if(free_blk[m].block == -1) {
                        free_blk[m].block = num_blocks[k];
                        k++;
                    }
                }
                return 0;
            }

            // setting the file's information into its directory entry and inode

            strcpy(dir[i].filename,filename);
            dir[i].in_use = 1;
            dir[i].inode = free_nd;
            
            node[free_nd].in_use = 1;
            node[free_nd].filesize = size;

            int j;
            for(j=0;j<blks_needed;j++) {
                node[free_nd].blocks[j] = num_blocks[j];
                fseek(fp,BLOCK_SIZE*j,SEEK_SET);
                // storing the file's data into data blocks
                fread(&data[1415 + num_blocks[j]][0],BLOCK_SIZE,1,fp);
            }

            time_t time_added;
            time(&time_added);

            // recording time that file was added
            strcpy(node[free_nd].time,ctime(&time_added));

            break;
        }
    }

    fclose(fp);
    return 1;
}

int delete_file(char* filename) {
    if(!opened) {
        printf("delete: no image is currently open.\n");
        return 0;
    }

    int i;
    int found = 0;
    for(i=0;i<MAX_FILE_COUNT;i++) {
        if(!strcmp(dir[i].filename,filename) && (dir[i].in_use == 1)) {

            // setting the in_use bits to 0 so that file can no longer be accessed
            dir[i].in_use = 0;
            node[dir[i].inode].in_use = 0;

            int j;
            int k = 0;

            // giving the file's blocks back to the free_block map so they can be used
            for(j=0; j<DATA_BLOCKS; j++) {
                if(k == MAX_BLOCKS_PER_FILE) {
                    break;
                }
                if(free_blk[j].block == -1) {
                    free_blk[j].block = node[dir[i].inode].blocks[k];
                    k++;
                }
            }

            found = 1;
            break;
        }
    }

    if(found == 0) {
        printf("Error: File not found.\n");
        return 0;
    }

    return 1;
}

int undelete(char* filename) {
    if(!opened) {
        printf("undelete: no image is currently open.\n");
        return 0;
    }

    int i;
    int found = 0;
    for(i=0;i<MAX_FILE_COUNT;i++) {
        if(!strcmp(dir[i].filename,filename) && dir[i].in_use == 0) {

            // if the file's directory entry has not been taken, 
            // setting its in_use bits to 1 so that it can be accessed
            dir[i].in_use = 1;
            node[dir[i].inode].in_use = 1;

            int k;

            // reclaiming its blocks that have not yet been taken
            for(k=0; k<MAX_BLOCKS_PER_FILE; k++) {
                int j;
                for(j=0; j<DATA_BLOCKS; j++) {
                    if(free_blk[j].block == node[dir[i].inode].blocks[k]) {
                        free_blk[j].block = -1;
                    }
                }
            }
            found = 1;
            break;
        }

        if(!strcmp(dir[i].filename,filename) && dir[i].in_use == 1) {
            printf("File is currently in directory. No need to undelete.\n");
            return 0;
        }
    }

    if(found == 0) {
        printf("undelete: Can not find the file.\n");
        return 0;
    }
    return 1;
}

int df() {
    if(!opened) {
        return -1;
    }

    int counter = 0;
    int i;

    // counting free blocks to find disk space
    for(i=0; i<DATA_BLOCKS; i++) {
        if(free_blk[i].block != -1) {
            counter++;
        }
    }

    // calculating disk space by multiplying the free blocks and the block size
    return counter*BLOCK_SIZE;
}

int retrieve(char* filename, char* newfilename) {
    if(!opened) {
        printf("retrieve: no image is currently open.\n");
        return 0;
    }

    char named_file[MAX_FILE_NAME];

    // specifying whether old filename or new filename will be used to put file into current directory
    if(newfilename == NULL) {
        strcpy(named_file,filename);
    }
    else {
        strcpy(named_file,newfilename);
    }

    int i;
    int found = 0;
    for(i=0; i<MAX_FILE_COUNT; i++) {
        if(!strcmp(dir[i].filename,filename) && dir[i].in_use == 1) {

            if(!(fp = fopen(named_file,"w"))) {
                printf("retrieve: could not create new file!\n");
                return 0;
            }

            int j = 0;
            while(node[dir[i].inode].blocks[j] != -1) {
                if(j == MAX_BLOCKS_PER_FILE) {
                    break;
                }
                fseek(fp,BLOCK_SIZE*j,SEEK_SET);
                // putting file data from its blocks into the output file
                fwrite(&data[1415 + node[dir[i].inode].blocks[j]][0],BLOCK_SIZE,1,fp);
                j++;
            }
            
            found = 1;
            break;
        }
    }

    if(found == 0) {
        printf("Error: File not found!\n");
        return 0;
    }

    fclose(fp);
    return 1;
}

int read(char* filename, int start, int num_bytes) {
    if(!opened) {
        printf("read: no image is currently open.\n");
        return 0;
    }

    // establishing the block to start reading from 
    // and establishing where in the block to start reading from
    int start_blk = start/1024;
    int start_pt = start % 1024;

    int i;
    int found = 0;
    for(i=0; i<MAX_FILE_COUNT; i++) {
        if(!strcmp(dir[i].filename,filename) && dir[i].in_use == 1) {
            if(node[dir[i].inode].in_use == 0) {
                continue;
            }

            if(start > node[dir[i].inode].filesize) {
                printf("read: starting byte is greater than filesize!\n");
                return 0;
            }

            if(num_bytes > node[dir[i].inode].filesize - start) {
                printf("read: cannot read the requested number of bytes from starting byte!\n"
                    "bytes exceed file.\n");
                return 0;
            }

            if(num_bytes <= (1024 - start_pt)) {
                char data_str[1024];
                // reading file data into string that will be outputed in hex format
                memcpy(data_str,&data[1415 + node[dir[i].inode].blocks[start_blk]][start_pt],num_bytes);
                for(int a = 0; a < num_bytes; a++) {
                    printf("%.2x",data_str[a]);
                }
                printf("\n");
                found = 1;
                break;
            }

            char data_str_blk[1024];
            // reading file data into string that will be outputed in hex format
            memcpy(data_str_blk,&data[1415 + node[dir[i].inode].blocks[start_blk]][start_pt],1024 - start_pt);
            for(int b = 0; b < 1024 - start_pt; b++) {
                printf("%.2x",data_str_blk[b]);
            }

            num_bytes -= (1024 - start_pt);

            int num_blks = num_bytes/1024;
            int rem = num_bytes % 1024;

            int j = start_blk + 1;
            while(j<(num_blks + start_blk + 1)) {
                char str_data[1024];
                // reading file data into string that will be outputed in hex format
                memcpy(str_data,&data[1415 + node[dir[i].inode].blocks[j]][0],1024);
                for(int c = 0; c < 1024; c++) {
                    printf("%.2x",str_data[c]);
                }
                j++;
            }

            if(rem != 0) {
                char data_blk_str[1024];
                // reading file data into string that will be outputed in hex format
                memcpy(data_blk_str,&data[1415 + node[dir[i].inode].blocks[j]][0],rem);
                for(int d = 0; d < rem; d++) {
                    printf("%.2x",data_blk_str[d]);
                }
            }

            printf("\n");

            found = 1;
            break;
        }
    }

    if(found == 0) {
        printf("Error: File not found!\n");
        return 0;
    }

    return 1;
}

int attrib(char* attribute, char* filename) {
    if(!opened) {
        printf("attrib: no image is currently open.\n");
        return 0;
    }

    // verifying that second command line argument is valid for adding or removing attributes
    if( strcmp(attribute,"+h") && 
        strcmp(attribute,"-h") && 
        strcmp(attribute,"+r") && 
        strcmp(attribute,"-r")) {
        printf("attrib: to set attribute, enter +h, -h, +r, or -r as second argument before filename.\n");
        return 0;
    }

    char action = attribute[0];
    char attr = attribute[1];

    int i;
    int found = 0;
    for(i=0; i<MAX_FILE_COUNT; i++) {
        if(!strcmp(dir[i].filename,filename) && dir[i].in_use == 1) {

            // adding or removing attributes depending on the attribute and action specified from the user
            if(action == '+') {
                if(attr == 'h') {
                    node[dir[i].inode].attribute |= HIDDEN;
                }
                if(attr == 'r') {
                    node[dir[i].inode].attribute |= READONLY;
                }
            }
            if(action == '-') {
                if(attr == 'h') {
                    node[dir[i].inode].attribute = 0;
                }
                if(attr == 'r') {
                    node[dir[i].inode].attribute = 0;
                }
            }

            found = 1;
            break;
        }
    }

    if(found == 0) {
        printf("attrib: File not found!\n");
        return 0;
    }

    return 1;
}

int list(char* feature1, char* feature2) {
    if(!opened) {
        printf("list: no image is currently open.\n");
        return 0;
    }

    // verifying that command line input is valid for listing files
    if(feature1 != NULL) {
        if(strcmp(feature1,"-h") && strcmp(feature1,"-a")) {
            printf("list: to view hidden files, enter -h. to view file attributes, enter -a.\n");
            return 0;
        }
    }

    // verifying that command line input is valid for listing files
    if(feature2 != NULL) {
        if(strcmp(feature2,"-h") && strcmp(feature2,"-a")) {
            printf("list: to view hidden files, enter -h. to view file attributes, enter -a.\n");
            return 0;
        }
    }

    int i;
    int count = 0;
    for(i=0; i<MAX_FILE_COUNT; i++) {
        if(dir[i].in_use == 1) {
            if(node[dir[i].inode].attribute & READONLY || node[dir[i].inode].attribute == 0) {
                printf("%s    ",dir[i].filename);
                printf("%d    ",node[dir[i].inode].filesize);
                printf("%s    ",node[dir[i].inode].time);

                if(((feature1 != NULL) && !strcmp(feature1,"-a")) || 
                ((feature2 != NULL) && !strcmp(feature2,"-a"))) {
                    // listing attribute for read-only files when prompted
                    if(node[dir[i].inode].attribute & READONLY) {
                        printf("00000010");
                        printf("\n");
                    }
                }
                printf("\n");
            }

            // listing hidden files when prompted
            if(((feature1 != NULL) && !strcmp(feature1,"-h")) || 
                ((feature2 != NULL) && !strcmp(feature2,"-h"))) {
                if(node[dir[i].inode].attribute & HIDDEN) {
                    printf("%s    ",dir[i].filename);
                    printf("%d    ",node[dir[i].inode].filesize);
                    printf("%s    ",node[dir[i].inode].time);
                    // listing attribute for hidden files when prompted
                    if(((feature1 != NULL) && !strcmp(feature1,"-a")) || 
                        ((feature2 != NULL) && !strcmp(feature2,"-a"))) {
                        printf("00000001");
                        printf("\n");
                    }
                    printf("\n");
                }
            }

            count++;
        }
    }

    if(count == 0) {
        printf("list: No files found.\n");
        return 0;
    }

    return 1;
}

int encrypt_decrypt(char* filename, int cipher) {
    if(!opened) {
        printf("encrypt: no image is currently open.\n");
        return 0;
    }

    // making sure the cipher is within valid bounds
    if(cipher < 1 || cipher > 256) {
        printf("encrypt: cipher needs to be between 1 and 256!\n");
        return 0;
    }
    
    int i;
    int found = 0;
    for(i=0; i<MAX_FILE_COUNT; i++) {
        if(!strcmp(dir[i].filename,filename) && dir[i].in_use == 1) {
            int j;
            for(j=0; j<MAX_BLOCKS_PER_FILE; j++) {
                if(node[dir[i].inode].blocks[j] <= -1) {
                    break;
                }

                char* byte = (char*)&data[1415 + node[dir[i].inode].blocks[j]][0];
                int k;
                // using the cipher to encrypt/decrypt every byte in file
                for(k=0; k < BLOCK_SIZE; k++) {
                    if(byte[k] == '\0' ){
                        byte[k] = ' ';
                    }
                    byte[k] = byte[k]^cipher;
                }
            }

            found = 1;
            break;
        }
    }

    if(found == 0) {
        printf("encrypt: File not found!\n");
        return 0;
    }

    return 1;
}

void help() {
    printf("valid commands:\n"
           "insert <filename>\n"
           "retrieve <filename>\n"
           "retrieve <filename> <newfilename>\n"
           "read <filename> <starting byte> <number of bytes>\n"
           "delete <filename>\n"
           "undelete <filename>\n"
           "list [-h] [-a]\n"
           "df\n"
           "open <filename>\n"
           "close\n"
           "createfs <filename>\n"
           "savefs\n"
           "attrib [+attribute] [-attribute] <filename>\n"
           "encrypt <filename> <cipher>\n"
           "encrypt <filename> <cipher>\n"
           "quit\n");
}