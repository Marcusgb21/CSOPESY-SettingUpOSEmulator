#ifndef HEADERS_H
#define HEADERS_H

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include <set>
#include <thread>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <condition_variable>

// Struct to hold text and color for buffer entries
struct BufferEntry {
    std::string text;
    WORD color;
};

// Abstract screen class to handle basic console operations
class AbstractScreen {
protected:
    std::vector<BufferEntry> buffer;
    std::mutex console_mutex;

    void write(const std::string& text, WORD color = 7);
public:
    virtual void redraw();
};

// Derived Screen class for specific screen operations
class Screen : public AbstractScreen {
private:
    std::string processName;
    int currentLine, totalLine;
    time_t timestamp;
public:
    Screen(std::string name, int currentLine = 1, int totalLine = 100);
    Screen(const Screen& other);  // Copy constructor
    Screen& operator=(const Screen& other);  // Assignment operator

    std::string getProcessName() const;
    void openScreen();
    void redraw() override;
    bool screenCommand(std::vector<std::string> separatedCommand, std::string command);
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
    void executeCurrentCommand(int coreID);
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
        if (commandCounter < 100) {
            return commandCounter++;
        }
    }
    ProcessState getState() const {
        return state;
    };

    int getCoreID() const {
        return coreID;
    };

    void setCoreID(int coreID) {
        this->coreID = coreID;
    };

private:
    int pid;
    std::string name;
    int coreID;
    std::vector<std::shared_ptr<ICommand>> commandList;
    int commandCounter;
    RequirementFlags reqFlags;
    ProcessState state;
};

// Scheduler class to manage process execution
class Scheduler {
private:
    std::deque<Process> processQueue;
    std::vector<Process> runningProcesses;
    std::vector<Process> finishedProcesses;   // Store finished processes
    std::vector<std::thread> coreThreads;    // CPU worker threads
    bool stopScheduler;                      // Flag to stop the scheduler
    std::mutex queueMutex;
    std::condition_variable cv;
    std::thread schedulerThread;             // Dedicated scheduler thread

    void runCore(int coreID);                // Worker thread method
    void schedulerLoop();                    // Main scheduler loop
public:
    Process getNextProcess();                // Method to get next process in FCFS
    void addProcess(Process& process);       // Add process to the queue
    void start();                            // Start scheduler and workers
    void stop();                             // Stop scheduler and workers
    std::deque<Process> getAllProcesses() {
        std::lock_guard<std::mutex> lock(queueMutex);
        std::deque<Process> allProcesses;

        int totalCores = coreThreads.size();  // Total number of cores
        int usedCores = runningProcesses.size();  // Active or running processes
        int availableCores = totalCores - usedCores;

        // Display cores usage
        std::cout << std::endl << "  CPU Utilization: 100%" << std::endl;
        std::cout << "  Cores used: " << usedCores << std::endl;
        std::cout << "  Cores available: " << availableCores << std::endl;
        std::cout << "  _______________________________________" << std::endl;

        // Display running processes
        std::cout << "  Running Processes:" << std::endl;
        for (const auto& process : runningProcesses) {
                std::cout << "  Name: " << process.getName()
                << " | Core: " << process.getCoreID()
                << " | " << process.getCurrentIteration() << " / 100 |" << std::endl;
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

            for (int i = 0; i < 10; i++) {
                for (it = finishedProcesses.begin(); it != finishedProcesses.end(); it++) {
                    // found nth element..print and break.
                    if (i == it->getPID()) {
                            std::cout << "  Name: " << it->getName()
                            << " | Finished"
                            << " | " << it->getCurrentIteration() << " / 100 |" << std::endl;
                        break;
                    }
                }
            }
            //CHANGE end
        }
        std::cout << "  _______________________________________" << std::endl;

        return allProcesses;
    }
    std::vector<Process>& getRunningProcesses() { // Add function to retrieve running processes
        std::lock_guard<std::mutex> lock(queueMutex);
        return runningProcesses;
    };

    std::vector<Process> getProcesses() {
        std::lock_guard<std::mutex> lock(queueMutex);
        std::vector<Process> allProcesses(runningProcesses.begin(), runningProcesses.end());



        while (!processQueue.empty()) {
            allProcesses.push_back(processQueue.front());
            processQueue.pop_front();
        }

        return allProcesses;
    }
    void displayFinishedProcesses();         // Display finished processes
};

// MainConsole class for handling user interaction and scheduling
class MainConsole : public AbstractScreen {
private:
    std::map<std::string, Screen> screensAvailable;
    std::string currentView;
    bool continueProgram;

    Scheduler scheduler;
    void setConsoleColor(int color);
    void printHeader();
    bool processCommand(const std::string& command);

public:
    void run();
};

#endif // HEADERS_H
