#include "FlatMemoryAllocator.h"
#include "headers.h"
#include <windows.h>
#include <string>
#include <ctime>
#include <vector>
#include <mutex>
#include <set>
#include <map>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <conio.h>
#include <random>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS

using namespace std;
typedef long long ll;
Config config;

/* ABSTRACT SCREEN IMPLEMENTATION */
void AbstractScreen::write(const string& text, WORD color) {
    lock_guard<mutex> lock(console_mutex);
    buffer.push_back({ text, color });
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    cout << text << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}
// Redraw(): responsible for re-opening a Screen and retaining its previous contents
void AbstractScreen::redraw() { //virtual
    system("cls");
    lock_guard<mutex> lock(console_mutex);
    for (const auto& entry : buffer) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), entry.color);
        cout << entry.text << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }
}



/* ALL ABOUT COMMANDS */
ICommand::ICommand(int pid, CommandType commandType) : pid(pid), commandType(commandType) {} // Dummy Command Constructor 
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

    //current_timestamp = timestampStream.str();

    // Check if the file exists
    std::string filename = "process_" + std::to_string(this->pid+1) + ".txt";
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



/* SCREEN IMPLEMENTATION */

// Three (3) constructors for Screen
Screen::Screen() : processName(""), currentLine(0), totalLine(0), timestamp(time(nullptr)) { } // Default
Screen::Screen(int processID, std::string name, int currentLine, int totalLine)
    : processID(processID), processName(name), currentLine(currentLine),
    totalLine(totalLine), timestamp(time(nullptr)) { }
Screen::Screen(int processID, string name, int totalLine)
    : processID(processID), processName(name), currentLine(currentLine),
    totalLine(totalLine), timestamp(time(nullptr)) { }

// Screen Variable Getters & Setters
string Screen::getProcessName() const {
    return processName;
}
int Screen::getCurrentLine() const {
    return currentLine;
}
int Screen::getTotalLine() const {
    return totalLine;
}
time_t Screen::getTimestamp() const {
    return timestamp;
}
void Screen::setCurrentLine(int currentLine) {
    this->currentLine = currentLine;
}

Screen::Screen(const Screen& other) {
    processID = other.processID;
    processName = other.processName;
    currentLine = other.currentLine;
    totalLine = other.totalLine;
    timestamp = other.timestamp;
}
Screen& Screen::operator=(const Screen& other) {
    if (this == &other) return *this;
    processID = other.processID;
    processName = other.processName;
    currentLine = other.currentLine;
    totalLine = other.totalLine;
    timestamp = other.timestamp;
    buffer = other.buffer;
    return *this;
}
bool Screen::operator<(const Screen& other) const {
    return timestamp < other.timestamp;
}

// Prints basic information inside opened screens
stringstream Screen::printScreen_helper() {
    stringstream ss;
    char buffer[26]; // Buffer large enough for the ctime_s output
    size_t size;

    // Convert the timestamp to a string using ctime_s
    if (ctime_s(buffer, sizeof(buffer), &timestamp) == 0) {
        ss << "  Process: " << processName << "\n"
            << "  ID: " << processID << "\n\n"
            << "  Current instruction line: " << currentLine << "\n"
            << "  Lines of code: " << totalLine << "\n"
            << "\n  Timestamp: " << buffer << "\n"; // Use the buffer for the timestamp
    }

    return ss;
}

// "opens" a new screen 
void Screen::openScreen() {
    system("cls");
    string content = printScreen_helper().str();
    int width = content.length() + 4;
    stringstream contentStream(content);
    string line;
    while (getline(contentStream, line)) {
        write(line + string(width - line.length() - 4, ' '));
    }
}

// Just calls abstract redraw that retains information when opening a screen
void Screen::redraw() { //override
    system("cls");
    AbstractScreen::redraw();
}

// Parang main menu inside an open Screen (only place for process-smi command)
bool Screen::screenCommand(vector<string> seperatedCommand, string command_to_check, Scheduler* scheduler) {

    const set<string> commands = { "exit" , "process-smi" };
    Process currentProcess = scheduler->getCurrentProcess(processName);
    string name = currentProcess.getName();
    //cout << "\nCURRENT = " << currentProcess.getCurrentIteration()<<endl;

    if (!commands.count(seperatedCommand[0])) {
        write("  Unknown command \"" + command_to_check + "\"!\n", 14);
        return false;
    }

    if (seperatedCommand[0] == "exit") {
        if (seperatedCommand.size() != 1) {
            write("  INVALID COMMAND");
            return false;
        }
        return true;
    }
    else if (seperatedCommand[0] == "process-smi") {
        if (currentProcess.getCurrentIteration() != 0 && currentProcess.commandList.size() != 0) {
            write("\n  Process: " + name);
            write("  ID: " + std::to_string(currentProcess.getPID()));
            write("\n  Current instruction line: " + std::to_string(currentProcess.getCurrentIteration()));
            write("  Lines of Code: " + std::to_string(currentProcess.commandList.size()) + "\n");
        }
        else {
            write("\n  Process: " + name);
            write("  ID: " + std::to_string(currentProcess.getPID()));
            write("\n  Finished!\n", 10);
        }

        return false;
    }
    return false;
}



/* PROCESS IMPLEMENTATION */

// Process Constructor
Process::Process(int pid, std::string name, RequirementFlags reqFlags, int commandCounter, size_t sizeMem)
    : pid(pid), name(name), reqFlags(reqFlags), commandCounter(0), sizeMem(config.mpp) {}

// Adds the dummy command to a Command List of a Process
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

// Executes current commands of processes (this is where the sleep delay happens)
void Process::executeCurrentCommand(int coreID, std::chrono::milliseconds dpe) {
    if (commandCounter < commandList.size()) {
        commandList[commandCounter]->execute(coreID);
        this->incrementIteration();  // Increment the iteration after executing the command

        std::time_t now = std::time(nullptr);
        std::tm* local_time = std::localtime(&now);

        // Use ostringstream to store formatted time as a string
        std::ostringstream oss;
        oss << std::put_time(local_time, "%m/%d/%Y %I:%M:%S%p");
        setCurrentTime(oss.str());

        std::this_thread::sleep_for(dpe);
        //std::cout << "COUNTER" << commandCounter << std::endl;
    }
}

// Checks if adding commands to a process is completed (returns how many commands)
bool Process::isFinished() const {
    return commandCounter == commandList.size();
}



/* SCHEDULER IMPLEMENTATION */

// Checks if the current process is running
int Scheduler::checkCurrentProcess(string processName) {
    int currentProcess, found = 0;
    vector<Process>& runningProcess = getRunningProcesses();

    for (int i = 0; i < runningProcess.size(); i++) {

        if (processName == runningProcess[i].getName()) {
            currentProcess = i;
            i = runningProcess.size();
            found = 1;
        }
    }

    if (found == 1)
        return 1;
    else
        return 0;
}

// Under-construction (might be needed in the future)
int Scheduler::ifProcessExists(string processName) {
    return 0;
}

// Finds currentProcess using the ProcessName & Returns the Process itself
Process Scheduler::getCurrentProcess(string processName) {
    int currentProcess, found = 0;
    vector<Process>& runningProcess = getRunningProcesses();

    for (int i = 0; i < runningProcess.size(); i++) {

        if (processName == runningProcess[i].getName()) {
            currentProcess = i;
            i = runningProcess.size();
            found = 1;
        }
    }

    if (found == 1)
        return runningProcess[currentProcess];
}

// Adds a process to the scheduler readyqueue
void Scheduler::addProcess(Process& process) {
    std::lock_guard<std::mutex> lock(queueMutex);
    processQueue.push_back(process);
}

// Actually starts the scheduler per quantum cycle
void Scheduler::start(int amtCpu, SchedulerAlgorithm algorithm, int timeQuantum, std::chrono::milliseconds dpe) {
    //MainConsole main;
    stopScheduler = false;
    schedulerThread = std::thread(&Scheduler::schedulerLoop, this);  // Start scheduler thread

    //main.printMemoryInfo();

    for (int i = 0; i < amtCpu; ++i) {
        coreThreads.push_back(std::thread(&Scheduler::runCore, this, i, algorithm, timeQuantum, dpe));
    }
}

// Where Algorithm is implemented (and assigned per core)
void Scheduler::runCore(int coreID, SchedulerAlgorithm algorithm, int timeQuantum, std::chrono::milliseconds dpe) {
    int time = 0;
    while (!stopScheduler) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return !processQueue.empty() || stopScheduler; });

        if (stopScheduler && processQueue.empty()) {
            break;  // Exit if no more processes to handle
        }

        if (!processQueue.empty()) {
            Process process = processQueue.front();
            process.setCoreID(coreID);  // Set the core ID

            processQueue.pop_front();  // Remove process from queue
            runningProcesses.push_back(process);  // Add to running processes
            lock.unlock();  // Unlock the mutex before executing the commands

            void* memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process.getSizeMem(), process.getName());
            if (memoryPtr) {
                //DEBUG:
                /*if (algorithm == SchedulerAlgorithm::RR)
                    std::cout << process.getName() << std::endl;*/

                if (algorithm == SchedulerAlgorithm::FCFS) {
                    // Execute all commands in the current process until it finishes
                    int burst = 0;

                    //void* memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process.getSizeMem(), process.getName());
                    //if (memoryPtr) {
                        while (!process.isFinished()) {
                            while (burst < timeQuantum)
                            {
                                process.executeCurrentCommand(coreID, dpe);  // Execute command

                                //update the process current iteration
                                std::vector<Process>& runningProcesses = Scheduler::getRunningProcesses();
                                std::vector<Process>::iterator it;
                                for (it = runningProcesses.begin(); it != runningProcesses.end(); ++it) {
                                    if (it->getPID() == process.getPID()) {
                                        it->setCurrentIteration(process.getCurrentIteration());
                                        break;
                                    }
                                }
                                burst++;
                                time++;
                            }
                            burst = 0;
                            FlatMemoryAllocator::getInstance()->printMemoryInfo(time);
                        }
                    //}
                    FlatMemoryAllocator::getInstance()->deallocate(memoryPtr);

                }
                else if (algorithm == SchedulerAlgorithm::RR) {
                    // Execute for the duration of timeQuantum
                    auto start_time = std::chrono::steady_clock::now();
                    int burst = 0;

                    //void* memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process.getSizeMem(), process.getName());
                    //if (memoryPtr) {
                    while (!process.isFinished() && burst < timeQuantum) {
                        process.executeCurrentCommand(coreID, dpe); // Execute command

                        //update the process current iteration
                        std::vector<Process>& runningProcesses = Scheduler::getRunningProcesses();
                        std::vector<Process>::iterator it;
                        for (it = runningProcesses.begin(); it != runningProcesses.end(); ++it) {
                            if (it->getPID() == process.getPID()) {
                                it->setCurrentIteration(process.getCurrentIteration());
                                break;
                            }
                        }
                        burst++;
                        time++;
                    }
                    //}
                   
                    FlatMemoryAllocator::getInstance()->printMemoryInfo(time);
                    FlatMemoryAllocator::getInstance()->deallocate(memoryPtr);
                }
            }

            if (algorithm == SchedulerAlgorithm::RR) {
                //Reinsert if not finished
                lock.lock(); // Lock again before modifying shared resources
                if (!process.isFinished()) {
                    processQueue.push_back(process); // Reinsert back into the queue
                }
                else {
                    finishedProcesses.push_back(process); // Store finished process
                    //FlatMemoryAllocator::getInstance()->deallocate(memoryPtr);
                }
            }

            // Remove from running processes after completion
            if (algorithm == SchedulerAlgorithm::FCFS) {
                std::lock_guard<std::mutex> finishedLock(queueMutex);
                runningProcesses.erase(
                    std::remove_if(runningProcesses.begin(), runningProcesses.end(),
                        [&](const Process& p) { return p.getPID() == process.getPID(); }),
                    runningProcesses.end()
                );
                finishedProcesses.push_back(process);  // Store finished process in vector
                cv.notify_all();
            }
            else if (algorithm == SchedulerAlgorithm::RR) {
                auto it = std::remove_if(runningProcesses.begin(), runningProcesses.end(),
                    [&](const Process& p) { return p.getPID() == process.getPID(); });
                if (it != runningProcesses.end()) {
                    runningProcesses.erase(it, runningProcesses.end());
                }
                cv.notify_all(); // Notify other threads
            }

        }
    }
}

// Runs scheduler until commanded to stop
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

// Not yet used
Process Scheduler::getNextProcess() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (!processQueue.empty()) {
        Process process = processQueue.front();
        processQueue.pop_front();
        return process;
    }
    return Process(-1, "", { false, 0, false, 0 }, 0, 0);  // Dummy process if queue is empty TEST
}

// Where scheduler-test is actually stopped
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

// Prints info to a file (used by report-util)
std::deque<Process> Scheduler::printAllProcesses() {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::deque<Process> allProcesses;

    int totalCores = coreThreads.size();       // Total number of cores
    int usedCores = runningProcesses.size();   // Active or running processes
    int availableCores = totalCores - usedCores;

    // Open a file to write the output; create if it doesn’t exist
    string fileDirectory = "csopesy-log.txt";

    std::ofstream outputFile(fileDirectory, std::ios::out | std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "  Failed to open the file for writing." << std::endl;
        return allProcesses;
    }

    float fUsed = static_cast <float>(usedCores);
    float fTotal = static_cast <float>(totalCores);

    // Write CPU utilization and core usage info
    outputFile << std::endl << "  CPU Utilization: " << (fUsed / totalCores) * 100 << "%" << std::endl;
    outputFile << "  Cores used: " << usedCores << std::endl;
    outputFile << "  Cores available: " << availableCores << std::endl;
    outputFile << "  _______________________________________" << std::endl;

    // Write running processes
    outputFile << "  Running Processes:" << std::endl;
    for (const auto& process : runningProcesses) {

        outputFile << "  Name: " << process.getName()
            << " | " << process.getCurrentTime()
            << " | Core: " << process.getCoreID()
            << " | " << process.getCurrentIteration() << " / " << process.commandList.size() << " | " << std::endl;
    }

    if (runningProcesses.empty()) {
        outputFile << "  No running processes." << std::endl;
    }

    // Copy finished processes
    allProcesses.insert(allProcesses.end(), finishedProcesses.begin(), finishedProcesses.end());

    // Write finished processes
    outputFile << std::endl << "  Finished Processes:" << std::endl;
    if (finishedProcesses.empty()) {
        outputFile << "  No finished processes." << std::endl;
    }
    else {
        //vector<Process>::iterator it;

        //for (it = finishedProcesses.begin(); it != finishedProcesses.end(); it++) {
        for (const auto& process : finishedProcesses) {
            outputFile << "  Name: " << process.getName()
                << " | " << process.getCurrentTime()
                << " | Finished"
                << " | " << process.getCurrentIteration() << " / " << process.commandList.size() << " | " << std::endl;
        }
        //}
    }
    outputFile << "  _______________________________________" << std::endl;

    outputFile.close();  // Close the file after writing
    cout << "  Report generated at " << fileDirectory << "!\n\n";
    return allProcesses;
};

// Returns running processes
std::vector<Process>& Scheduler::getRunningProcesses() { // Add function to retrieve running processes
    std::lock_guard<std::mutex> lock(queueMutex);
    return runningProcesses;
};

// Prints for "screen -ls" command and returns ALL processes
deque<Process> Scheduler::getAllProcesses() {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::deque<Process> allProcesses;

    int totalCores = coreThreads.size();  // Total number of cores
    int usedCores = runningProcesses.size();  // Active or running processes
    int availableCores = totalCores - usedCores;

    float fUsed = static_cast <float>(usedCores);
    float fTotal = static_cast <float>(totalCores);

    // Display cores usage
    std::cout << std::endl << "  CPU Utilization: " << (fUsed / totalCores) * 100 << "%" << std::endl;
    std::cout << "  Cores used: " << usedCores << std::endl;
    std::cout << "  Cores available: " << availableCores << std::endl;
    std::cout << "  _______________________________________" << std::endl;
    // Display running processes
    std::cout << "  Running Processes:" << std::endl;

    std::vector<Process>::iterator i;
    for (i = runningProcesses.begin(); i != runningProcesses.end(); i++) {
        std::cout << "  Name: " << i->getName()
            << " | " << i->getCurrentTime()
            << " | Core: " << i->getCoreID()
            << " | " << i->getCurrentIteration() << " / " << i->commandList.size() << " | " << std::endl;
    }

    if (runningProcesses.empty()) {
        std::cout << "  No running processes." << std::endl;
    }

    // Copy finished processes
    allProcesses.insert(allProcesses.end(), finishedProcesses.begin(), finishedProcesses.end());

    // Display finished processes
    std::cout << std::endl << "  Finished Processes:" << std::endl;
    if (finishedProcesses.empty()) {
        std::cout << "  No finished processes." << std::endl;
    }
    else {
        //CHANGE start
        std::vector<Process>::iterator it;

        for (it = finishedProcesses.begin(); it != finishedProcesses.end(); it++) {
            std::cout << "  Name: " << it->getName()
                << " | " << it->getCurrentTime()
                << " | Finished"
                << " | " << it->getCurrentIteration() << " / " << it->commandList.size() << " | " << endl;
        }

        //CHANGE end
    }

    cout << "  _______________________________________\n" << endl;

    return allProcesses;
}

void Scheduler::printMemory() {
    int totalCores = coreThreads.size();  // Total number of cores
    int usedCores = runningProcesses.size();  // Active or running processes
    float fUsed = static_cast <float>(usedCores);
    float fTotal = static_cast <float>(totalCores);

    double cpuUtil = (static_cast<double>(fUsed) / totalCores) * 100;

    int max = static_cast<int>(FlatMemoryAllocator::getInstance()->getMaxSize());
    int usage = static_cast<int>(FlatMemoryAllocator::getInstance()->getUsage());
    double memoryUtil = (static_cast<double>(usage) / max) * 100;

    cout << "\n  ----------------------------------------------" << endl;
    cout << "  |  PROCESS-SMI V01.00  DriverVersion: 01.00  |" << endl;
    cout << "  ----------------------------------------------" << endl;
    cout << "  CPU-Util: " << fixed << setprecision(2) << cpuUtil << "%" << endl;
    cout << "  Memory Usage: " << usage << " MiB / " << max << " MiB" << endl;
    cout << "  Memory Util: " << fixed << setprecision(2) << memoryUtil << "%" << endl << endl;
    cout << "  ==============================================" << endl;

    cout << "  Running Processes and Memory Usage:" << endl;
    cout << "  ----------------------------------------------" << endl;

    int procInMemory = static_cast<int>(FlatMemoryAllocator::getInstance()->getNumberOfProcessesInMemory());

    unordered_map<size_t, string> allocationMap = FlatMemoryAllocator::getInstance()->getAllocMap();
    map<string, size_t> processSizes;

    for (const auto& pair : allocationMap) {
        if (!pair.second.empty()) {
            processSizes[pair.second] += pair.first;
        }
    }

    if (processSizes.empty()) {
        cout << "  There are no running processes!" << endl;
    }
    else {
        for (const auto& pair : processSizes) {
            double procSize = static_cast<double>(pair.second) / config.mpp;
            cout << "  " << pair.first << "  " << fixed << setprecision(2) << procSize << " MiB" << endl;
        }
    }

    cout << "  ----------------------------------------------" << endl << endl;
}


/* MAIN CONSOLE IMPLEMENTATION */

// MainMenu UI & User-input
void MainConsole::run() {
    string user_input;

    continue_program = true;
    currentView = "MainMenu";
    print_header();

    while (continue_program) {
        cout << "  root:\\> ";
        getline(cin, user_input);
        continue_program = processCommand(user_input);
    }
}

// Recognize a command
void MainConsole::commandRecognize(string command_to_check) {
    write("\n  " + command_to_check + " command recognized.\n", 14);
}

// Handle unrecognized command
void MainConsole::commandNotRecognize(string command_to_check) {
    write("  Unknown command \"" + command_to_check + "\"!\n", 4);
}

// Handle invalid command arguments
void MainConsole::invalidCommand(string command_to_check) {
    write("  Invalid Arguments for \"" + command_to_check + "\"!\n", 4);
}

// Handle screen not found
void MainConsole::screenNotFound() {
    write("  Screen not found!\n");
}

// Under-construction (might be needed in the future)
void MainConsole::newProcess(int mins, int maxins) {

}

// This is where Dummy Processes are made
void MainConsole::initializeScheduler(int mins, int maxins, int mpp) {
    int counter = 1, id = 0; // Unique process identifier
    stop_scheduler = false; // Reset flag when starting scheduler
    int cycleCounter = 0;

    while (!stop_scheduler) {
        cycleCounter++;

        if (cycleCounter >= cycleInterval) {
            string processName = "Process_" + to_string(counter++);
            Process newProcess(id++, processName, { false, 0, false, 0 }, 0, mpp);

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distr(mins, maxins);

            for (int i = 1; i < distr(gen); i++) {
                newProcess.addCommand(ICommand::PRINT);
            }

            scheduler.addProcess(newProcess);
            //readyQueue.push(newProcess); // Add process to ready queue
            screensAvailable[processName] = Screen(id, processName, newProcess.commandList.size());
            //write("  Created process: " + processName, 10); test purposes

            cycleCounter = 0;  // Reset cycle counter after generating a process
        }

        checkForStopCommand();
        std::this_thread::sleep_for(std::chrono::seconds(config.bpf));
    }
}

// Checks for a scheduler-stop command
void MainConsole::checkForStopCommand() {
    if (_kbhit()) { // Checks if there is input available
        std::string command;
        std::getline(std::cin, command);
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);

        if (command == "scheduler-stop") {
            stop_scheduler = true; // Set flag to stop the loop
            write("  Scheduler stopped.", 14);
        }
        else {
            write("  Unknown command during scheduler-test: " + command, 12);
        }
    }
}

// Starts the creation of Dummy Processes
void MainConsole::schedulerTest() {

    if (stop_scheduler) {  // If previously stopped, reset flag
        stop_scheduler = false;
    }
    if (!schedulerThread.joinable()) {
        write("  Scheduler started.\n", 10);
        schedulerThread = std::thread(&MainConsole::initializeScheduler, this, config.mins, config.maxins, config.mpp);
    }
    else {
        write("  Scheduler is already running.", 14);
    }
}

// Stops the creation of Dummy Processes
void MainConsole::stopScheduler() {

    stop_scheduler = true;
    write("  Scheduler stopped externally.\n", 14);

    if (schedulerThread.joinable()) {
        schedulerThread.join();  // Wait for the scheduler thread to finish
    }
}

// Handle's recognition of main menu commands
bool MainConsole::mainMenuCommand(vector<string> seperatedCommand, string command_to_check) {
    const set<string> commands = { "initialize", "screen", "scheduler-test", "scheduler-stop", "report-util", "clear", "exit", "a", "process-smi", "vmstat"};

    // Check if the command is recognized
    if (seperatedCommand.empty() || commands.count(seperatedCommand[0]) == 0) {
        commandNotRecognize(command_to_check);
        return true; // Allow more input
    }
    else if (seperatedCommand[0] == "initialize") {
        if (!(seperatedCommand.size() == 1)) {
            invalidCommand(command_to_check);
            return true;
        }

        // Read configuration from the text file
        ifstream configFile("config.txt");
        if (configFile.is_open()) {
            string line;

            while (getline(configFile, line)) {
                istringstream iss(line);
                string key;
                if (iss >> key) {
                    if (key == "num-cpu") {
                        iss >> config.cpu; // Assign to config.cpu
                    }
                    else if (key == "scheduler") {
                        string temp;
                        getline(iss, temp); // Read the whole line after `scheduler`
                        size_t firstQuote = temp.find('"');
                        size_t lastQuote = temp.rfind('"');
                        if (firstQuote != string::npos && lastQuote != string::npos && firstQuote != lastQuote) {
                            config.algorithm = temp.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                        }

                        //string temp;
                        //iss >> temp; // skip the quote
                        //getline(iss, config.algorithm, '"'); // read until the next quote
                    }
                    else if (key == "quantum-cycles") {
                        iss >> config.qCycles; // skip "cycles"
                    }
                    else if (key == "batch-process-freq") {
                        iss >> config.bpf; // Assign to config.bpf
                    }
                    else if (key == "min-ins") {
                        iss >> config.mins; // Assign to config.mins
                    }
                    else if (key == "max-ins") {
                        iss >> config.maxins; // Assign to config.maxins
                    }
                    else if (key == "delay-per-exec") {
                        iss >> config.dpe; // Assign to config.dpe
                    }
                    else if (key == "max-overall-mem") {
                        iss >> config.mom; // Assign to config.dpe
                    }
                    else if (key == "mem-per-frame") {
                        iss >> config.mpf; // Assign to config.dpe
                    }
                    else if (key == "mem-per-proc") {
                        iss >> config.mpp; // Assign to config.dpe
                    }
                }
            }
        }

        doneInitialize = 1;
        configFile.close();

        // Optionally, print or log the loaded values
        write("\n  Configuration File Loaded!", 10);
        cout << "  CPU: " << config.cpu << endl;
        cout << "  Scheduler: " << config.algorithm << endl;
        cout << "  Quantum Cycles: " << config.qCycles << endl;
        cout << "  Batch Process Frequency: " << config.bpf << endl;
        cout << "  Min Instructions: " << config.mins << endl;
        cout << "  Max Instructions: " << config.maxins << endl;
        cout << "  Delay per Execution: " << config.dpe << endl;
        cout << "  max-overall-mem: " << config.mom << endl;
        cout << "  mem-per-frame: " << config.mpf << endl;
        cout << "  mem-per-proc: " << config.mpp << endl << endl;

        std::chrono::milliseconds quantumTime{ config.qCycles };
        std::chrono::milliseconds delayTime{ config.dpe };

        FlatMemoryAllocator::initialize(config.mom);


        // Initialize memory
        
        if (config.algorithm == "fcfs") {
            scheduler.start(config.cpu, Scheduler::SchedulerAlgorithm::FCFS, config.qCycles , delayTime);
        }
        else if (config.algorithm == "rr") {
            scheduler.start(config.cpu, Scheduler::SchedulerAlgorithm::RR, config.qCycles, delayTime);
        }
    }
    else if (seperatedCommand[0] == "clear") {
        if (!(seperatedCommand.size() == 1)) {
            invalidCommand(command_to_check);
            return true;
        }
        system("cls");
        this->buffer.clear();
        print_header();
    }
    else if (seperatedCommand[0] == "exit") {
        if (seperatedCommand.size() != 1) {
            invalidCommand(command_to_check);
            return true;
        }

        scheduler.stop();

        //if scheduler-stop isnt commanded yet, call it muna
        if (stop_scheduler == false) {
            stop_scheduler = true;

            if (schedulerThread.joinable()) {
                schedulerThread.join();  // Wait for the scheduler thread to finish
            }
        }

        write("  Exiting the program. Bye!", 14);
        return false;
    }
    else if (doneInitialize == 0) {
        write("  Command \"" + command_to_check + "\" cannot be used now.", 14);
    }

    if (doneInitialize) {
        if (seperatedCommand[0] == "scheduler-test") {
            if (seperatedCommand.size() != 1) {
                invalidCommand(command_to_check);
                return true;
            }
            //commandRecognize(command_to_check);
            schedulerTest(); // Start the scheduler
        }
        else if (seperatedCommand[0] == "scheduler-stop") {
            if (seperatedCommand.size() != 1) {
                invalidCommand(command_to_check);
                return true;
            }
            //commandRecognize(command_to_check);
            stopScheduler(); // Stop the scheduler
        }
        else if (seperatedCommand[0] == "screen") {
            if (seperatedCommand.size() < 2 || seperatedCommand.size() > 3) {
                invalidCommand(command_to_check);
                return true;
            }

            if (seperatedCommand.size() == 2) {
                if (seperatedCommand[1] == "-ls") {
                    std::deque<Process> allProcesses = scheduler.getAllProcesses();
                    //return true;
                }
                else {
                    return true;
                }
            }
            else if (seperatedCommand[1] == "-r") {
                if (!screensAvailable.count(seperatedCommand[2])) {
                    screenNotFound();
                    return true;
                }

                //std::vector<Process>& runnningProcess = scheduler.getRunningProcesses();
                int isCurrent = scheduler.checkCurrentProcess(seperatedCommand[2]);

                if (isCurrent) {
                    Process currentProcess = scheduler.getCurrentProcess(seperatedCommand[2]);

                    screensAvailable[seperatedCommand[2]].setCurrentLine(currentProcess.getCurrentIteration());
                    screensAvailable[seperatedCommand[2]].openScreen();

                    screensAvailable[seperatedCommand[2]].redraw();
                    currentView = screensAvailable[seperatedCommand[2]].getProcessName();
                }
                else {
                    screensAvailable[seperatedCommand[2]].redraw();
                    currentView = screensAvailable[seperatedCommand[2]].getProcessName();
                    //write("  There is no running process called \"" + seperatedCommand[2] + "\" yet!", 14);
                }

            }
            else if (seperatedCommand[1] == "-s") {
                if (screensAvailable.count(seperatedCommand[2])) {
                    write("  Process called \"" + seperatedCommand[2] + "\" is already exists!\n", 4);
                    return true;
                }

                std::vector<Process> allProcesses = scheduler.getRunningProcesses();
                int id = allProcesses.size();
                Process newProcess(id++, seperatedCommand[2], { false, 0, false, 0 }, 0, 0);

                /* Pang-randomize */
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distr(config.mins, config.maxins);

                for (int i = 1; i < distr(gen); i++) {
                    newProcess.addCommand(ICommand::PRINT);
                }

                scheduler.addProcess(newProcess);
                screensAvailable[seperatedCommand[2]] = Screen(id, seperatedCommand[2], 0, newProcess.commandList.size());
                screensAvailable[seperatedCommand[2]].openScreen();
                currentView = screensAvailable[seperatedCommand[2]].getProcessName();
            }
            else {
                invalidCommand(command_to_check);
            }
        }
        else if (seperatedCommand[0] == "report-util") {
            if (seperatedCommand.size() != 1) {
                invalidCommand(command_to_check);
                return true;
            }
            //commandRecognize(command_to_check);
            scheduler.printAllProcesses();

        }
        else if (seperatedCommand[0] == "a") {
            if (seperatedCommand.size() != 1) {
                invalidCommand(command_to_check);
                return true;
            }
            //commandRecognize(command_to_check);
            write("  Available Screens:");
            //for (auto [_, sc] : screensAvailable) {
            //	string content = sc.getProcessName();
            //	write(sc.getProcessName());
            //}
            for (auto& screenPair : screensAvailable) {
                string content = screenPair.second.getProcessName();
                write(content);  // Display the process name of each available screen.
            }
        }
        else if (seperatedCommand[0] == "process-smi")
        {
            scheduler.printMemory();
        }
        else if (seperatedCommand[0] == "vmstat") {
            cout << "\n  You entered VMSTAT lol pls populate this area" << endl << endl;
            /* POPULATE CODE FOR VMSTAT COMMAND HERE */
        }
        else if (seperatedCommand[0] != "initialize" && seperatedCommand[0] != "clear" && seperatedCommand[0] != "exit") {
            commandNotRecognize(command_to_check);
        }

        return true;
    }
    else {
        write("  Program not yet initialized!\n", 14);
    }

    return true;
}

// Process commands based on current view
bool MainConsole::processCommand(const string& command) {
    string command_to_check = command;
    transform(command_to_check.begin(), command_to_check.end(), command_to_check.begin(), ::tolower);

    stringstream stream(command);
    vector<string> seperatedCommand;

    string splitter;
    while (stream >> splitter) {
        seperatedCommand.push_back(splitter);
    }

    if (currentView == "MainMenu") {
        return mainMenuCommand(seperatedCommand, command_to_check);
    }
    else {
        if (screensAvailable[currentView].screenCommand(seperatedCommand, command_to_check, &scheduler)) {
            currentView = "MainMenu";
            this->redraw();
        }
    }
    return true;
}

// Function for printing CSOPESY header
void MainConsole::print_header() {
    HANDLE console_color;
    console_color = GetStdHandle(STD_OUTPUT_HANDLE);

    write("                                                                                       \n"
        "    .d8888b.   .d8888b.   .d88888b.  8888888b.  8888888888 .d8888b. Y88b   d88P        \n"
        "   d88P  Y88b d88P  Y88b d88P\" \"Y88b 888   Y88b 888       d88P  Y88b Y88b d88P       \n"
        "   888    888 Y88b.      888     888 888    888 888       Y88b.       Y88o88P          \n"
        "   888         \"Y888b.   888     888 888   d88P 8888888    \"Y888b.     Y888P         \n"
        "   888            \"Y88b. 888     888 8888888P\"  888           \"Y88b.    888          \n"
        "   888    888       \"888 888     888 888        888             \"888    888          \n"
        "   Y88b  d88P Y88b  d88P Y88b. .d88P 888        888       Y88b  d88P    888           \n"
        "    \"Y8888P\"   \"Y8888P\"   \"Y88888P\"  888        8888888888 \"Y8888P\"     888         \n"
        "                                                                                       \n",
        11);

    int P = 10; //color green
    SetConsoleTextAttribute(console_color, P);
    write("  Hello, Welcome to CSOPESY commandline!", 10);

    P = 14; //color yellow
    SetConsoleTextAttribute(console_color, P);
    write("  Type 'exit' to quit, 'clear' to clear the screen\n", 14);
}

/*void MainConsole::printMemoryInfo() {
    std::lock_guard<std::mutex> lock(queueMutex); // Ensure thread safety when accessing shared data

    std::ofstream outFile("memory_stamp.txt"); // Specify the file name
    if (!outFile.is_open()) {
        write("Failed to open file for writing.\n", 14);
        return; // Exit if the file cannot be opened
    }

    // Get the total memory from configuration
    int totalMemory = config.mom; // Total memory from the configuration
    if (totalMemory <= 0) {
        outFile << "Total memory is not set correctly.\n"; // Write to file instead of console
        outFile.close(); // Close the file
        return; // Exit if total memory is invalid
    }

    // Get the number of processes in memory
    const auto& runningProcesses = scheduler.getRunningProcesses();
    int numProcesses = runningProcesses.size(); // Number of processes in memory
    int allocatedMemory = 0; // To keep track of allocated memory
    int totalExternalFragmentationKB = 0; // To be calculated

    // Calculate allocated memory and total external fragmentation
    for (const auto& process : runningProcesses) {
        allocatedMemory += process.getSizeMem(); // Assuming each process has a method to get its size
    }
    totalExternalFragmentationKB = totalMemory - allocatedMemory;

    // Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time_t);

    // Create a string stream to format the timestamp
    std::stringstream timestampStream;
    timestampStream << std::put_time(&local_tm, "%m/%d/%Y %I:%M:%S %p");
    std::string currentTimestamp = timestampStream.str();

    // Print the memory information to the file
    outFile << "Timestamp: (" + currentTimestamp + ")\n";
    outFile << "Number of processes in memory: " + std::to_string(numProcesses) + "\n";
    outFile << "Total external fragmentation in KB: " + std::to_string(totalExternalFragmentationKB) + "\n\n";
    outFile << "------end----- = " + std::to_string(totalMemory) + "\n\n";

    // Print details of each process with memory limits
    int currentAddress = 0; // Start from address 0
    for (const auto& process : runningProcesses) {
        int lowerLimit = currentAddress; // Lower limit for the current process
        int upperLimit = currentAddress + process.getSizeMem(); // Upper limit for the current process

        outFile << totalMemory << "\n"; // Print total memory again for each process
        outFile << process.getName() << "\n"; // Print process name
        outFile << process.getSizeMem() << "\n"; // Print process size

        // Print memory limits
        outFile << "Lower Limit: " << lowerLimit << " KB\n"; // Lower limit
        outFile << "Upper Limit: " << upperLimit << " KB\n\n"; // Upper limit

        currentAddress += process.getSizeMem(); // Update current address for the next process
    }

    // Print external fragmentation
    outFile << "External Fragmentation: " << totalExternalFragmentationKB << " KB\n";

    outFile.close(); // Close the file after writing
} */

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize) : maximumSize(maximumSize), allocatedSize(0)
{
    memory.resize(maximumSize);

    initializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator()
{
    memory.clear();
}

FlatMemoryAllocator* FlatMemoryAllocator::flatMemoryAllocator = nullptr;

void FlatMemoryAllocator::initialize(size_t maximumMemorySize) {
    flatMemoryAllocator = new FlatMemoryAllocator(maximumMemorySize);
}

FlatMemoryAllocator* FlatMemoryAllocator::getInstance() {
    return flatMemoryAllocator;
}

std::mutex allocationMapMutex;  // Mutex for protecting allocationMap

void* FlatMemoryAllocator::allocate(size_t size, string process) {
    {
        std::lock_guard<std::mutex> lock(allocationMapMutex);  // Lock to ensure thread safety

        // Check for the availability of a suitable block
        for (size_t i = 0; i < maximumSize - size + 1; ++i) {
            // Check if the memory block is available
            if (allocationMap.find(i) == allocationMap.end() || allocationMap[i].empty()) {
                if (canAllocateAt(i, size)) {
                    // Ensure that the requested block doesn't go out of bounds
                    if (i + size <= maximumSize) {
                        allocateAt(i, size, process);
                        return &memory[i];  // Return pointer to allocated memory
                    }
                }
            }
        }
    }

    return nullptr;  // Return nullptr if allocation fails
}

void FlatMemoryAllocator::deallocate(void* ptr) {
    std::lock_guard<std::mutex> lock(allocationMapMutex);
    size_t index = static_cast<char*>(ptr) - &memory[0];
    if (allocationMap[index] != "") {
        deallocateAt(index);
    }
}

std::string FlatMemoryAllocator::visualizeMemory() {
    return std::string(memory.begin(), memory.end());
}

void FlatMemoryAllocator::initializeMemory() {
    // Initialize the memory vector with '.'
    memory.resize(maximumSize, '.');

    // Initialize the allocationMap with 'false' for all indices
    for (size_t i = 0; i < maximumSize; ++i) {
        allocationMap[i] = "";
    }
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) {
    return (index + size <= maximumSize);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size, string process) {
    // Fill allocation map with true values starting from index until the process size
    for (size_t i = index; i < index + size; ++i) {
        allocationMap[i] = process;
    }
    allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
    size_t size = config.mpp;
    for (size_t i = index; i < index + size && i < maximumSize; ++i) {
        allocationMap[i] = "";
    }
    allocatedSize -= size;
}

size_t FlatMemoryAllocator::calculateExternalFragmentation() {
    size_t externalFragmentation = 0;
    for (size_t i = 0; i < maximumSize; i += 1) {
        if (allocationMap[i] == "") {
            externalFragmentation++;
        }
    }
    return externalFragmentation;
}

size_t FlatMemoryAllocator::getNumberOfProcessesInMemory() {
    size_t allocatedBlocks = 0;

    {
        std::lock_guard<std::mutex> lock(allocationMapMutex);

        // Iterate through the allocation map and count allocated blocks
        for (size_t i = 0; i < allocationMap.size(); ++i) {
            //cout << "i: " << allocationMap[i] << endl;
            if (allocationMap.find(i) != allocationMap.end() && !allocationMap[i].empty()) {  // If the block is allocated
                allocatedBlocks++;
            }
        }
    }
    // Calculate the number of processes
    size_t processSizeInBlocks = config.mpp;  // Number of blocks per process
    return allocatedBlocks / processSizeInBlocks;  // Total number of processes in memory
}

void FlatMemoryAllocator::printMemoryInfo(int quantum_size) {
    static int curr_quantum_cycle = 0;  // Counter for unique file naming
    curr_quantum_cycle = curr_quantum_cycle + quantum_size;

    // Create a unique filename for each call
    std::string filename = "memory_stamp_" + std::to_string(curr_quantum_cycle) + ".txt";
    std::ofstream outFile(filename);  // Open a uniquely named file for writing

    if (!outFile) {  // Check if the file was successfully opened
        std::cerr << "Error opening file for writing.\n";
        return;
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::stringstream timestampStream;
    timestampStream << std::put_time(std::localtime(&now_time_t), "%m/%d/%Y  %H:%M:%S %p"); 
    std::string timestamp = timestampStream.str();

    size_t numProcessesInMemory = FlatMemoryAllocator::getInstance()->getNumberOfProcessesInMemory();

    // Print the information to the file
    outFile << "Timestamp: " << timestamp << "\n";
    outFile << "Number of processes in memory: " << numProcessesInMemory << "\n";
    outFile << "Total External fragmentation in KB: " << calculateExternalFragmentation() << "\n\n";
    outFile << "----end---- = " << maximumSize << "\n\n";

    std::string currentProcessName = "";
    std::string incomingProcessName = "";

    for (size_t i = maximumSize - 1; i > 0; i--) {
        incomingProcessName = allocationMap[i];
        if (currentProcessName.empty() && !incomingProcessName.empty()) {
            currentProcessName = incomingProcessName;
            outFile << i + 1 << "\n" << currentProcessName << "\n";
        }
        else if (currentProcessName != incomingProcessName) {
            outFile << i + 1 << "\n";
            currentProcessName = incomingProcessName;
            outFile << currentProcessName << "\n";
        }
        else if (incomingProcessName.empty() && i == maximumSize - 1) {
            outFile << i + 1 << "\n";
        }
    }
    outFile << "\n ----start---- = 0\n";

    outFile.close();  // Close the file
}

size_t FlatMemoryAllocator::getMaxSize() {
    return maximumSize;
};

size_t FlatMemoryAllocator::getUsage() {
    return allocatedSize;
};

unordered_map<size_t, string> FlatMemoryAllocator::getAllocMap() {
    return allocationMap;
}
