/*
 * Id: $Id: main.cpp,v 1.1 2003/10/11 19:51:15 bwalle Exp $
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
#include <iostream>
#include <stdexcept>
#ifdef DEBUG
    #include <mcheck.h>
#endif

#include <qapplication.h>
#include <qmessagebox.h>

#include "qpamat.h"

int main(int argc, char** argv)
{
#ifdef DEBUG
    mtrace();
#endif
    int returncode;
    
    QApplication app(argc, argv);
    try
    {
        Qpamat* qpamat = new Qpamat();
        app.setMainWidget(qpamat);
        qpamat->show();
        
        returncode = app.exec();
    }
    catch (const std::bad_alloc& e)
    {
        QMessageBox::warning(0, QObject::tr("QPaMaT"), 
            QObject::tr("No more memory available. The application\nwill be closed."),
            QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton); 
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(0, QObject::tr("QPaMaT"),
            QObject::tr("An unknown error occured:\n%1\nThe application will be closed.")
            .arg(e.what()),
            QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton);
    }
    
    return returncode;
}


