#include <stdio.h>
#include <stdint.h>
#define BLOCK_SIZE 1024
#define BLOCK_COUNT 65536
#define MAX_FILE_SIZE 1048576
#define MAX_FILE_COUNT 256
#define MAX_FILE_NAME 64
#define MAX_BLOCKS_PER_FILE 1024
#define DATA_BLOCKS 64121
#define HIDDEN 0x1
#define READONLY 0x2

#define MAX_COMMAND_SIZE 255
#define MAX_COMMAND_ARGUMENTS 5

uint8_t data[BLOCK_COUNT][BLOCK_SIZE];

struct dir_entry {
    char filename[MAX_FILE_NAME];
    int8_t in_use; 
    int32_t inode;
};

struct dir_entry* dir;

struct inode {
    int32_t blocks[MAX_BLOCKS_PER_FILE];
    int8_t in_use;
    uint16_t attribute;
    int32_t filesize;
    char time[32];
};

struct inode* node;

struct free_inode {
    int16_t inode;
};

struct free_inode* free_node;  

struct free_block {
    int32_t block;
};

struct free_block* free_blk;

struct block {
    uint8_t bytes[1024];
};

struct block* blk;

FILE* fp;
char file_image[MAX_FILE_NAME];
int opened;
int needed_blks[1024];

// initializes the data block and establishes the organization of structures and data
void init();

// creates a file using the filename and calls init() to initialize the data block
// returns 1 if successful and 0 if unsuccessful 
int create(char* image_file);

// saves the changes made on the data block to the disk image file
// returns 1 if successful and 0 if unsuccessful
int save();

// initializes data block and opens a disk image using the filename
// retrieves data from disk image to store in data block for manipulation
// returns 1 if successful and 0 if unsuccessful
int open(char* image_file);

// closes the disk image
// no operations can be done when disk image is closed
// returns 1 if successful and 0 if unsuccessful
int close();

// finds free blocks for for a new file to be inserted 
// depending on the blocks needed to fit the file's size
// returns a pointer to an array of available blocks
int* find_free_blks(int blks_needed);

// finds a free inode for a new file to be inserted
// used by the insert() function
// returns 1 if successful and 0 if unsuccessful
int find_free_inode();

// inserts a file in the data block using the filename
// file must be less than max file size and fit within remaining disk space
// returns 1 if successful and 0 if unsuccessful
int insert(char* filename);

// deletes a file using the filename
// directory and inode information not removed
// returns 1 if successful and 0 if unsuccessful
int delete_file(char* filename);

// undeletes a previously deleted file by using the filename
// file is only retrievable if its directory entry was not taken
// some of the file data could still be lost
// returns 1 if successful and 0 if unsuccessful
int undelete(char* filename);

// outputs the amount of disk space available in the data block
// returns 1 if successful and 0 if unsuccessful
int df();

// retrieves and copies a file into the current directory using the filename
// if a new filename is entered, it uses that name for the file
// returns 1 if successful and 0 if unsuccessful
int retrieve(char* filename, char* newfilename);

// reads a specified number of bytes from a file from a starting byte using the filename
// outputs in hexadecimal
// returns 1 if successful and 0 if unsuccessful
int read(char* filename, int start, int num_bytes);

// sets or removes an attribute to a file using the filename
// returns 1 if successful and 0 if unsuccessful
int attrib(char* attribute, char* filename);

// lists the files in the directory according to features entered on command line
// could list hidden files and attributes along with filenames
// returns 1 if successful and 0 if unsuccessful
int list(char* feature1, char* feature2);

// uses a cipher to encrypt or decrypt a file 
// returns 1 if successful and 0 if unsuccessful
int encrypt_decrypt(char* filename, int cipher);

// a help menu that lists possible commands
void help();


