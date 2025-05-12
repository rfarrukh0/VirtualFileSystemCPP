#include "vfs.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    initDisk(); // setup disk
    std::string line, cmd, arg1, arg2;

    while (true) {
        std::cout << "> "; // display prompt
        std::getline(std::cin, line);

        // saving the command to the history file
        std::ofstream hist("history.txt", std::ios::app);
        hist << line << "\n";
        hist.close();

        // parse input: (command, arg1, arg2)
        std::istringstream iss(line);
        iss >> cmd >> arg1;
        std::getline(iss, arg2);
        if (!arg2.empty() && arg2[0] == ' ') arg2 = arg2.substr(1);

        // core commands implemented
        if (cmd == "mkdir") mkdir(arg1);                      // create new dir
        else if (cmd == "touch") touch(arg1);                 // create new file 
        else if (cmd == "ls") ls();                           // list dirs and files
        else if (cmd == "cd") cd(arg1);                       // change dir
        else if (cmd == "cat") cat(arg1);                     // read display file
        else if (cmd == "write") writeFile(arg1, arg2);       // write to file
        else if (cmd == "rm") rm(arg1);                       // delete file or dir
        else if (cmd == "pwd") pwd();                         // show cur path
        else if (cmd == "tree") tree();                       // print dir structure
        else if (cmd == "login") login(arg1);                 // login as user
        else if (cmd == "logout") logout();                   // go back to root
        else if (cmd == "format") formatDisk();               // wipe disk with confirmation
        else if (cmd == "search") search(arg1);               // search recursiverly from current dir



        // display history
        else if (cmd == "history") {
            std::ifstream histRead("history.txt");
            std::string histLine;
            int count = 1;
            while (std::getline(histRead, histLine)) {
                std::cout << count++ << ": " << histLine << "\n";
            }
        }

        // run script file 
        else if (cmd == "run") {
            std::ifstream script(arg1);
            if (!script) {
                std::cout << "Failed to open script file\n";
                continue;
            }

            std::string scriptLine;
            while (std::getline(script, scriptLine)) {
                std::cout << "> " << scriptLine << "\n";

                std::istringstream ss(scriptLine);
                std::string sCmd, sArg1, sArg2;
                ss >> sCmd >> sArg1;
                std::getline(ss, sArg2);
                if (!sArg2.empty() && sArg2[0] == ' ') sArg2 = sArg2.substr(1);
                
                // same as above, just if from file 
                if (sCmd == "mkdir") mkdir(sArg1);
                else if (sCmd == "touch") touch(sArg1);
                else if (sCmd == "ls") ls();
                else if (sCmd == "cd") cd(sArg1);
                else if (sCmd == "cat") cat(sArg1);
                else if (sCmd == "write") writeFile(sArg1, sArg2);
                else if (sCmd == "rm") rm(sArg1);
                else if (sCmd == "pwd") pwd();
                else if (sCmd == "tree") tree();
                else if (sCmd == "login") login(sArg1);
                else if (sCmd == "logout") logout();
                else if (sCmd == "history") {
                    std::ifstream h("history.txt");
                    std::string l;
                    int c = 1;
                    while (std::getline(h, l)) std::cout << c++ << ": " << l << "\n";
                }
                else std::cout << "Invalid command in script: " << sCmd << "\n";
            }
        }

        else if (cmd == "exit") break; // quit
        else std::cout << "Invalid command\n"; // invalid command
    }

    return 0;
}
