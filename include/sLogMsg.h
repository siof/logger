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

#include <ctime>
#include <string>

class SLogMsg
{
public:
    SLogMsg(){}
    SLogMsg(const SLogMsg& p);
    SLogMsg(const std::string & msg);
    ~SLogMsg(){}

    const std::string & GetMsg() const;
    const std::time_t & GetTime() const;

    friend std::ostream & operator<< (std::ostream & out, const SLogMsg & msg);

    SLogMsg & operator = (const SLogMsg & p);
private:
    std::time_t time_;
    std::string msg_;
};

#endif // SIOF_LOGGER_MSG
