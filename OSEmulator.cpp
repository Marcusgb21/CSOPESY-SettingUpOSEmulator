#include <iostream>
#include <string>
#include <cstdlib> 

//header
void printHeader() {
    std::cout << "  *****   *****     *****    ******   *******   *****   *       * \n";
    std::cout << " *       *     *   *     *   *     *  *        *     *   *     *  \n";
    std::cout << "*        *        *       *  *     *  *        *          *   *   \n";
    std::cout << "*         *****   *       *  ******   *******   *****      * *    \n";
    std::cout << "*              *  *       *  *        *              *      *     \n";
    std::cout << " *       *     *   *     *   *        *        *     *      *     \n";
    std::cout << "  *****   *****     *****    *        *******   *****       *     \n";
    std::cout << "Hello, Welcome to CSOPESY command line!\n";
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen.\n";
}

// Clears the screen and reprints the header
void clearScreen() {
    system("clear || cls");  // Linux/Mac - clear || Windows - cls 
    printHeader();
}

//function for commands
void runCommandLine() {
    std::string command;

    //print header at the start always
    printHeader();


    while (true) {
        std::cout << "\nEnter a command: ";
        std::getline(std::cin, command);

        if (command == "initialize") {
            std::cout << "initialize command recognized. Doing something.\n";
        }
        else if (command == "screen") {
            std::cout << "screen command recognized. Doing something.\n";
        }
        else if (command == "scheduler-test") {
            std::cout << "scheduler-test command recognized. Doing something.\n";
        }
        else if (command == "scheduler-stop") {
            std::cout << "scheduler-stop command recognized. Doing something.\n";
        }
        else if (command == "report-util") {
            std::cout << "report-util command recognized. Doing something.\n";
        }
        else if (command == "clear") {
            clearScreen();  
        }
        else if (command == "exit") {
            break; 
        }
        else {
            std::cout << "Unknown command: " << command << "\n";
        }
    }
}

int main() {
    runCommandLine();
    return 0;
}
