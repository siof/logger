/*
*    SLogger - Simple/Safe(thread safe)/siof(?) Logger
*    Copyright (C) 2014 siof
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License version 3 as
*    published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sLog.h"

#include <algorithm>
#include <cstdarg>
#include <iomanip>
#include <iostream>

#include "sLogMsg.h"

/// dtor
SLog::~SLog()
{
    Close();
}

/// Here you can set/change filename to save files
void SLog::SetFileName(std::string & fileName)
{
    std::lock_guard<std::mutex> guard(fileMutex_);

    fileName_ = fileName;

    ReopenFile();
}

/// Returns actual file name
const std::string & SLog::GetFileName() const
{
    return fileName_;
}

/// returns if logging thread is potentially working
bool SLog::IsWorking() const
{
    // returns if thread is joinable (potentially running)
    return thread_ && thread_->joinable();
}

/// starts logging thread (if not started)
void SLog::Start()
{
    try
    {
        // try to close thread if is running
        Close();
        ReopenFile();

        // and start it again
        thread_ = std::shared_ptr<std::thread>(new std::thread(&SLog::LogWriter, this));
    }
    catch (std::exception & e)
    {
        WriteToStdOut(e.what());
    }
}

void SLog::Close()
{
    if (!closeMutex_.try_lock())
        return;

    if (thread_ && thread_->joinable())
    {
        closing_ = true;
        canLog_.notify_all();

        thread_->join();
        thread_.reset();
        file_.flush();
        file_.close();
    }

    closeMutex_.unlock();
}

void SLog::AddMessage(const std::string & msg)
{
    SLogMsg * tmpMsg = nullptr;
    try
    {
        std::lock_guard<std::mutex> lock(dataMutex_);

        tmpMsg = new SLogMsg(msg);
        queued_.push_back(std::shared_ptr<SLogMsg>(tmpMsg));

        canLog_.notify_one();
    }
    catch (std::exception & e)
    {
        WriteToStdOut(e.what());

        // just in case
        if (tmpMsg != nullptr)
            delete tmpMsg;
    }
}

void SLog::AddMessage(const char * fmt, ...)
{
    char * buffer = nullptr;

    va_list lst;
    va_start(lst, fmt);

    try
    {
        buffer = new char[strlen(fmt) + 1024];

        vsprintf(buffer, fmt, lst);

        std::string tmpStr(buffer);
        AddMessage(tmpStr);
    }
    catch (std::exception & e)
    {
        WriteToStdOut(e.what());
    }

    va_end(lst);

    if (buffer != nullptr)
        delete buffer;
}

void SLog::OpenFileIfNeeded(const SLogMsg * time)
{
    if (!file_.is_open())
        ReopenFile();
}

void SLog::ReopenFile()
{
    if (file_.is_open())
        file_.close();

    fileOpenTime_ = std::time(nullptr);

    file_.open(fileName_, std::ios_base::app);

    file_ << std::setfill('0') << std::endl << separator_ << std::endl << std::endl;
}

void SLog::WriteToStdOut(const std::string & str)
{
    WriteToStdOut(str.c_str());
}

void SLog::WriteToStdOut(const char * str, ...)
{
    char * buffer = nullptr;

    va_list lst;
    va_start(lst, str);

    try
    {
        buffer = new char[strlen(str) + 1024];

        vsprintf(buffer, str, lst);

        std::cout << buffer << std::endl;
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
    }

    va_end(lst);

    if (buffer != nullptr)
        delete buffer;
}

void SLog::LogWriter()
{
    closing_ = false;
    bool justStarted = true;
    SLogMsgList localList;

    while (true)
    {
        dataMutex_.lock();
        if (!closing_ && queued_.empty())
        {
            dataMutex_.unlock();

            std::unique_lock<std::mutex> lock(logMutex_);
            canLog_.wait(lock);

            dataMutex_.lock();
        }

        //dataMutex_.lock();

        localList.swap(queued_);

        dataMutex_.unlock();

        std::lock_guard<std::mutex> guard(fileMutex_);

        std::for_each(localList.begin(), localList.end(), [this] (std::shared_ptr<SLogMsg> &p) -> void
                                                            {
                                                                OpenFileIfNeeded(p.get());
                                                                file_ << *p;
                                                            });

        localList.clear();

        file_.flush();

        if (closing_)
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            if (queued_.empty())
                return;
        }
    }
}
