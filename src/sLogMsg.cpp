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

#include "sLogMsg.h"

#include <iomanip>

SLogMsg::SLogMsg(const SLogMsg& p)
{
    time_ = p.GetTime();
    msg_ = p.GetMsg();
}

SLogMsg::SLogMsg(const std::string & msg)
{
    time_ = std::chrono::system_clock::now();
    msg_ = msg;
}

const std::string & SLogMsg::GetMsg() const
{
    return msg_;
}

std::time_t SLogMsg::GetCTime() const
{
    // commented couse could round to up value
    //return std::chrono::system_clock::to_time_t(time_);
    return std::chrono::duration_cast<std::chrono::seconds>(time_.time_since_epoch()).count();
}

const SLogTimePoint & SLogMsg::GetTime() const
{
    return time_;
}

SLogMsg & SLogMsg::operator = (const SLogMsg & p)
{
    time_ = p.GetTime();
    msg_ = p.GetMsg();

    return *this;
}

std::ostream & operator<< (std::ostream & out, const SLogMsg & msg)
{
    // seconds
    std::time_t tmpTime = msg.GetCTime();
    // miliseconds
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(msg.time_ - std::chrono::system_clock::from_time_t(tmpTime));

    out << std::put_time(std::localtime(&tmpTime), "%Y-%m-%d %H:%M:%S.");
    out << std::setw(3) << ms.count();
    out << std::setw(0) << " - " << msg.msg_ << "\n";//std::endl;

    return out;
}
