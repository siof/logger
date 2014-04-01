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
    time_ = std::time(nullptr);
    msg_ = msg;
}

const std::string & SLogMsg::GetMsg() const
{
    return msg_;
}

const std::time_t & SLogMsg::GetTime() const
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
    out << std::put_time(std::localtime(&msg.time_), "") << " - " << msg.msg_ << std::endl;

    return out;
}
