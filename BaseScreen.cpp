#include <iostream>
#include <Windows.h>

#include "AConsole.h"
#include "BaseScreen.h"
#include "ConsoleManager.h"
#include <chrono>
#include <ctime>

using namespace std;

BaseScreen::BaseScreen(std::shared_ptr<Process> process, String processName) : AConsole(processName)
{
    this->attachedProcess = process;
}

void BaseScreen::onEnabled()
{
    this->enabled = true;
}

void BaseScreen::display()
{
    exited = false;
    string command, option, name;
    onEnabled();
    std::cout << "root:\> ";
    std::getline(std::cin, command);
    if (command == "exit") {
        this->enabled = false;
        exited = true;
    }
    else if (command == "clear" || command == "cls")
    {
        std::cout << "\033[2J\033[1;1H";
        printProcessData();
    }
    else if (command == "process-smi")
    {
        printProcessData;
    }
    else
    {
        std::cout << "Invalid command!" << std::endl;
    }
}

void BaseScreen::process()
{
    if (this->refreshed == false)
    {
        this->refreshed = true;
        this->printProcessData();

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
        this->printProcessData();
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

bool BaseScreen::hasExited()
{
    return exited;
}

void BaseScreen::printProcessData() const
{
    std::cout << "Process Name: " << this->name;
    std::cout << "0/50" << std::endl;
    std::cout << "Creation time: " << this->creationTime << std::endl;
}

void BaseScreen::initializeCreationTime()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm;
    localtime_s(&local_tm, &now_c);

    std::ostringstream oss;

    oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");

    creationTime = oss.str();
}
