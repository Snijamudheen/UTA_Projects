#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

#define BLOCK_SIZE 1024
#define NUM_BLOCKS 65536
#define BLOCKS_PER_FILE 1024
#define NUM_FILES 256
#define FIRST_DATA_BLOCK 1001
#define MAX_FILE_SIZE 1048576
#define IS_HIDDEN 2
#define IS_READ_ONLY 1

uint8_t data[NUM_BLOCKS][BLOCK_SIZE];

// 512 blocks just for free block map
uint8_t * free_blocks;
uint8_t * free_inodes;

// directory
struct directoryEntry
{
    char filename[64];
    short in_use;
    int32_t inode;
};

struct directoryEntry * directory;

// inode
struct inode
{
    int32_t blocks[BLOCKS_PER_FILE];
    short in_use;
    uint8_t attribute; // 1 = hidden and 2 = read-only 
    uint32_t file_size;
};

struct inode * inodes;

FILE    *fp;
char    image_name[64];
uint8_t image_open;

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 5 

int32_t findFreeBlock()
{
    int i;

    // If the block is free, return its index plus 1001 since the block numbering starts at 1001
    for(i = 0; i < NUM_BLOCKS; i++)
    {
        if(free_blocks[i])
        {
            return i + 1001;
        }
    }
    return -1;
}

int32_t findFreeInode()
{
    int i;

    // If the current inode is free return 1 to indicate that a free inode was found
    for(i = 0; i < NUM_FILES; i++)
    {
        if(free_inodes[i])
        {
            return 1;
        }
    }
    return -1;
}

// This function finds a free block in an inode of a file
int32_t findFreeInodeBlock(int32_t inode)
{
    int i;

    // If a free block is found, its index is returned.
    // If no free block is found, -1 is returned.
    for(i = 0; i < BLOCKS_PER_FILE; i++)
    {
        if(inodes[inode].blocks[i] == -1)
        {
            return i;
        }
    }
    return -1;
}

// This function initializes the file system by setting up pointers to the various data structures
// and initializing the directory entries, inodes, and free block/inode arrays.
void init()
{
    directory = (struct directoryEntry*)&data[0][0];
    inodes = (struct inode *)&data[20][0];
    free_blocks = (uint8_t *)&data[1000][0];
    free_inodes = (uint8_t *)&data[19][0];

    // Initialize the image name and image open flag
    memset(image_name, 0, 64);
    image_open = 0;

    int i;
    for(i = 0; i < NUM_FILES; i++)
    {
        directory[i].in_use = 0;
        directory[i].inode = -1;
        free_inodes[i] = 1;

        memset(directory[i].filename, 0, 64);

        int j;
        for(j = 0; j < NUM_BLOCKS; j++)
        {
            inodes[i].blocks[j] = -1;
            inodes[i].in_use = 0;
            inodes[i].attribute = 0;
            inodes[i].file_size = 0;
        }
    }

    int j;

    // Set all blocks in the free_blocks array to 1 (i.e. free)
    for(j = 0; j < NUM_BLOCKS; j++)
    {
        free_blocks[j] = 1;
    }
}

// This function calculates the amount of free space in the file system by counting the number of
// free blocks and multiplying that by the block size.
uint32_t df()
{
    int j;
    int count = 0;

    for(j = FIRST_DATA_BLOCK; j < NUM_BLOCKS; j++)
    {
        if(free_blocks[j])
        {
            count ++;
        }
    }
    return count * BLOCK_SIZE;
}

// This function creates a new file system image with the given filename. 
void createfs(char * filename)
{
    // Open the file with the given filename in write mode
    fp = fopen(filename, "w");

    // Check if the file was successfully opened
    if (!fp)
    {
        perror(filename);
        return;
    }

    // Copy the filename into the global variable 'image_name'
    // Initialize all the blocks in the file system to 0
    strncpy(image_name, filename, strlen(filename));
    memset(data, 0, NUM_BLOCKS * BLOCK_SIZE);

    // Mark the file system image as open
    image_open = 1;

    int i;
    for(i = 0; i < NUM_FILES; i++)
    {
        // Initialize the directory entry
        directory[i].in_use = 0;
        directory[i].inode = -1;
        free_inodes[i] = 1;

        // Set the filename to an empty string
        memset(directory[i].filename, 0, 64);

        int j;

        // Initialize the inode block
        for(j = 0; j < NUM_BLOCKS; j++)
        {
            inodes[i].blocks[j] = -1;
            inodes[i].in_use = 0;
            inodes[i].attribute = 0;
            inodes[i].file_size = 0;
        }
    }

    int j;

    // Mark all the blocks as free
    for(j = 0; j < NUM_BLOCKS; j++)
    {
        free_blocks[j] = 1;
    }

    // Close the file
    fclose(fp);
}

void savefs()
{
    // Check if an image is open for writing
    if(image_open == 0)
    {
        printf("ERROR: Disk image is not open\n");
        return;  // Exit the function if the image is not open
    }

    fp = fopen(image_name, "w");    // Open the image file for writing

    // Check if the file was opened successfully
    if(fp == NULL)
    {
        perror(image_name);
        return;
    }

    // Write the data into the image file
    fwrite(&data[0][0], BLOCK_SIZE, NUM_BLOCKS, fp);

    fclose(fp);
}

void openfs(char * filename)
{
    fp = fopen(filename, "r");  // Open the image file for reading

    // Check if the image is open for reading
    if(fp == NULL)
    {
        perror(filename);
        return;
    }
   
    strncpy(image_name, filename, strlen(filename));

    // Read data from the image file
    fread(&data[0][0], BLOCK_SIZE, NUM_BLOCKS, fp);

    // Set the image_open flag to 1 to indicate that the image is open
    image_open = 1;

    fclose(fp);
}

void closefs()
{
    // Check if an image is open
    if(image_open == 0)
    {
        printf("ERROR: Disk image is not open\n");
        return;
    }

    // Set the image_open flag to 0 to indicate that the image is closed
    image_open = 0;

    // Clear the image_name buffer to erase the image file name
    memset(image_name, 0, 64);
}

void list(char **args)
{
    int i;
    int not_found = 1;
    int show_hidden = 0;
    int show_attr = 0;

    // Loop through the command line arguments
    while(*args)
    {
        // If the "-h" is entered, show hidden image
        if(! strcmp(*args, "-h"))
        {
            show_hidden = 1;
        } 
        
        // If the "-a" is entered, show the attributes in the image
        else if (! strcmp(*args, "-a"))
        {
            show_attr = 1;
        }

        args++;
    }

    for(i = 0; i < NUM_FILES; i++)
    {
        // Check if the current directory entry is in use and not hidden 
        if(directory[i].in_use && (show_hidden || !(inodes[directory[i].inode].attribute & IS_HIDDEN)))
        {
            not_found = 0;
            char filename[65];

            // Copy the filename from the directory entry to a local buffer
            memset(filename, 0, 65);
            strncpy(filename, directory[i].filename, strlen(directory[i].filename));

            // If the show_attr flag is set, print the filename and its attribute
            if(show_attr)
            {
                printf("%s %d\n", filename, inodes[directory[i].inode].attribute);
            } 
            
            // Otherwise, just print the filename
            else
            {
                printf("%s\n", filename);
            }
        }
    }
    
    // If no files were found, print an error message
    if(not_found)
    {
        printf("ERROR: No files found.\n");
    }
}

void insert(char * filename)
{
    //verify the filename isn't NULL
    if(filename == NULL)
    {
        printf("ERROR: Filename is NULL\n");
        return;
    }

    // verify the file exists
    struct stat buf;
    int ret = stat(filename, &buf);

    if(ret == -1)
    {
        printf("ERROR: File does not exist\n");
        return;
    }

    // verify the file isn't too big
    if(buf.st_size > MAX_FILE_SIZE)
    {
        printf("ERROR: File is too large\n");
        return;
    }

    // verify there is enough space
    if(buf.st_size > df())
    {
        printf("ERROR: Not enough free disk space\n");
        return;
    }

    // find an empty directory entry 
    int i;
    int directory_entry = -1;

    for(i = 0; i < NUM_FILES; i++)
    {
        if(directory[i].in_use == 0)
        {
            directory_entry = i;
            break;
        }
    }

    if(directory_entry == -1)
    {
        printf("ERROR: Could not find a free directory entry\n.");
        return;
    }

    // Open the input file read-only 
    FILE *ifp = fopen ( filename, "r" ); 

    if (!ifp)
    {
        perror(filename);
        return;
    }
    
    // Print the number of bytes being read from the input file.
    printf("Reading %d bytes from %s\n", (int) buf . st_size, filename);

    
    int32_t copy_size   = buf . st_size; // Get the size of the input file
    int32_t offset      = 0;               
    int32_t block_index = -1;
 
    // find a free inode
    int32_t inode_index = findFreeInode();   

    if(inode_index == -1)
    {
        printf("ERROR: Can not find a free inode");
        return;
    }

    // place the file info in the directory
    directory[directory_entry].in_use = 1;
    directory[directory_entry].inode = inode_index;
    strncpy(directory[directory_entry].filename, filename, strlen(filename));
    inodes[inode_index].file_size = buf.st_size;

    // Copy the input file into the file system one block at a time.
    while( copy_size > 0 )
    {
        // Set the file position to the offset in the input file.
        fseek( ifp, offset, SEEK_SET );

        // find a free block
        block_index = findFreeBlock();

        if(block_index == -1)
        {
            printf("ERROR: Can not find a free block");
            return;
        }

        // Read the block from the input file.
        int32_t bytes  = fread( data[block_index], BLOCK_SIZE, 1, ifp );

        // save the block in the inode
        int32_t inode_block = findFreeInodeBlock(inode_index);
        inodes[inode_index].blocks[inode_block] = block_index;

        // If bytes == 0 and we haven't reached the end of the file then something is 
        // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
        // It means we've reached the end of our input file.
        if( bytes == 0 && !feof( ifp ) )
        {
            printf("ERROR: An error occured reading from the input file.\n");
            return;
        }

        // Clear the EOF file flag.
        clearerr( ifp );

        // Reduce copy_size by the BLOCK_SIZE bytes.
        copy_size -= BLOCK_SIZE;
        
        // Increase the offset into our input file by BLOCK_SIZE.  This will allow
        // the fseek at the top of the loop to position us to the correct spot.
        offset    += BLOCK_SIZE;

        // find another free block if there is still data to copy
        block_index = findFreeBlock();
    }

    // We are done copying from the input file so close it.
    fclose( ifp );
}

// This function takes an array of arguments and retrieves a file from the file system.
void retrieve(char ** args)
{
    // verify the file exists 
    int i;
    int directory_entry = -1;

    for(i = 0; i < NUM_FILES; i++)
    {
        // If we find the file in the directory, we save its index
        if(directory[i].in_use == 1 && ! strcmp(args[0], directory[i].filename))
        {
            directory_entry = i;
            break;
        }
    }

    if(directory_entry == -1)
    {
        // If we couldn't find the file, print an error message and return
        printf("ERROR: Could not find that filename.\n");
        return;
    }

    // If we found the file, get its inode
    int inode = directory[i].inode;

    // Determine the output file name to write to
    char * ofn = args[ args[1] ? 1 : 0];
    
    // Open the output file to write
    FILE *ofp = fopen( ofn, "w" ); 

    // If the output file could not be opened, print an error message and return
    if(!ofp)
    {
        printf("ERROR: Could not open that file to write.\n");
        return;
    }
 
    // Get the file size of the file we are retrieving
    int32_t copy_size = inodes[inode].file_size;

    // Print out the file size and output file name we are writing to
    printf("Writing %d bytes to %s\n", copy_size, ofn);              

    int32_t block_index;
    int32_t inode_index = 0;   

    // Copy the input file into the file system one block at a time.
    while( copy_size > 0 )
    {
        // Get the block index of the current inode
        block_index = inodes[inode].blocks[inode_index++];

        // If the block index is invalid, print an error message and return
        if(block_index == -1)
        {
            printf("ERROR: Found invalid block index.\n");
            return;
        }

        // Determine how much data to write to the output file
        int write_size = copy_size > BLOCK_SIZE ? BLOCK_SIZE : copy_size;

        // Write the data to the output file
        int32_t bytes  = fwrite (data[block_index], write_size, 1, ofp );

        // If an error occured writing to the output file, print an error message and return
        if( bytes == 0 )
        {
            printf("ERROR: An error occured writing to the output file.\n");
            return;
        }

        // Reduce copy_size by the BLOCK_SIZE bytes.
        copy_size -= BLOCK_SIZE;
    }

    // We are done copying from the output file so close it out.
    if (fclose( ofp ))
        perror("Write close");
}

void readfs(char ** args)
{
    // verify the file exists 
    int i;
    int directory_entry = -1;

    // Loop through the directory to find the file
    for(i = 0; i < NUM_FILES; i++)
    {
        // If we find the file in the directory, we save its index
        if(directory[i].in_use == 1 && ! strcmp(args[0], directory[i].filename))
        {
            directory_entry = i;
            break;
        }
    }

    // If we couldn't find the file, print an error message and return
    if(directory_entry == -1)
    {
        printf("ERROR: Could not find that filename.\n");
        return;
    }

    // Get the inode for the file
    int inode = directory[i].inode;
 
    int32_t copy_size, start;
    int32_t file_size = inodes[inode].file_size;

    // Parse the starting byte and number of bytes to read from the command line arguments
    if (1 != sscanf(args[1], "%d", &start) || start < 0 || start >= file_size)
    {
        printf("Invalid starting byte '%s' for file size %d.\n", args[1], file_size);
        return;
    } 
    
    if (1 != sscanf(args[2], "%d", &copy_size) || copy_size < 1 || start + copy_size > file_size)
    {
        printf("Invalid number of bytes '%s' for file size %d.\n", args[2], file_size);
        return;
    }            

    int32_t block_index;
    int32_t inode_index = 0, byte_index = BLOCK_SIZE;
    copy_size += start;

    // Loop through the blocks of the file to read the requested bytes
    while( copy_size > 0 )
    {
        // If we've read all the bytes we need from this block, move to the next block
        if (byte_index == BLOCK_SIZE)
        {
            block_index = inodes[inode].blocks[inode_index++];

            if (block_index == -1)
            {
                printf("ERROR: Found invalid block index.\n");
                return;
            }

            byte_index = 0;
        }

        // If we've reached the starting byte, print the current byte to the console
        if (start-- <= 0)
        {
            printf("%.2x", data[block_index][byte_index]);
        }
        
        byte_index++;
        copy_size--;
    }

    printf("\n");
}

// This function is used to delete a file from the file system.
// It takes a filename as an argument and searches for the file in the directory.
// If the file is found, it is marked as not in use in the directory and is effectively deleted.
// If the file is not found, an error message is printed and the function returns.
void delete(char * filename)
{
    // verify the file exists 
    int i;
    int directory_entry = -1;

    for(i = 0; i < NUM_FILES; i++)
    {
        if(directory[i].in_use == 1 && ! strcmp(filename, directory[i].filename))
        {
            directory_entry = i;
            break;
        }
    }

    if(directory_entry == -1)
    {
        printf("ERROR: Could not find that filename.\n");
        return;
    }

    directory[i].in_use = 0;
}

// This function attempts to restore a previously deleted file 
// by searching the directory for a file with the specified filename
// If such a file is found, the function sets its in_use flag to 1 to mark it as undeleted.
void undel(char * filename)
{
    // verify the file exists 
    int i;
    int directory_entry = -1;

    for(i = 0; i < NUM_FILES; i++)
    {
        if(directory[i].in_use == 0 && ! strcmp(filename, directory[i].filename))
        {
            directory_entry = i;
            break;
        }
    }

    if(directory_entry == -1)
    {
        printf("ERROR: Could not find that filename.\n");
        return;
    }

    directory[i].in_use = 1;
}

void attrib(char ** args)
{
    // Set the filename based on the second argument or, if not provided, the first argument
    char * filename = args[2] ? args[2] : args[1];

    // Initialize flags for each attribute
    int hon = 0, hoff = 0, ron = 0, roff = 0;

    if (!strcmp(args[0],"+h"))        // If the first argument is "+h", then make the file hidden
    {
        hon = 1;
    } 
    
    else if (!strcmp(args[0], "-h")) // If the first argument is "-h", then make the file non-hidden
    {
        hoff = 1;
    } 
    
    else if (!strcmp(args[0], "+r")) // If the first argument is "+r", make the file read-only
    {
        ron = 1;
    } 
    
    else if (!strcmp(args[0], "-r")) // If the first argument is "-r", make the file non read-only
    {
        roff = 1;
    }

    if (args[2])
    {
        if (!strcmp(args[1],"+h"))  // If the second argument is "+h", then make the file hidden
        {
            hon = 1;
        } 
        
        else if (!strcmp(args[1], "-h")) // If the second argument is "-h", then make the file non-hidden
        {
            hoff = 1;
        } 
        
        else if (!strcmp(args[1], "+r")) // If the second argument is "+r", make the file read-only
        {
            ron = 1;
        } 
        
        else if (!strcmp(args[1], "-r")) // If the second argument is "-r", make the file non read-only
        {
            roff = 1;
        }
    }

    // verify the file exists 
    int i;
    int directory_entry = -1;

    for(i = 0; i < NUM_FILES; i++)
    {
        if(directory[i].in_use == 1 && ! strcmp(filename, directory[i].filename))
        {
            directory_entry = i;
            break;
        }
    }

    // If the file was not found, print an error message and return
    if(directory_entry == -1)
    {
        printf("ERROR: Could not find that filename.\n");
        return;
    }

    // Get the inode for the file
    int inode = directory[i].inode;

    // Set or unset attribute flags for the inode based on the "hon", "hoff", "ron", and "roff" flags
    inodes[inode].attribute |= (hon * IS_HIDDEN) | (ron * IS_READ_ONLY);
    inodes[inode].attribute &= ~(hoff * IS_HIDDEN) & ~(roff * IS_READ_ONLY);
}

void encrypt(char ** args)
{
    // verify the file exists 
    int i;
    int directory_entry = -1;

    // Search for the file in the directory
    for(i = 0; i < NUM_FILES; i++)
    {
        if(directory[i].in_use == 1 && ! strcmp(args[0], directory[i].filename))
        {
            directory_entry = i;
            break;
        }
    }

    // If the file is not found, display an error message and return
    if(directory_entry == -1)
    {
        printf("ERROR: Could not find that filename.\n");
        return;
    }

    // Get the inode number for the file
    int inode = directory[i].inode;
 
    int32_t copy_size = inodes[inode].file_size;
    int32_t block_index;
    int32_t inode_index = 0, byte_index = BLOCK_SIZE;

    // Iterate over the blocks in the file, XORing each byte with the encryption key
    while( copy_size > 0 )
    {
        // If we've processed all bytes in the current block, move to the next block
        if (byte_index == BLOCK_SIZE)
        {
            block_index = inodes[inode].blocks[inode_index++];

            // If the block index is invalid, display an error message and return
            if (block_index == -1)
            {
                printf("ERROR: Found invalid block index.\n");
                return;
            }

            // Reset the byte index for the new block
            byte_index = 0;
        }

        // XOR the byte with the encryption key
        data[block_index][byte_index] ^= args[1][0];
        byte_index++;
        copy_size--;
    }
}

int main()
{
    char * command_string = (char*) malloc(MAX_COMMAND_SIZE);
    fp = NULL;

    init();

    while(1)
    {
        // Print out the mfs prompt
        printf( "mfs> " );

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while( !fgets ( command_string, MAX_COMMAND_SIZE, stdin ) );

        // Parse input 
        char *token[MAX_NUM_ARGUMENTS];

        for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
        {
            token[i] = NULL;
        }

        int token_count = 0;                                 
                                                            
        // Pointer to point to the token
        // parsed by strsep
        char *argument_ptr = NULL;                                         
                                                            
        char *working_string  = strdup( command_string );                

        // we are going to move the working_string pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *head_ptr = working_string;

        // Tokenize the input strings with whitespace used as the delimiter
        while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && (token_count < MAX_NUM_ARGUMENTS))
        {
            if( strlen(argument_ptr) == 0 )
            {
                token[token_count] = NULL;
                break;
            }

            token[token_count++] = strndup( argument_ptr, MAX_COMMAND_SIZE );
        }

        // process the filesystem commands
        if (strcmp("createfs", token[0]) == 0)
        {
            if(token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
            }
            
            else
            {
                createfs(token[1]);  // create a new file system with the specified filename
            }
        } 
        
        else if (strcmp("savefs", token[0]) == 0)
        {
            savefs(); // Save the current file system
        } 
        
        else if (strcmp("open", token[0]) == 0)
        {  
            if (token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
            } 
            
            else {    
                openfs(token[1]);   // Open the specified file system
            }
        } 
        
        else if (strcmp("close", token[0]) == 0)
        {
            closefs();  // Close the current file system
        } 
        
        else if (strcmp("list", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            
            else
                list(token + 1);  // List the contents of the current directory
        }  
        
        else if (strcmp("df", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            else
                printf("%d bytes free\n", df());  // Display the amount of free space in the file system
        }  
        
        else if (strcmp("quit", token[0]) == 0)
        {
            exit(0);  // Exit the program
        } 
        
        else if(strcmp("insert", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            
            else if (token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
            } 
            
            else  
                insert(token[1]);    // Insert the specified file into the file system        
        } 
        
        else if (strcmp("retrieve", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            
            else if (token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");

            } 
            
            else  
                retrieve(token + 1);   // call the function to retrieve a file from the file system            
        } 
        
        else if (strcmp("read", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            
            else if (token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
            } 
            
            else if (token[2] == NULL)
            {
                printf("ERROR: No starting byte specified\n");
            } 
            
            else if (token[3] == NULL)
            {
                printf("ERROR: No number of bytes specified\n");
            } 
            
            else  
                readfs(token + 1);   // call the function to read a certain number of bytes from a file in the file system                   
        } 
        
        else if (strcmp("delete", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            
            else if (token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
            } 
            
            else  
                delete(token[1]);   // call delete function with the filename passed as argument
        } 
        
        else if (strcmp("undel", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            
            else if(token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
            } 
            
            else  
                undel(token[1]);   // call undel function with the filename passed as argument
        } 
        
        else if (strcmp("attrib", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            
            else if (token[1] == NULL)
            {
                printf("ERROR: No attribute or filename specified\n");
            } 
            
            else if (token[2] == NULL)
            {
                printf("ERROR: No filename specified\n");
            } 
            
            else  
                attrib(token + 1);    // call attrib function with the attributes and filename passed as argument
        } 
        
        else if (strcmp("encrypt", token[0]) == 0 || strcmp("decrypt", token[0]) == 0)
        {
            if (!image_open)
            {
                printf("ERROR: Disk image is not opened.\n");
            } 
            
            else if (token[1] == NULL)
            {
                printf("ERROR: No filename specified\n");
            } 
            
            else if (token[2] == NULL)
            {
                printf("ERROR: No cipher specified\n");
            } 
            
            else  
                encrypt(token + 1);    // call encrypt function with the filename and cipher passed as argument       
        } 
        
        else 
        {
            // if the command is not recognized, print error message
            printf("Unknown command: %s\n", token[0]); 
        }

        // Cleanup allocated memory
        for ( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
        {
            if ( token[i] != NULL )
            {
                free( token[i] );
                token[i] = NULL;
            }
        }

        free( head_ptr ); // free memory for the linked list of deleted files
    }

    free( command_string );  // free memory for the command string

    return 0;
}