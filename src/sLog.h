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

#include <atomic>
#include <fstream>
#include <list>
#include <mutex>
#include <string>
#include <thread>

class SLogMsg;

#define SLOG_SEP_DEFAULT    "##########################################################################\n"\
                            "##########################################################################"

class SLog
{
public:
    /// only creates logger
    SLog() : separator_(SLOG_SEP_DEFAULT) {}
    /// dtor
    ~SLog();

    /// Here you can set/change filename to save files
    /// function automatically closes old file and reopen new
    void SetFileName(std::string & fileName);
    ///
    void SetSeparator(std::string & separator);
    /// Returns actual file name
    const std::string & GetFileName() const;

    /// returns if logging thread is working
    bool IsWorking() const;

    /// starts logging thread (if not started)
    void Start();

    void Close();

    void AddMessage(const std::string & msg);
    void AddMessage(const char * fmt, ...);

private:

    /// function to open/reopen file if needed
    void OpenFileIfNeeded(const std::time_t * time);
    void ReopenFile();

    std::atomic<bool> closing_;

    ///
    std::list<SLogMsg> queued_;

    std::mutex dataMutex_;
    std::mutex fileMutex_;
    std::mutex closeMutex_;

    std::thread thread_;
    std::string fileName_;

    std::string separator_;

    std::ofstream file_;

    std::time_t fileOpenTime_;
};

#endif // SIOF_LOGGER
