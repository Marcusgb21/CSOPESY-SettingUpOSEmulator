#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <Windows.h>

using namespace std;

//header
void printHeader() {

    HANDLE console_color;
    console_color = GetStdHandle(STD_OUTPUT_HANDLE);

    cout << "                                                                                       \n"
            "    .d8888b.   .d8888b.   .d88888b.  8888888b.  8888888888 .d8888b. Y88b   d88P        \n"
            "   d88P  Y88b d88P  Y88b d88P\" \"Y88b 888   Y88b 888       d88P  Y88b Y88b d88P       \n"
            "   888    888 Y88b.      888     888 888    888 888       Y88b.       Y88o88P          \n"
            "   888         \"Y888b.   888     888 888   d88P 8888888    \"Y888b.     Y888P         \n"
            "   888            \"Y88b. 888     888 8888888P\"  888           \"Y88b.    888         \n"
            "   888    888       \"888 888     888 888        888             \"888    888          \n"
            "   Y88b  d88P Y88b  d88P Y88b. .d88P 888        888       Y88b  d88P    888            \n"
            "    \"Y8888P\"   \"Y8888P\"   \"Y88888P\"  888        8888888888 \"Y8888P\"     888    \n\n";

    int P = 10; //color green
    SetConsoleTextAttribute(console_color, P);
    cout << "  Hello, Welcome to CSOPESY command line!\n";

    P = 14; //color yellow
    SetConsoleTextAttribute(console_color, P);
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

    HANDLE console_color;
    console_color = GetStdHandle(STD_OUTPUT_HANDLE);

    while (true) {

        int P = 8; //color grey
        SetConsoleTextAttribute(console_color, P);
        cout << "  Enter a command: ";
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
