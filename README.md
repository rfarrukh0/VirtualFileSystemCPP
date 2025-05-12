# Virtual File System (C++)

This is a simple UNIX-style virtual file system I built in C++. It runs in your terminal like a shell and uses a binary file to simulate a disk. You can make folders, create and edit files, log in as different users, and even run scripts.

---

##  What it Can Do

- Simulates a 1MB disk using a binary file (`virtual_disk.bin`)
- Basic file system commands like:
  - `mkdir`, `cd`, `ls`, `touch`, `rm`, `pwd`, `tree`
- File operations:
  - `write <file> <text>`, `cat <file>`
- Multi-user support with `login <name>` and `logout`
- UNIX-style permission system (`rwx` format like `755`, `644`)
- Recursive file search with `search <filename>`
- Scripting with `run <script.vfs>` to batch commands
- Command logging with `history`
- Safe `format` command that wipes everything (with Y/N confirm)

---

##  How to run

1. Clone the repo  
2. Compile it using g++:

```
g++ main.cpp vfs.cpp -o vfs
```

3. Run the program:
```
./vfs
```
4.) Start using it like a mini shell. Try:

```
Copy
Edit
mkdir test
cd test
touch notes.txt
write notes.txt hello world
cat notes.txt
```

5.) Or run the demo script to see everything in action:
```
run setup.vfs
