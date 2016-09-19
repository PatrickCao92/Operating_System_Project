Project6 Designing Your Mini File System

Student: Shuaiqi Cao (sc3ez)

This project aims to design and implement a mini file system on top of a virtual disk. a library is offered (disk.c), which contains some basic file system calls (open,read write). This file system is designed as two part, virtual disk and memory. The virtual disk is used to hold the metadata and data permanently, even the file system is closed, while the memory is a array which is active to hold the metadata when the file system is open. 
Layout of the virtual disk
The virtual disk has 64 blocks, each block has 16 bytes. The first 32 blocks are used for store metadata, while the rest 32 blocks are data block. The first block, block[0], is the super block. Block[2] to Block[5] are used to stored FAT. Block[6] to Block[9] are used to stored directory. Block[32] to Block[63] are used to store data.
FAT contains 32 entries, each has 2 bytes. The first byte represent whether the block is available, the second byte stores the block number of next block of this file.
Directory contains 8 entries, each represents a file, and has 8 bytes. Byte 1 means whether the file is existing; byte 2 and 3 represent the length of file; byte 4 to byte 7 store the name of the file; the last byte, byte8, stores the first block of this file. 
2. OFT 
The open file table is a table stores the information of opened files, which has 4 entries. Each entry has 3 items, which represent whether the file is open, file descriptor, and offset.
3.  Sub-functions
To realize the file system, there are 12 basic function need to be built. The following is the implement of the sub-function.
int make_fs(char *disk_name);
call the make_disk() and open_disk(). Complete the disk creation for later use.
(2) int mount_fs(char *disk_name);
Read all the metadata from the virtual disk to the metadata buffer which is created in the function, by calling the block_read() function.
(3) int dismount_fs(char *disk_name);
Write the updated metadata buffer back to the virtual disk, by calling the block_write() function.
(4) int fs_open(char *name);
Firstly check the information in the directory according to the name, if the file exists, write it in the OFT table. 
(5) int fs_close(int fildes);
Check the OFT according to the file descriptor, if the file is existing in OFT, remove it from the OFT.
(6) int fs_create(char *name);
Firstly check the FAT and directory make sure there is no duplicate file. Then allocate a position of directory for it, and find a empty block according to FAT for it as the first block.
(7) int fs_delete(char *name);
After check whether it is open, set all blocks belong to this file to empty by calling the block_write() function.
(8) int fs_read(int fildes, void *buf, size_t nbyte);
According to the offset to find the substring of file, put it into buffer. After read, update the offset in OFT.
(9) int fs_write(int fildes, void *buf, size_t nbyte);
Firstly check whether there is empty block for writing. Then put the context of input block by block through using block_writing(), when write the next block, search the FAT, find the empty block, mark it in the current FAT entry, and move to that block to write. If the offset is not 0, the function will find the break point according to offset, set the rest part of the file as empty, and append the new input after the breakpoint. After that, update offset and file length.
(10) int fs_get_filesize(int fildes);
Calculate the file length from the directory and return.
(12) int fs_lseek(int fildes, off_t offset);
Use the current offset minus the input offset, and update the offset in OFT.
(13) int fs_truncate(int fildes, off_t length);
Find the breakpoint according to the length. use the strcnpy() to cut the desire length, and use bock_write() write it back.

