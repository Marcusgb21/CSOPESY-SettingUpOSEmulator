#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <Windows.h>

#include "ConsoleManager.h"

using namespace std;

//header
#include <iostream>
#include <string>
#include <Windows.h>

#include "ConsoleManager.h"

using namespace std;

// Header function
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

    int P = 10; // color green
    SetConsoleTextAttribute(console_color, P);
    cout << "  Hello, Welcome to CSOPESY commandline!\n";

    P = 14; // color yellow
    SetConsoleTextAttribute(console_color, P);
    cout << "  Type 'exit' to quit, 'clear' to clear the screen, 'screen' to switch screens\n";
}

// Clears the screen and reprints the header
void clearScreen() {
    system("cls"); // Clear screen for Windows
    printHeader();
}

// Command line function
void runCommandLine() {
    string command;
    ConsoleManager::initialize(); // Initialize ConsoleManager

    // Print header at the start
    printHeader();

    HANDLE console_color = GetStdHandle(STD_OUTPUT_HANDLE);

    while (true) {
        int P = 15; // Color white
        SetConsoleTextAttribute(console_color, P);
        cout << "\n  Enter a command: ";
        getline(cin, command);

        if (command == "initialize") {
            cout << "  initialize command recognized. ConsoleManager initialized.\n";
        }
        else if (command == "screen") {
            cout << "  Enter screen name to switch to (MAIN_CONSOLE, MARQUEE_CONSOLE, SCHEDULING_CONSOLE): ";
            string screenName;
            getline(cin, screenName);

            // Switch to the selected screen
            ConsoleManager::getInstance()->switchConsole(screenName);
            cout << "  Switched to screen: " << screenName << "\n";
        }
        else if (command == "clear") {
            clearScreen();
        }
        else if (command == "exit") {
            cout << "  Exiting...\n";
            break;
        }
        else {
            cout << "  Unknown command: " << command << "\n";
        }
    }

    ConsoleManager::destroy(); // Clean up ConsoleManager before exit
}

int main() {
    runCommandLine();
    return 0;
}
