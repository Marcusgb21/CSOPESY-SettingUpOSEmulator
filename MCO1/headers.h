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
#include <unordered_map>
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
    size_t mom;
    size_t mpf;
    size_t mpp;
    size_t maxmpp;
    size_t minmpp;

    // Constructor to initialize default values
    Config() : cpu(0), qCycles(0), bpf(0), mins(0), maxins(0), dpe(0), mom(0), mpf(0), mpp(0), minmpp(0), maxmpp(0) {}
};

// Structure for buffer entry
struct BufferEntry {
    std::string text;
    WORD color;
};

struct AllocationRecord {
    size_t startBlock; // Starting block index of the allocation
    size_t size;       // Size of the allocation in bytes or blocks
    std::string name;  // Name of process that owns the allocation
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
    size_t getSizeMem() const {
        return sizeMem;
    }
    std::vector<std::shared_ptr<ICommand>> commandList;

    void setSizeMem(size_t sizeOfProc) {
        sizeMem = sizeOfProc;
    }

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
    void start(int amtCpu, SchedulerAlgorithm schedulerAlgorithm, int timeQuantum, std::chrono::milliseconds dpe);// Start scheduler and workers
    void stop();// Stop scheduler and workers
    size_t generateRandomMemory(size_t minMem, size_t maxMem);
    int amtCpu;
    std::chrono::milliseconds timeQuantum/*{ 3000 }*/;//ORIG: 50
    int bpf;
    int mins;
    int maxins;
    std::chrono::milliseconds dpe;
    deque<Process> getAllProcesses();
    deque<Process> printAllProcesses();
    vector<Process>& getRunningProcesses();
    void printMemory();
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
    void runCore(int coreID, SchedulerAlgorithm algorithm, int timeQuantum, std::chrono::milliseconds dpe);

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
    void initializeScheduler(int mins, int maxins, int mpp);
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
