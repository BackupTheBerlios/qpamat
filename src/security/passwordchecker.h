/*
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; You may only use
 * version 2 of the License, you have no option to use any other version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * -------------------------------------------------------------------------------------------------
 */
#ifndef PASSWORDCHECKER_H
#define PASSWORDCHECKER_H

#include <stdexcept>

#include <QString>

#include "global.h"

class PasswordCheckException : public std::runtime_error
{
    public:
        PasswordCheckException(const std::string& error) : std::runtime_error(error) { }
};

class PasswordChecker
{
    public:
        static const unsigned int CRACKS_PER_SECOND;

    public:
        virtual ~PasswordChecker() { };

        virtual double passwordQuality(const QString& password) throw (PasswordCheckException) = 0;
};

#endif // PASSWORDCHECKER_H

// vim: set sw=4 ts=4 et: :tabSize=4:indentSize=4:maxLineLen=100:
