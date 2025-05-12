#include "vfs.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

// virtual disk filename
const std::string DISK_FILE = "virtual_disk.bin";

// global disk and state variables
std::fstream disk;
FileEntry metadata[MAX_FILES];         // stores all files/folders
bool blockBitmap[TOTAL_BLOCKS] = {false}; // keeps track of used blocks
int currentDirIndex = 0;               // index of current working directory
int currentUserId = 0;                 // current logged in user
std::string currentUsername = "root";  // current username

// save metadata array to disk
void writeMetadata() {
    disk.seekp(0);
    disk.write(reinterpret_cast<char*>(metadata), sizeof(metadata));
}

// load metadata array from disk
void readMetadata() {
    disk.seekg(0);
    disk.read(reinterpret_cast<char*>(metadata), sizeof(metadata));
}

// set up disk if it doesnt exist or load 
void initDisk() {
    disk.open(DISK_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (!disk) {
        std::cout << "Creating virtual disk...\n";
        std::ofstream create(DISK_FILE, std::ios::binary);
        create.seekp(BLOCK_SIZE * TOTAL_BLOCKS - 1);
        create.write("", 1);
        create.close();

        disk.open(DISK_FILE, std::ios::in | std::ios::out | std::ios::binary);
        FileEntry root = {};
        strcpy(root.name, "/");
        root.isFile = false;
        root.parentIndex = -1;
        root.used = true;
        root.ownerId = 0;
        root.permissions = 755;
        metadata[0] = root;
        writeMetadata();
    } else {
        readMetadata();
    }
}

// find free block to write data
int allocateBlock() {
    for (int i = DATA_BLOCK_START; i < TOTAL_BLOCKS; i++) {
        if (!blockBitmap[i]) {
            blockBitmap[i] = true;
            return i;
        }
    }
    return -1; // no space
}

// make new folder
void mkdir(const std::string& name) {
    // check if already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used &&
            metadata[i].parentIndex == currentDirIndex &&
            name == metadata[i].name) {
            std::cout << "File or directory already exists\n";
            return;
        }
    }

    // find empty slot and create folder
    for (int i = 0; i < MAX_FILES; i++) {
        if (!metadata[i].used) {
            strcpy(metadata[i].name, name.c_str());
            metadata[i].isFile = false;
            metadata[i].used = true;
            metadata[i].parentIndex = currentDirIndex;
            metadata[i].ownerId = currentUserId;
            metadata[i].permissions = 755;
            writeMetadata();
            std::cout << "Directory created\n";
            return;
        }
    }

    std::cout << "No space for more directories\n";
}

// make new file
void touch(const std::string& name) {
    // check if already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used &&
            metadata[i].parentIndex == currentDirIndex &&
            name == metadata[i].name) {
            std::cout << "File or directory already exists\n";
            return;
        }
    }

    // create file in free slot
    for (int i = 0; i < MAX_FILES; i++) {
        if (!metadata[i].used) {
            strcpy(metadata[i].name, name.c_str());
            metadata[i].isFile = true;
            metadata[i].used = true;
            metadata[i].size = 0;
            metadata[i].startBlock = -1;
            metadata[i].parentIndex = currentDirIndex;
            metadata[i].ownerId = currentUserId;
            metadata[i].permissions = 644;
            writeMetadata();
            std::cout << "File created\n";
            return;
        }
    }
    std::cout << "No space for more files\n";
}

// read and print
void cat(const std::string& name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used && metadata[i].isFile &&
            metadata[i].parentIndex == currentDirIndex &&
            name == metadata[i].name) {

            // check read permission
            bool isOwner = metadata[i].ownerId == currentUserId;
            int perm = metadata[i].permissions;
            int readBit = isOwner ? (perm / 100) : ((perm / 10) % 10);

            if ((readBit & 4) == 0) {
                std::cout << "Permission denied\n";
                return;
            }

            // read from disk and print
            char buffer[BLOCK_SIZE] = {};
            disk.seekg(metadata[i].startBlock * BLOCK_SIZE);
            disk.read(buffer, metadata[i].size);
            std::cout << buffer << "\n";
            return;
        }
    }
    std::cout << "File not found\n";
}

// write content to file
void writeFile(const std::string& name, const std::string& content) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used && metadata[i].isFile &&
            metadata[i].parentIndex == currentDirIndex &&
            name == metadata[i].name) {

            // check write permission
            bool isOwner = metadata[i].ownerId == currentUserId;
            int perm = metadata[i].permissions;
            int writeBit = isOwner ? (perm / 100) : ((perm / 10) % 10);

            if ((writeBit & 2) == 0) {
                std::cout << "Permission denied\n";
                return;
            }

            // allocate block and write
            int block = allocateBlock();
            if (block == -1) {
                std::cout << "Disk full\n";
                return;
            }

            metadata[i].startBlock = block;
            metadata[i].size = content.size();
            disk.seekp(block * BLOCK_SIZE);
            disk.write(content.c_str(), content.size());
            writeMetadata();
            std::cout << "Written to file\n";
            return;
        }
    }
    std::cout << "File not found\n";
}

// list all files and dirs in dir
void ls() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used && metadata[i].parentIndex == currentDirIndex) {
            std::cout << (metadata[i].isFile ? "[FILE] " : "[DIR] ") << metadata[i].name << "\n";
        }
    }
}

// change working dir
void cd(const std::string& name) {
    if (name == "..") {
        // go up if not root
        if (metadata[currentDirIndex].parentIndex != -1)
            currentDirIndex = metadata[currentDirIndex].parentIndex;
        return;
    }

    // go into target dir
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used && !metadata[i].isFile &&
            metadata[i].parentIndex == currentDirIndex &&
            name == metadata[i].name) {
            currentDirIndex = i;
            return;
        }
    }

    std::cout << "Directory not found\n";
}

// delete file or dir (must be empty)
void rm(const std::string& name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used &&
            metadata[i].parentIndex == currentDirIndex &&
            name == metadata[i].name) {

            // check if dir is empty
            if (!metadata[i].isFile) {
                for (int j = 0; j < MAX_FILES; j++) {
                    if (metadata[j].used && metadata[j].parentIndex == i) {
                        std::cout << "Directory not empty\n";
                        return;
                    }
                }
            }

            // free block if file
            if (metadata[i].isFile && metadata[i].startBlock != -1) {
                blockBitmap[metadata[i].startBlock] = false;
            }

            metadata[i] = {};
            writeMetadata();
            std::cout << "Deleted successfully\n";
            return;
        }
    }
    std::cout << "File or directory not found\n";
}

// print full path to current folder
void pwd() {
    std::vector<std::string> path;
    int index = currentDirIndex;
    while (index != -1) {
        path.push_back(metadata[index].name);
        index = metadata[index].parentIndex;
    }
    for (int i = path.size() - 1; i >= 0; i--) {
        if (i == path.size() - 1 && path[i] == "/") std::cout << "/";
        else std::cout << path[i] << (i != 0 ? "/" : "");
    }
    std::cout << "\n";
}

// draw a tree view of curr dir struct
void tree(int index, int depth) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used && metadata[i].parentIndex == index) {
            std::cout << std::string(depth * 2, ' ') << (metadata[i].isFile ? "- " : "+ ") << metadata[i].name << "\n";
            if (!metadata[i].isFile) tree(i, depth + 1);
        }
    }
}

// log in 
void login(const std::string& username) {
    currentUsername = username;
    currentUserId = std::hash<std::string>{}(username) % 10000;
    std::cout << "Logged in as " << username << " (UID " << currentUserId << ")\n";
}

// log out aand go back to root usr
void logout() {
    currentUsername = "root";
    currentUserId = 0;
    std::cout << "Logged out. Defaulting to root\n";
}

// format the disk (erase)
void formatDisk() {
    std::string input;
    std::cout << "Are you sure you want to format the virtual disk? This will erase all data. (Y/N): ";
    std::getline(std::cin, input);

    if (input != "Y" && input != "y") {
        std::cout << "Format canceled.\n";
        return;
    }

    // clear metadata
    for (int i = 0; i < MAX_FILES; i++) {
        metadata[i] = {};
    }

    // reset block bitmap
    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        blockBitmap[i] = false;
    }

    // re-create root directory
    FileEntry root = {};
    strcpy(root.name, "/");
    root.isFile = false;
    root.parentIndex = -1;
    root.used = true;
    root.ownerId = 0;
    root.permissions = 755;
    metadata[0] = root;

    currentDirIndex = 0;
    currentUserId = 0;
    currentUsername = "root";

    // wipe disk data
    disk.seekp(0);
    char zero = 0;
    for (int i = 0; i < DISK_SIZE; i++) {
        disk.write(&zero, 1);
    }

    writeMetadata();
    std::cout << "Disk formatted successfully.\n";
}

// helper function for recursive search
void searchHelper(const std::string& target, int index, std::string path) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (metadata[i].used && metadata[i].parentIndex == index) {
            std::string currentPath = path + "/" + metadata[i].name;
            if (metadata[i].name == target) {
                std::cout << "Found at: " << currentPath << "\n";
            }
            if (!metadata[i].isFile) {
                searchHelper(target, i, currentPath);
            }
        }
    }
}

// search for file by name starting from current dir
void search(const std::string& name) {
    searchHelper(name, currentDirIndex, "");
}
