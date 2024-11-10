#pragma once
#pragma once
#ifndef HEADERS_H
#define HEADERS_H
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifdef _WIN32
#include <windows.h>
#else
typedef unsigned short WORD;
#endif

#include <string>
#include <vector>
#include <mutex>
#include <set>
#include <map>
#include <ctime>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <thread>
#include <condition_variable>
using namespace std;

class Scheduler;

struct Config {
    int cpu;              // Number of CPUs
    std::string algorithm;     // Scheduling algorithm
    int qCycles;         // Quantum cycles
    int bpf;             // Batch process frequency
    int mins;            // Minimum instructions
    int maxins;          // Maximum instructions
    int dpe;             // Delay per execution

    // Constructor to initialize default values
    Config() : cpu(0), qCycles(0), bpf(0), mins(0), maxins(0), dpe(0) {}
};
// Structure for buffer entry
struct BufferEntry {
    std::string text;
    WORD color;
};

// Base class for screen management
class AbstractScreen {
protected:
    std::vector<BufferEntry> buffer;
    std::mutex console_mutex;

    void write(const std::string& text, WORD color = 7);
public:
    virtual void redraw();
};

// Derived class for handling screen processes
class Screen : public AbstractScreen {
private:
    std::string processName;
    int currentLine;
    int totalLine;
    time_t timestamp;
    int processID;

    std::stringstream printScreen_helper();

public:
    Screen(int processID, std::string name, int currentLine, int totalLine);
    Screen(int processID, std::string name, int totalLine);
    Screen(std::string name);
    Screen(const Screen& other);
    Screen& operator=(const Screen& other);
    Screen();

    std::string getProcessName() const;
    int getCurrentLine() const;
    int getTotalLine() const;
    time_t getTimestamp() const;
    bool operator<(const Screen& other) const;
    void setCurrentLine(int currentLine);

    void openScreen();
    void redraw() override;//CHANGED TO override
    bool screenCommand(std::vector<std::string> seperatedCommand, std::string command_to_check, Scheduler* scheduler);//CHANGED TO bool from boolean
};


// ICommand base class for commands (Print, IO, etc.)
class ICommand {
public:
    enum CommandType { IO, PRINT };
    ICommand(int pid, CommandType commandType);

    CommandType getCommandType() const;
    virtual void execute(int coreID) = 0;  // Pure virtual function
    int getPID() {
        return pid;
    }
protected:
    int pid;
    CommandType commandType;
};



// Derived PrintCommand class
class PrintCommand : public ICommand {
private:
    std::string toPrint;
    std::string timestamp;
public:
    PrintCommand(int pid, const std::string& timestamp, const std::string& toPrint);
    void execute(int coreID) override;
};

// Process class to handle process attributes and commands
class Process {
public:
    struct RequirementFlags {
        bool needFiles;
        int amtFiles;
        bool needMem;
        int amtMem;
    };
    enum ProcessState { READY, RUNNING, WAITING, FINISHED };

    Process(int pid, std::string name, RequirementFlags reqFlags, int commandCounter);//experiment from READY
    void addCommand(ICommand::CommandType commandType);
    void executeCurrentCommand(int coreID, std::chrono::milliseconds dpe);
    bool isFinished() const;

    int getPID() const {
        return pid;
    };
    std::string getName() const {
        return name;
    };
    int getCurrentIteration() const {
        return commandCounter;
    };
    void setCurrentIteration(int commandCounter) {
        this->commandCounter = commandCounter;
    }
    int incrementIteration() {
        if (commandCounter < commandList.size()) {
            return commandCounter++;
        }
    }
    ProcessState getState() const {
        return state;
    };

    int getCoreID() const {
        return coreID;
    };
    string getCurrentTime() const {
        return current_timestamp;
    }
    void setCurrentTime(string currentTime) {
        this->current_timestamp = currentTime;
    }
    void setCoreID(int coreID) {
        this->coreID = coreID;
    };
    std::vector<std::shared_ptr<ICommand>> commandList;
private:
    int pid;
    std::string name;
    int coreID;
    string current_timestamp;

    int commandCounter;
    RequirementFlags reqFlags;
    ProcessState state;
};

// Scheduler class to manage process execution
class Scheduler {
public:
    Process getNextProcess();
    int checkCurrentProcess(string processName);
    int ifProcessExists(string processName);
    Process getCurrentProcess(string processName);
    void addProcess(Process& process);
    enum class SchedulerAlgorithm { FCFS, RR };
    void start(int amtCpu, SchedulerAlgorithm schedulerAlgorithm, std::chrono::milliseconds timeQuantum, std::chrono::milliseconds dpe);// Start scheduler and workers
    void stop();// Stop scheduler and workers

    int amtCpu;
    std::chrono::milliseconds timeQuantum/*{ 3000 }*/;//ORIG: 50
    int bpf;
    int mins;
    int maxins;
    std::chrono::milliseconds dpe;

    std::deque<Process> getAllProcesses() {
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
        /*for (const auto& process : runningProcesses) {
            std::cout << "  Name: " << process.getName()
                << " | " << process.getCurrentTime()
                << " | Core: " << process.getCoreID()
                << " | " << process.getCurrentIteration() << " / " << process.commandList.size() << " | " << std::endl;
        }*/

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
                    << " | " << it->getCurrentIteration() << " / " << it->commandList.size() << " | " << std::endl;
            }

            //CHANGE end
        }
        std::cout << "  _______________________________________\n" << std::endl;

        return allProcesses;
    }
    std::deque<Process> printAllProcesses();
    std::vector<Process>& getRunningProcesses() { // Add function to retrieve running processes
        std::lock_guard<std::mutex> lock(queueMutex);
        return runningProcesses;
    };
private:
    std::deque<Process> processQueue;
    std::vector<Process> runningProcesses;
    std::vector<Process> finishedProcesses;   // Store finished processes
    std::vector<std::thread> coreThreads;    // CPU worker threads
    bool stopScheduler;                      // Flag to stop the scheduler
    std::mutex queueMutex;
    std::condition_variable cv;
    std::thread schedulerThread;             // Dedicated scheduler thread

    // Worker thread method
    void schedulerLoop();                    // Main scheduler loop
    void runCore(int coreID, SchedulerAlgorithm algorithm, std::chrono::milliseconds timeQuantum, std::chrono::milliseconds dpe);

};

// Class for handling the main console commands and screens
class MainConsole : public AbstractScreen {
private:
    int doneInitialize = 0;
    std::string currentView;
    bool continue_program;
    std::map<std::string, Screen> screensAvailable;
    Scheduler scheduler;
    std::atomic<bool> stop_scheduler{ false };
    int cycleInterval;  // Number of cycles between process generation
    std::queue<Screen> readyQueue;
    std::thread schedulerThread;

    void commandRecognize(std::string command_to_check);
    void commandNotRecognize(std::string command_to_check);
    void invalidCommand(std::string command_to_check);
    void screenNotFound();
    void newProcess(int mins, int maxins);
    void initializeScheduler(int mins, int maxins);
    void schedulerTest();
    void checkForStopCommand();
    void stopScheduler();
    void loadConfig();
    bool mainMenuCommand(std::vector<std::string> seperatedCommand, std::string command_to_check);
    bool processCommand(const std::string& command);
    void setConsoleColor(int color);
    void print_header();

public:
    void run();
};

#endif // HEADERS_H
