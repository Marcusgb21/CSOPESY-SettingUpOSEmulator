#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <conio.h>  // for _kbhit() and _getch()
#include <cstdlib>  // for rand() and srand()
#include <ctime>    // for time()
#include <string>   // for std::string
#include <vector>   // for std::vector

using namespace std;

class ConsoleManager {
public:
    static void clearConsole() {
        // Clears the console screen
        system("CLS");
    }

    static void setCursorPosition(int x, int y) {
        // Moves the cursor to the specified x, y position
        printf("\033[%d;%dH", y + 1, x + 1);
    }

    static void checkBounds(int& x, int& y, int maxWidth, int maxHeight, int& dx, int& dy) {
        // Bounce back if the text reaches the console edges
        if (x >= maxWidth || x < 0) dx = -dx;
        // Prevent hitting the top (2) and bottom (height - 2)
        if (y >= maxHeight - 3 || y < 3) dy = -dy;
    }

    static bool isKeyHit() {
        return _kbhit();
    }

    static char getKey() {
        return _getch();
    }
};

class MarqueeWorkerThread {
public:
    string marqueeText;  // Made public to allow direct access for updating
    string headerText;
    int posX, posY;
    int width, height;
    int dx, dy;  // Directions for diagonal movement
    atomic<bool> running;

    MarqueeWorkerThread(string header, string text, int screenWidth, int screenHeight)
        : headerText(header), marqueeText(text), posX(0), posY(2), width(screenWidth), height(screenHeight), running(true) {
        srand(static_cast<unsigned int>(time(0)));  // Seed the random generator
        dx = (rand() % 2 == 0) ? 1 : -1;  // Randomly set initial x direction
        dy = (rand() % 2 == 0) ? 1 : -1;  // Randomly set initial y direction
    }

    void stop() {
        running = false;
    }

    void run() {
        while (running) {
            ConsoleManager::clearConsole();
            printHeader();
            printMarquee();
            this_thread::sleep_for(chrono::milliseconds(100));

            // Move diagonally
            posX += dx;
            posY += dy;

            // Check bounds and reverse direction if necessary
            ConsoleManager::checkBounds(posX, posY, width, height, dx, dy);
        }
    }

    void printHeader() {
        // Print the header text at the top
        ConsoleManager::setCursorPosition(0, 0);
        cout << headerText;
    }

    void printMarquee() {
        // Print the marquee text at the current position
        ConsoleManager::setCursorPosition(posX, posY);
        cout << marqueeText;

        // Always show the command prompt at the bottom
        ConsoleManager::setCursorPosition(0, height - 2);
        cout << "Enter a command for MARQUEE_CONSOLE: ";
    }
};

class MarqueeConsole {
    MarqueeWorkerThread* workerThread;
    thread* worker;
    atomic<bool> isRunning;
    string userInput;
    vector<string> commandHistory; // Store previous commands

public:
    MarqueeConsole(string header, string text, int screenWidth, int screenHeight) {
        workerThread = new MarqueeWorkerThread(header, text, screenWidth, screenHeight);
        worker = nullptr;
        isRunning = true;
        userInput = "";
    }

    ~MarqueeConsole() {
        stop();
        delete workerThread;
    }

    void startThreaded() {
        worker = new thread(&MarqueeWorkerThread::run, workerThread);
    }

    void captureKeyboardInput() {
        while (isRunning) {
            if (ConsoleManager::isKeyHit()) {
                char ch = ConsoleManager::getKey();
                if (ch == 13) {  // Enter key
                    // Add the command to history
                    commandHistory.push_back(userInput);
                    userInput.clear();  // Clear the input after pressing Enter
                }
                else if (ch == 'q' || ch == 'Q') {
                    isRunning = false;
                    workerThread->stop();
                }
                else if (ch == 8) { // Backspace
                    if (!userInput.empty()) {
                        userInput.pop_back(); // Remove last character
                    }
                }
                else {
                    userInput += ch;  // Append character to user input
                }
            }

            // Update the command prompt with the current user input
            ConsoleManager::setCursorPosition(0, workerThread->height - 2);
            cout << "Enter a command for MARQUEE_CONSOLE: " << userInput;

            // Show the command history below the input
            int commandOffset = 0; // Offset for command history display
            for (size_t i = 0; i < commandHistory.size(); ++i) {
                ConsoleManager::setCursorPosition(0, workerThread->height - 1 + commandOffset);
                cout << "Command processed in MARQUEE_CONSOLE: " << commandHistory[i];
                commandOffset++;
            }

            this_thread::sleep_for(chrono::milliseconds(100));  // Adjusted delay for smoother refresh
        }
    }

    void stop() {
        if (worker != nullptr) {
            workerThread->stop();
            worker->join();
            delete worker;
        }
    }

    void run() {
        startThreaded();
        captureKeyboardInput();
        stop();
    }
};

int main() {
    int screenWidth = 80;   // Assume a console width of 80 characters
    int screenHeight = 30;  // Assume a console height of 25 lines
    string headerText = "*****************************************\n * Displaying a marquee console *\n*****************************************";
    string marqueeText = "Hello world in Marquee!";

    MarqueeConsole marqueeConsole(headerText, marqueeText, screenWidth, screenHeight);

    // Start in threaded mode
    marqueeConsole.run();

    return 0;
}

