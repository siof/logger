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

#ifndef SIOF_LOGGER_MSG
#define SIOF_LOGGER_MSG

#include <chrono>
#include <ctime>
#include <string>

typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration> SLogTimePoint;

enum SLogLevel
{
    SLOG_LEVEL_NONE         = 0,
    SLOG_LEVEL_DEBUG        = 1,
    SLOG_LEVEL_DEBUG2       = 2,
    SLOG_LEVEL_WARNING      = 3,
    SLOG_LEVEL_ERROR        = 4,
    SLOG_LEVEL_FATAL        = 5,
    SLOG_LEVEL_EXCEPTION    = 6,

    SLOG_LEVEL_INFO         = 7,

    SLOG_LEVEL_COUNT
};

class SLogMsg
{
public:
    SLogMsg(){}
    SLogMsg(const SLogMsg& p);
    SLogMsg(SLogLevel logLevel, const std::string & msg);
    ~SLogMsg(){}

    const std::string & GetMsg() const;
    inline std::time_t GetCTime() const;
    const SLogTimePoint & GetTime() const;

    friend std::ostream & operator<< (std::ostream & out, const SLogMsg & msg);

    SLogMsg & operator = (const SLogMsg & p);

    static inline std::string GetLogLevelStr(SLogLevel logLevel);

private:

    SLogTimePoint time_;
    SLogLevel logLevel_;
    std::string msg_;
};

#endif // SIOF_LOGGER_MSG
