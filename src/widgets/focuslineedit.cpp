/*
 * Id: $Id: focuslineedit.cpp,v 1.1 2003/12/17 21:56:43 bwalle Exp $
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
#include <qstring.h>
#include <qlineedit.h>
#include <qobject.h>

#include "focuslineedit.h"

// -------------------------------------------------------------------------------------------------
FocusLineEdit::FocusLineEdit(QWidget* parent, const char* name)
// -------------------------------------------------------------------------------------------------
            : QLineEdit(parent, name)
{}


// -------------------------------------------------------------------------------------------------
FocusLineEdit::FocusLineEdit(const QString& contents, QWidget* parent, const char* name)
// -------------------------------------------------------------------------------------------------
            : QLineEdit(contents, parent, name)
{}


// -------------------------------------------------------------------------------------------------
FocusLineEdit::FocusLineEdit(const QString& contents, const QString& inputMask, QWidget* parent, 
            const char* name)
// -------------------------------------------------------------------------------------------------
            : QLineEdit(contents, inputMask, parent, name)
{}


// -------------------------------------------------------------------------------------------------
void FocusLineEdit::focusInEvent(QFocusEvent* evt)
// -------------------------------------------------------------------------------------------------
{
    QLineEdit::focusInEvent(evt);
    emit gotFocus();
}
