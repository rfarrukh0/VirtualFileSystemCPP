#ifndef VFS_H
#define VFS_H

#include <string>

const int DISK_SIZE = 1048576;                              // size of virtual disk in bytes (1MB)
const int BLOCK_SIZE = 512;                                 // size of one block in bytes
const int TOTAL_BLOCKS = DISK_SIZE / BLOCK_SIZE;            // total number of blocks on the disk
const int MAX_FILES = 128;                                  // max number of files/folders allowed
const int METADATA_SIZE = MAX_FILES * 64;                   // total metadata size (1 entry = 64 bytes)
const int METADATA_BLOCKS = METADATA_SIZE / BLOCK_SIZE + 1; // how many blocks the metadata takes up
const int DATA_BLOCK_START = METADATA_BLOCKS;               // where actual file data starts (after metadata)

// file/folder entry structure
struct FileEntry {
    char name[32];         // file or folder name
    bool isFile;           // true = file, false = directory
    int size;              // size in bytes (for files)
    int startBlock;        // where file data starts in disk
    int parentIndex;       // index of parent folder
    bool used;             // whether this entry is in use
    int ownerId;           // user id of owner
    int permissions;       // permission bits (e.g. 755)
};

void writeMetadata();                                                                // saves metadata to disk
void readMetadata();                                                                 // loads metadata from disk
void initDisk();                                                                     // sets up disk on first run or loads it

 // refer to main.cpp 
void mkdir(const std::string& name);                                                 
void touch(const std::string& name);                                                 
void ls();                                                                           
void cd(const std::string& name);
void writeFile(const std::string& name, const std::string& content);
void cat(const std::string& name);
void rm(const std::string& name);
void pwd();
void tree(int index = 0, int depth = 0);
void login(const std::string& username);
void logout();
void formatDisk();
void search(const std::string& name);


#endif
