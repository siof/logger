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

#include "logger.h"

#include <algorithm>
#include <cstdarg>
#include <iomanip>
#include <iostream>

namespace siof
{
    /// dtor
    Logger::~Logger()
    {
        Close();
    }

    /// Here you can set/change filename to save files
    void Logger::SetFileName(std::string & fileName, std::string extension)
    {
        std::lock_guard<std::mutex> guard(fileMutex_);

        fileName_ = fileName;
        fileExtension_ = extension;

        ReopenFile();
    }

    /// Returns actual file name
    const std::string & Logger::GetFileName() const
    {
        return fileName_;
    }

    /// returns if logging thread is potentially working
    bool Logger::IsWorking() const
    {
        // returns if thread is joinable (potentially running)
        return thread_ && thread_->joinable();
    }

    /// starts logging thread (if not started)
    void Logger::Start()
    {
        try
        {
            // try to close thread if is running
            Close();
            OpenFileIfNeeded(nullptr);

            // and start it again
            thread_ = std::shared_ptr<std::thread>(new std::thread(&Logger::LogWriter, this));
        }
        catch (std::exception & e)
        {
            WriteToStdOut(e.what());
        }
    }

    void Logger::Close()
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

    void Logger::AddMessage(LogLevel logLevel, const std::string & msg)
    {
        if (GetMinimalLogLevel() > logLevel)
            return;

        LogMsg * tmpMsg = nullptr;
        try
        {
            std::lock_guard<std::mutex> lock(dataMutex_);

            tmpMsg = new LogMsg(logLevel, msg);
            queued_.push_back(std::shared_ptr<LogMsg>(tmpMsg));

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

    void Logger::AddMessage(LogLevel logLevel, const char * fmt, ...)
    {
        if (GetMinimalLogLevel() > logLevel)
            return;

        char * buffer = nullptr;

        va_list lst;
        va_start(lst, fmt);

        try
        {
            buffer = new char[strlen(fmt) + 1024];

            vsprintf(buffer, fmt, lst);

            std::string tmpStr(buffer);
            AddMessage(logLevel, tmpStr);
        }
        catch (std::exception & e)
        {
            WriteToStdOut(e.what());
        }

        va_end(lst);

        if (buffer != nullptr)
            delete buffer;
    }

    void Logger::SetOption(LoggerOptions option, bool enabled)
    {
        options_[option] = enabled;
    }

    bool Logger::IsOptionSet(LoggerOptions option)
    {
        return options_[option];
    }

    void Logger::SetMinimalLogLevel(LogLevel logLevel)
    {
        minLogLevel_ = logLevel;
    }

    LogLevel Logger::GetMinimalLogLevel()
    {
        return minLogLevel_;
    }

    void Logger::OpenFileIfNeeded(const LogMsg * time)
    {
        bool openFile = !file_.is_open();

        if (!openFile && IsOptionSet(OPTION_FILE_ROLLING) && time)
        {
            std::time_t tmpTime = time->GetCTime();
            std::tm * tmpTm = std::localtime(&tmpTime);
            std::tm * tmpCurrTm = std::localtime(&fileOpenTime_);

            if (tmpTm && tmpCurrTm && tmpTm->tm_mday != tmpCurrTm->tm_mday)
                openFile = true;

            delete tmpTm;
            delete tmpCurrTm;
        }

        if (openFile)
            ReopenFile();
    }

    void Logger::ReopenFile()
    {
        if (file_.is_open())
            file_.close();

        fileOpenTime_ = std::time(nullptr);

        std::tm * tmpCurrTm = std::localtime(&fileOpenTime_);

        char date[12];
        memset(date, 0, sizeof(date));
        std::strftime(date, sizeof(date), "_%Y_%m_%d", tmpCurrTm);

        delete tmpCurrTm;

        std::string destFileName = fileName_;
        destFileName += date;
        destFileName += "." + fileExtension_;

        file_.open(destFileName, std::ios_base::app);

        file_ << std::setfill('0') << std::endl << separator_ << std::endl << std::endl;
    }

    void Logger::WriteToStdOut(const std::string & str)
    {
        WriteToStdOut(str.c_str());
    }

    void Logger::WriteToStdOut(const char * str, ...)
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

    void Logger::LogWriter()
    {
        closing_ = false;
        bool justStarted = true;
        LoggerMsgList localList;

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

            localList.swap(queued_);

            dataMutex_.unlock();

            std::lock_guard<std::mutex> guard(fileMutex_);

            std::for_each(localList.begin(), localList.end(), [this] (std::shared_ptr<LogMsg> &p) -> void
                                                                {
                                                                    OpenFileIfNeeded(p.get());
                                                                    file_ << *p;
                                                                });

            file_.flush();

            localList.clear();

            if (closing_)
            {
                std::lock_guard<std::mutex> lock(dataMutex_);
                if (queued_.empty())
                    return;
            }
        }
    }
}
