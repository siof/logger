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

#ifndef SIOF_LOGGER
#define SIOF_LOGGER

#include <array>
#include <atomic>
#include <condition_variable>
#include <ctime>
#include <fstream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "logMsg.h"

namespace siof
{
    #define SLOG_SEP_DEFAULT    "##########################################################################\n"\
                                "##########################################################################"

    typedef std::list<std::shared_ptr<LogMsg> > LoggerMsgList;

    enum LoggerOptions
    {
        OPTION_FILE_ROLLING     = 0,

        OPTIONS_COUNT
    };

    class Logger
    {
    public:
        /// only creates logger
        Logger() : closing_(false), minLogLevel_(SLOG_LEVEL_NONE), separator_(SLOG_SEP_DEFAULT), fileOpenTime_(0) {}
        /// dtor
        ~Logger();

        /// Here you can set/change filename to save files
        /// function automatically closes old file and reopen new
        void SetFileName(std::string & fileName, std::string extension = "log");
        ///
        void SetSeparator(std::string & separator);
        /// Returns actual file name
        const std::string & GetFileName() const;

        /// returns if logging thread is working
        bool IsWorking() const;

        /// starts logging thread (if not started)
        void Start();

        void Close();

        void AddMessage(LogLevel logLevel, const std::string & msg);
        void AddMessage(LogLevel logLevel, const char * fmt, ...);

        void SetOption(LoggerOptions option, bool enabled);

        bool IsOptionSet(LoggerOptions option);

        void SetMinimalLogLevel(LogLevel logLevel);

        LogLevel GetMinimalLogLevel();

    private:

        /// function to open/reopen file if needed
        void OpenFileIfNeeded(const LogMsg * time);
        void ReopenFile();

        void WriteToStdOut(const std::string & str);
        void WriteToStdOut(const char * str, ...);

        void LogWriter();

        std::atomic<bool> closing_;

        std::array<std::atomic<bool>, OPTIONS_COUNT> options_;

        std::atomic<LogLevel> minLogLevel_;

        std::condition_variable canLog_;

        ///
        LoggerMsgList queued_;

        std::mutex dataMutex_;
        std::mutex logMutex_;
        std::mutex fileMutex_;
        std::mutex closeMutex_;

        std::shared_ptr<std::thread> thread_;
        std::string fileName_;
        std::string fileExtension_;

        std::string separator_;

        std::ofstream file_;

        std::time_t fileOpenTime_;
    };
}

#endif // SIOF_LOGGER
