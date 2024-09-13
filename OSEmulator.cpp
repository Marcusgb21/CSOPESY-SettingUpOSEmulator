#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstdlib> 

using namespace std;

//header
void printHeader() {

    cout << "                                                              \n"
            "    ██████╗███████╗ ██████╗ ██████╗ ███████╗███████╗██╗   ██╗ \n"
            "   ██╔════╝██╔════╝██╔═══██╗██╔══██╗██╔════╝██╔════╝╚██╗ ██╔╝ \n"
            "   ██║     ███████╗██║   ██║██████╔╝█████╗  ███████╗ ╚████╔╝  \n"
            "   ██║     ╚════██║██║   ██║██╔═══╝ ██╔══╝  ╚════██║  ╚██╔╝   \n"
            "   ╚██████╗███████║╚██████╔╝██║     ███████╗███████║   ██║    \n"
            "    ╚═════╝╚══════╝ ╚═════╝ ╚═╝     ╚══════╝╚══════╝   ╚═╝    \n\n";
                                                          
    cout << "  Hello, Welcome to CSOPESY command line!\n";
    cout << "  Type 'exit' to quit, 'clear' to clear the screen.\n";
}

// Clears the screen and reprints the header
void clearScreen() {
    system("clear || cls");  // Linux/Mac - clear || Windows - cls 
    printHeader();
}

//function for commands
void runCommandLine() {
    string command;

    //print header at the start always
    printHeader();


    while (true) {
        cout << "\n  Enter a command: ";
        getline(std::cin, command);

        if (command == "initialize") {
            cout << "  initialize command recognized. Doing something.\n";
        }
        else if (command == "screen") {
            cout << "  screen command recognized. Doing something.\n";
        }
        else if (command == "scheduler-test") {
            cout << "  scheduler-test command recognized. Doing something.\n";
        }
        else if (command == "scheduler-stop") {
            cout << "  scheduler-stop command recognized. Doing something.\n";
        }
        else if (command == "report-util") {
            cout << "  report-util command recognized. Doing something.\n";
        }
        else if (command == "clear") {
            clearScreen();  
        }
        else if (command == "exit") {
            break; 
        }
        else {
            cout << "  Unknown command: " << command << "\n";
        }
    }
}

int main() {
    runCommandLine();
    return 0;
}
