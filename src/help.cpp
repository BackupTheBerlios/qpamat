/*
 * Id: $Id: help.cpp,v 1.1 2003/10/11 19:51:00 bwalle Exp $
 * -------------------------------------------------------------------------------------------------
 * 
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
#include <qobject.h>
#include <qapplication.h>
#include <qmessagebox.h>

#include "globals.h"
#include "help.h"


// -------------------------------------------------------------------------------------------------
void Help::showAbout()
// -------------------------------------------------------------------------------------------------
{
    QMessageBox::about(qApp->mainWidget(), QPAMAT_USERVISIBLE,
        tr("<p><b>QPaMaT "VERSION"</b></p><p>Password managing tool for Unix, Windows and MacOS X. "
            "It uses the OpenSSL library for encryption and is distributed under the terms "
            "of the GNU General Public License.</p>"
            "<p>� Bernhard Walle (<tt>bernhard.walle@gmx.de</tt>)"));
}
