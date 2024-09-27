#include <iostream>
#include <Windows.h>

#include "AConsole.h"
#include "BaseScreen.h"
#include "ConsoleManager.h"

BaseScreen::BaseScreen(std::shared_ptr<Process> process, String processName) : AConsole(processName)
{
    this->attachedProcess = process;
}

void BaseScreen::onEnabled()
{
    this->enabled = true;
}

void BaseScreen::process()
{
    if (this->refreshed == false)
    {
        this->refreshed = true;
        this->printProcessInfo();

    }

    std::cout << "root:\\>";

    // Read User Input
    std::string command;
    std::getline(std::cin, command);

    if (command == "clear" || command == "cls") {
        system("cls");
    }
    else if (command == "process-smi")
    {
        this->printProcessInfo();
    }
    else if (command == "exit")
    {
        ConsoleManager::getInstance()->returnToPreviousConsole();
    ConsoleManager:getInstance()->unregisterScreen(this->name);
    }
    else
    {

    }

}