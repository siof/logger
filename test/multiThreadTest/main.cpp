#include <algorithm>
#include <functional>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
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

        logFile.AddMessage(SLOG_LEVEL_DEBUG, "thread %i iter %i", threadId.hash(), i);
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
        std::time_t tmpTime = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
        // miliseconds
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time - std::chrono::system_clock::from_time_t(tmpTime));

        *file << std::put_time(std::localtime(&tmpTime), "%Y-%m-%d %H:%M:%S.");
        *file << std::setw(3) << ms.count();
        *file << std::setw(0) << " - thread " << threadId << " iter " << i << "\n";//std::endl;
    }
}

std::mutex fileMutex;

void log3(int msgCount, int waitTime, std::ofstream * file)
{
    std::thread::id threadId = std::this_thread::get_id();
    SLogTimePoint time;
    for (int i = 0; i < msgCount; ++i)
    {
        if (waitTime > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(waitTime));

        std::lock_guard<std::mutex> guard(fileMutex);
        time = std::chrono::system_clock::now();
        std::time_t tmpTime = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
        // miliseconds
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time - std::chrono::system_clock::from_time_t(tmpTime));

        *file << std::put_time(std::localtime(&tmpTime), "%Y-%m-%d %H:%M:%S.");
        *file << std::setw(3) << ms.count();
        *file << std::setw(0) << " - thread " << threadId << " iter " << i << "\n";//std::endl;
        //file->flush();
    }
}

int main(int argc, char * argv[])
{
    int threadCount = THREAD_COUNT;
    int logMsgPerThread = LOG_MSG_PER_THREAD;
    int logWait = LOG_WAIT;
    std::string fileName("multiThreadTest");

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

    auto endThreaded = std::chrono::high_resolution_clock::now();

    logFile.Close();

    auto safeclosed = std::chrono::high_resolution_clock::now();

    threads.clear();

    auto start = std::chrono::high_resolution_clock::now();

    std::ofstream file(fileName + "2");

    for (int i = 0; i < threadCount; ++i)
        threads.push_back(std::thread(std::bind(log2, logMsgPerThread, logWait, &file)));

    std::for_each (threads.begin(), threads.end(), [](std::thread & thr) -> void { if (thr.joinable()) thr.join();});

    auto end = std::chrono::high_resolution_clock::now();

    file.flush();

    file.close();

    auto blockedstart = std::chrono::high_resolution_clock::now();

    file.open(fileName + "3");

    for (int i = 0; i < threadCount; ++i)
        threads.push_back(std::thread(std::bind(log3, logMsgPerThread, logWait, &file)));

    std::for_each (threads.begin(), threads.end(), [](std::thread & thr) -> void { if (thr.joinable()) thr.join();});

    auto blockedend = std::chrono::high_resolution_clock::now();

    file.flush();

    file.close();

    int64_t tmpt1 = std::chrono::duration_cast<std::chrono::milliseconds>(endThreaded - startThreaded).count();
    int64_t tmpt11 = std::chrono::duration_cast<std::chrono::milliseconds>(safeclosed - endThreaded).count();
    int64_t tmpt2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    int64_t tmpt3 = std::chrono::duration_cast<std::chrono::milliseconds>(blockedend - blockedstart).count();

    double permsg1 = double(tmpt1)/(logMsgPerThread*threadCount);
    double permsg2 = double(tmpt2)/(logMsgPerThread*threadCount);
    double permsg3 = double(tmpt3)/(logMsgPerThread*threadCount);

    std::cout << "Counted (in milliseconds) for: " << threadCount << " threads with writing " << logMsgPerThread << " msg's per thread and " << logWait << " log wait time" << std::endl;
    std::cout << "safe threaded: " << tmpt1 << " per msg: " << permsg1 << "waiting for log thread to close: " << tmpt11 << std::endl;
    std::cout << "unsafe threaded ms: " << tmpt2 << " per msg: " << permsg2 << std::endl;
    std::cout << "block threaded ms: " << tmpt3 << " per msg: " << permsg2 << std::endl;
    std::cout << "diff safe - unsafe: " << tmpt1 - tmpt2 << " per msg: " << permsg1 - permsg2 << std::endl;
    std::cout << "diff blocked - safe: " << tmpt3 - tmpt1 << " per msg: " << permsg3 - permsg1 << std::endl;

    return 0;
}
