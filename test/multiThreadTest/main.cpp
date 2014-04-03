#include <algorithm>
#include <functional>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include <string>
#include <vector>

#include <sLog.h>
#include <sLogMsg.h>

#define THREAD_COUNT        100
#define LOG_MSG_PER_THREAD  1000
#define LOG_WAIT            0

SLog logFile;

void log(int msgCount, int waitTime)
{
    std::thread::id threadId = std::this_thread::get_id();
    for (int i = 0; i < msgCount; ++i)
    {
        if (waitTime > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(waitTime));

        logFile.AddMessage("thread %i iter %i", threadId.hash(), i);
    }
}

void log2(int msgCount, int waitTime, std::ofstream * file)
{
    std::thread::id threadId = std::this_thread::get_id();
    SLogTimePoint time;
    for (int i = 0; i < msgCount; ++i)
    {
        if (waitTime > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(waitTime));

        time = std::chrono::system_clock::now();
        std::time_t tmpTime = std::chrono::system_clock::to_time_t(time);
        // miliseconds
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time - std::chrono::system_clock::from_time_t(tmpTime));

        *file << std::put_time(std::localtime(&tmpTime), "%Y-%m-%d %H:%M:%S.");
        *file << std::setw(3) << ms.count();
        *file << std::setw(0) << " - thread " << threadId << " iter" << i << "\n";//std::endl;
    }
}

int main(int argc, char * argv[])
{
    int threadCount = THREAD_COUNT;
    int logMsgPerThread = LOG_MSG_PER_THREAD;
    int logWait = LOG_WAIT;
    std::string fileName("multiThreadTest.txt");

    std::vector<std::thread> threads;

    if (argc > 1)
        threadCount = atoi(argv[1]);

    if (argc > 2)
        logMsgPerThread = atoi(argv[2]);

    if (argc > 3)
        logWait = atoi(argv[3]);

    auto startThreaded = std::chrono::high_resolution_clock::now();

    logFile.SetFileName(fileName);
    logFile.Start();

    for (int i = 0; i < threadCount; ++i)
        threads.push_back(std::thread(std::bind(log, logMsgPerThread, logWait)));

    std::for_each (threads.begin(), threads.end(), [](std::thread & thr) -> void { if (thr.joinable()) thr.join();});

    logFile.Close();

    auto endThreaded = std::chrono::high_resolution_clock::now();

    threads.clear();

    auto start = std::chrono::high_resolution_clock::now();

    std::ofstream file(fileName + "2");

    for (int i = 0; i < threadCount; ++i)
        threads.push_back(std::thread(std::bind(log2, logMsgPerThread, logWait, &file)));

    std::for_each (threads.begin(), threads.end(), [](std::thread & thr) -> void { if (thr.joinable()) thr.join();});

    file.flush();

    file.close();

    auto end = std::chrono::high_resolution_clock::now();

    uint64_t tmpt1 = std::chrono::duration_cast<std::chrono::milliseconds>(endThreaded - startThreaded).count();
    uint64_t tmpt2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double permsg1 = double(tmpt1)/(logMsgPerThread*threadCount);
    double permsg2 = double(tmpt2)/(logMsgPerThread*threadCount);
    std::cout << "Counted (in milliseconds) for: " << threadCount << " threads with writing " << logMsgPerThread << " msg's per thread and " << logWait << " log wait time" << std::endl;
    std::cout << "threaded: " << tmpt1 << " per msg: " << permsg1 << std::endl;
    std::cout << "mono ms: " << tmpt2 << " per msg: " << permsg2 << std::endl;
    std::cout << "diff: " << tmpt1 - tmpt2 << " per msg: " << permsg1 - permsg2 << std::endl;

    return 0;
}
