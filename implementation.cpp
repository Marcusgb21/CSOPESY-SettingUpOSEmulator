#include "headers.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <sstream>
#include <windows.h>
#include <chrono>
#include <iomanip>

// AbstractScreen methods
void AbstractScreen::write(const std::string& text, WORD color) {
    std::lock_guard<std::mutex> lock(console_mutex);
    buffer.push_back({ text, color });
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    std::cout << text << std::endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);  // Reset color
}

void AbstractScreen::redraw() {
    system("cls");
    std::lock_guard<std::mutex> lock(console_mutex);
    for (const auto& entry : buffer) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), entry.color);
        std::cout << entry.text << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);  // Reset color
    }
}

// Screen methods
Screen::Screen(std::string name, int currentLine, int totalLine)
    : processName(name), currentLine(currentLine), totalLine(totalLine), timestamp(time(nullptr)) {}

std::string Screen::getProcessName() const {
    return processName;
}

void Screen::redraw() {
    system("cls");
    AbstractScreen::redraw();
}

bool Screen::screenCommand(std::vector<std::string> separatedCommand, std::string command) {
    // Handle commands specific to Screen
    return false;
}

// PrintCommand methods
PrintCommand::PrintCommand(int pid, const std::string& timestamp, const std::string& toPrint)
    : ICommand(pid, PRINT), timestamp(timestamp), toPrint(toPrint) {
}

void PrintCommand::execute(int coreID) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Capture the current time at the moment of execution
    auto now = std::chrono::high_resolution_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Convert to local time (or you can choose another format as per your needs)
    std::stringstream timestampStream;
    timestampStream << std::put_time(std::localtime(&now_time_t), "%m/%d/%Y  %H:%M:%S %p");
    std::string timestamp = timestampStream.str();

    // Check if the file exists
    std::string filename = "process_" + std::to_string(this->pid) + ".txt";
    FILE* file = fopen(filename.c_str(), "r");
    if (file != nullptr) {
        std::ofstream outfile(filename, std::ios_base::app);
        outfile << std::endl << "\(" << timestampStream.str() << "\) " << " Core:" << coreID << "  " << toPrint;
        outfile.close();
        fclose(file);
    }
    else {
        // File doesn't exist (CREATE)
        std::ofstream outfile(filename);

        outfile << "Process Name: process_" << std::to_string(this->pid) << std::endl;
        outfile << "Logs: " << std::endl;
        outfile << std::endl << "\(" << timestampStream.str() << "\) " << " Core:" << coreID << "  " << toPrint;
        outfile.close();
    }
}

// Process methods
Process::Process(int pid, std::string name, RequirementFlags reqFlags, int commandCounter)
    : pid(pid), name(name), reqFlags(reqFlags), commandCounter(0) {}

void Process::addCommand(ICommand::CommandType commandType) {
    if (commandType == ICommand::PRINT) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        std::string timestamp = std::to_string(ltm->tm_hour) + ":" +
            std::to_string(ltm->tm_min) + ":" +
            std::to_string(ltm->tm_sec);

        std::shared_ptr<ICommand> printCmd = std::make_shared<PrintCommand>(pid, timestamp, "\"Hello world from " + name + "!\"");
        commandList.push_back(printCmd);
    }
}

void Process::executeCurrentCommand(int coreID) {
    if (commandCounter < commandList.size()) {
        commandList[commandCounter]->execute(coreID);
        this->incrementIteration();  // Increment the iteration after executing the command
        //DEBUG
        //std::cout << "COUNTER" << commandCounter << std::endl;
    }
}

bool Process::isFinished() const {
    return commandCounter == commandList.size();
}

// Scheduler methods
void Scheduler::addProcess(Process& process) {
    std::lock_guard<std::mutex> lock(queueMutex);
    processQueue.push_back(process);
}

void Scheduler::start() {
    stopScheduler = false;
    schedulerThread = std::thread(&Scheduler::schedulerLoop, this);  // Start scheduler thread

    for (int i = 0; i < 4; ++i) {
        coreThreads.push_back(std::thread(&Scheduler::runCore, this, i));
    }
}

void Scheduler::runCore(int coreID) {
    while (!stopScheduler) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return !processQueue.empty() || stopScheduler; });

        if (stopScheduler && processQueue.empty()) {
            break;  // Exit if no more processes to handle
        }

        if (!processQueue.empty()) {
            Process process = processQueue.front();
            process.setCoreID(coreID);  // Set the core ID

            //PUT CODE HERE
            // Check if process is already finished
                // If the process is not found in finishedProcesses, pop it from the queue and continue execution
            processQueue.pop_front();  // Remove process from queue
            runningProcesses.push_back(process);  // Add to running processes

            lock.unlock();  // Unlock the mutex before executing the commands

            //DEBUG:
            //std::cout << process.getName() << std::endl;

            // Execute all commands in the current process until it finishes
            while (!process.isFinished()) {
                process.executeCurrentCommand(coreID);  // Execute command

                //update the process current iteration
                std::vector<Process>& runningProcesses = Scheduler::getRunningProcesses();
                std::vector<Process>::iterator it;
                // Loop through running processes and update the iteration of the matching PID
                for (it = runningProcesses.begin(); it != runningProcesses.end(); ++it) {
                    if (it->getPID() == process.getPID()) {  // Match by PID
                        it->setCurrentIteration(process.getCurrentIteration());
                        break;  // Once found, no need to continue the loop
                    }
                }
            }

            // Remove from running processes after completion
            std::lock_guard<std::mutex> finishedLock(queueMutex);
            runningProcesses.erase(
                std::remove_if(runningProcesses.begin(), runningProcesses.end(),
                    [&](const Process& p) { return p.getPID() == process.getPID(); }),
                runningProcesses.end()
            );
            finishedProcesses.push_back(process);  // Store finished process in vector
            cv.notify_all();
        }
    }
}

void Scheduler::schedulerLoop() {
    while (!stopScheduler) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!processQueue.empty()) {
                cv.notify_one();  // Notify worker threads
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
}

Process Scheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (!processQueue.empty()) {
        Process process = processQueue.front();
        processQueue.pop_front();
        return process;
    }
    return Process(-1, "", { false, 0, false, 0 }, 0);  // Dummy process if queue is empty TEST
}

void Scheduler::stop() {
    stopScheduler = true;
    cv.notify_all();  // Wake up all waiting threads

    if (schedulerThread.joinable()) {
        schedulerThread.join();  // Stop the scheduler thread
    }

    for (auto& thread : coreThreads) {
        if (thread.joinable()) {
            thread.join();  // Stop all worker threads
        }
    }
    coreThreads.clear();
}

// MainConsole methods
void MainConsole::run() {

    //init processes and process commands
    Process process0(0, "process_" + std::to_string(0), { false, 0, false, 0 }, 0);
    Process process1(1, "process_" + std::to_string(1), { false, 0, false, 0 }, 0);
    Process process2(2, "process_" + std::to_string(2), { false, 0, false, 0 }, 0);
    Process process3(3, "process_" + std::to_string(3), { false, 0, false, 0 }, 0);
    Process process4(4, "process_" + std::to_string(4), { false, 0, false, 0 }, 0);
    Process process5(5, "process_" + std::to_string(5), { false, 0, false, 0 }, 0);
    Process process6(6, "process_" + std::to_string(6), { false, 0, false, 0 }, 0);
    Process process7(7, "process_" + std::to_string(7), { false, 0, false, 0 }, 0);
    Process process8(8, "process_" + std::to_string(8), { false, 0, false, 0 }, 0);
    Process process9(9, "process_" + std::to_string(9), { false, 0, false, 0 }, 0);

    for (int i = 0; i < 100; i++)
        process0.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process1.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process2.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process3.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process4.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process5.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process6.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process7.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process8.addCommand(ICommand::PRINT);
    for (int i = 0; i < 100; i++)
        process9.addCommand(ICommand::PRINT);

    scheduler.addProcess(process0);
    scheduler.addProcess(process1);
    scheduler.addProcess(process2);
    scheduler.addProcess(process3);
    scheduler.addProcess(process4);
    scheduler.addProcess(process5);
    scheduler.addProcess(process6);
    scheduler.addProcess(process7);
    scheduler.addProcess(process8);
    scheduler.addProcess(process9);

    scheduler.start();

    continueProgram = true;
    currentView = "MainMenu";
    printHeader();

    std::string user_input;
    while (continueProgram) {
        std::cout << "\n  Enter a command: ";
        std::getline(std::cin, user_input);
        continueProgram = processCommand(user_input);
    }
}

bool MainConsole::processCommand(const std::string& command) {
    if (command == "exit") {
        
    }

    /*else if (command == "add process") {
        Process::RequirementFlags reqFlags = { false, 0, false, 0 };
        Process newProcess(1, "TestProcess", reqFlags);
        newProcess.addCommand(ICommand::PRINT);
        newProcess.addCommand(ICommand::PRINT);
        scheduler.addProcess(newProcess);

        std::cout << newProcess.getPID() << std::endl;
        std::cout << newProcess.getName() << std::endl;
        std::cout << newProcess.getState() << std::endl;

        return true;
    }*/

    if (command == "initialize") {
        std::cout << "  initialize command recognized. Doing something.\n";
    }
    else if (command == "screen") {
        std::cout << "  screen command recognized. Doing something.\n";
    }
    else if (command == "screen -ls") {
        std::deque<Process> allProcesses = scheduler.getAllProcesses();
        return true;
    }
    else if (command == "scheduler-test") {
        std::cout << "  scheduler-test command recognized. Doing something.\n";
    }
    else if (command == "scheduler-stop") {
        std::cout << "  scheduler-stop command recognized. Doing something.\n";
    }
    else if (command == "report-util") {
        std::cout << "  report-util command recognized. Doing something.\n";
    }
    else if (command == "clear") {
        std::cout << "  clear command recognized. Doing something.\n";
        system("clear || cls");  // Linux/Mac - clear || Windows - cls 
        printHeader();
    }
    else if (command == "exit") {
        std::cout << std::endl << "  exit command recognized!\n";
        scheduler.stop();
        return false;
    }
    else {
        std::cout << "  Unknown command: " << command << "\n";
    }
}

ICommand::ICommand(int pid, CommandType commandType) : pid(pid), commandType(commandType) {} //dummy constructor



void MainConsole::setConsoleColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void MainConsole::printHeader() {
    HANDLE console_color;
    console_color = GetStdHandle(STD_OUTPUT_HANDLE);

    std::cout << "                                                                                       \n"
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
    std::cout << "  Hello, Welcome to CSOPESY commandline!\n";

    P = 14; //color yellow
    SetConsoleTextAttribute(console_color, P);
    std::cout << "  Type 'exit' to quit, 'clear' to clear the screen\n";
    setConsoleColor(7);
}
