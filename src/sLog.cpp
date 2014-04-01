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
    return thread_.joinable();
}

/// starts logging thread (if not started)
void SLog::Start()
{
}

void SLog::Close()
{
    if (!closeMutex_.try_lock())
        return;

    if (!thread_.joinable())
        return;

    thread_.join();
}

void SLog::AddMessage(const std::string & msg)
{
}

void SLog::AddMessage(const char * fmt, ...)
{
}

void SLog::OpenFileIfNeeded(const std::time_t * time)
{
    std::lock_guard<std::mutex> guard(fileMutex_);

    if (!file_.is_open())
        ReopenFile();
}

void SLog::ReopenFile()
{
    if (file_.is_open())
        file_.close();

    fileOpenTime_ = std::time(nullptr);

    file_.open(fileName_, std::ios_base::app);

    file_ << std::endl << separator_ << std::endl << std::endl;
}
