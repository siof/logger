#include <algorithm>
#include <functional>
#include <thread>
#include <string>
#include <vector>

#include <sLog.h>

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

    logFile.SetFileName(fileName);
    logFile.Start();

    for (int i = 0; i < threadCount; ++i)
        threads.push_back(std::thread(std::bind(log, logMsgPerThread, logWait)));

    std::for_each (threads.begin(), threads.end(), [](std::thread & thr) -> void { if (thr.joinable()) thr.join();});

    logFile.Close();

    return 0;
}
