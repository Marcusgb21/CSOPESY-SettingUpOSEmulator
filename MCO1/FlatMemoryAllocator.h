#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
using namespace std;

class FlatMemoryAllocator
{
public:
    FlatMemoryAllocator(size_t totalMemoryPaging, size_t frameSizePaging);
    ~FlatMemoryAllocator();


    void* allocate(size_t size, string process);
    void deallocate(void* ptr);
    std::string visualizeMemory();
    void initializeMemory();
    bool canAllocateAt(size_t index, size_t size);
    void allocateAt(size_t index, size_t size, string processName);
    void deallocateAt(size_t index);
    static FlatMemoryAllocator* getInstance();
    static void initialize(size_t maximumMemorySize, size_t frames);
    FlatMemoryAllocator() : maximumSize(0), allocatedSize(0), frameSizePaging(0) {};

    void printMemoryInfo(int quantum);
    size_t calculateExternalFragmentation();
    size_t getNumberOfProcessesInMemory();
    size_t getMaxSize();
    size_t getUsage();
    unordered_map<size_t, string> getAllocMap();

    //paging

    bool allocatePagesPaging(int pid, size_t pagesNeededPaging);
    void deallocatePagesPaging(int pid);
    size_t getPagesRequiredPaging(size_t memoryPaging) const;

    size_t getTotalMemoryPaging() const;
    size_t getUsedMemoryPaging() const;
    size_t getFreeMemoryPaging() const;
    void initializePaging(size_t totalMemoryPaging, size_t frameSizePaging);
    void initializeMemoryPaging();
    int pageIn = 0;
    int pageOut = 0;
    size_t actualpagesNeededPaging;
    void clearMap() {
        allocationMap.clear();
    }
private:
    static FlatMemoryAllocator* flatMemoryAllocator;
    size_t maximumSize;
    size_t allocatedSize;
    std::vector<char> memory;
    std::unordered_map<size_t, string> allocationMap;

    //paging
    size_t totalFramesPaging = 0;
    size_t frameSizePaging;
    size_t usedFramesPaging = 0;
    std::vector<int> memoryPaging;  // -1 for free, otherwise stores the PID

};