#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <Windows.h>
#include <ctime>
#include <iomanip>
using namespace std;

void printLayout(int pid[], std::string typ[], std::string name[], std::string usage[], int size) {

    cout << "+--------------------------------------------------------------------------------------------------+" << endl;

    // First row
    cout << "| NVIDIA-SMI 551.86                     Driver Version: 551.86             CUDA Version: 12.4      |" << endl;

    // Divider row
    cout << "+---------------------------------------------+----------------------------+-----------------------+" << endl;

    // Second row (Headers)
    cout << "| GPU      Name                      TCC/WDDM | Bus-Id         Disp.A      | Volatile Uncorr. ECC  |" << endl;
    cout << "| FAN      TEMP   PERF         Pwr: Usage/Cap |              Memory-Usage  | GPU-Util Compute M.   |" << endl;
    // Divider row
    cout << "+=============================================+============================+=======================+" << endl;

    // Third row (Data)
    cout << "|   0      NVIDIA GeForce GTX 1080     WDDM   | 00000000:26:00.0      On   |                  N/A  |" << endl;
    cout << "| 28%        37C   P8             11W /  180W |   701MiB      /   8192MiB  |   0%         Default  |" << endl;
    cout << "|                                             |                            |                  N/A  |" << endl;
    // Divider row
    cout << "+---------------------------------------------+----------------------------+-----------------------+" << endl;

    cout << "+---------------------------------------------+----------------------------+-----------------------+" << endl;
    cout << "|  Processes:                                                                                      |" << endl;
    cout << "|   GPU      GI   CI          PID   Type   Process name                                 GPU Memory |" << endl;
    cout << "|            ID   ID                                                                    Usage      |" << endl;
    cout << "|==================================================================================================|" << endl;
    for (int i = 0; i < size; i++) {
        // Convert the PID to a string for display purposes
        string pidStr = to_string(pid[i]);
        // Limit the PID to the first 4 characters for formatted output
        string pidLimited = pidStr.substr(0, 4);

        // Limit the width for type, taking the first 3 characters
        string type = typ[i].substr(0, 3);

        // Get the full process name for length checking
        string processName = name[i];

        // Check the length of processName
        if (processName.length() > 35) {
            // If longer than 35 characters, prepend "..." to the last 35 characters
            processName = "..." + processName.substr(processName.length() - 35);
        }

        // Limit memory usage to the first 10 characters for formatted output
        string memUsage = usage[i].substr(0, 10);

        // Print formatted output for each process
        // - %-4s is for left-aligned string output (PID, max 4 characters)
        // - %-3s is for left-aligned string output (Type, max 3 characters)
        // - %-38s is for left-aligned string output (Process Name, max 38 characters)
        // - %-10s is for left-aligned string output (Memory Usage, max 10 characters)
        printf("|    0      N/A   N/A         %-4s   %-3s   %-38s        %-10s|\n",
            pidLimited.c_str(), type.c_str(), processName.c_str(), memUsage.c_str());
    }

    cout << "+--------------------------------------------------------------------------------------------------+" << endl;

}

void printBuildDateTime() {
    cout << __TIMESTAMP__ << endl;
}

int main() {


    int pd[5] = { 4321, 5432, 6543, 7654,  1234 };
    std::string typ[5] = { "C+G", "C+G", "C+G", "C+G", "C+G" };
    std::string name[5] = { "Pogi", "ni", "sir", "wootwootwootwootwootwootwootwootwoo", "wootwootwootwootwootwootwootwootwootwootwoot.exe" };
    std::string usage[5] = { "N/A", "N/A", "N/A", "N/A", "N/A" };
    std::cout << "PS C:\\Users\\Ayushi> nvidia-smi" << std::endl;
    printBuildDateTime();
    printLayout(pd, typ, name, usage, 5);

}