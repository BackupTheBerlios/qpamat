/*
 * Id: $Id$
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
#ifndef QPAMATADAPTOR_H
#define QPAMATADAPTOR_H

#include <QObject>
#ifdef Q_WS_X11

#include <QtDBus>
#include <QDBusConnection>
#include <QDBusAbstractAdaptor>

class Qpamat;

class QpamatAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "de.berlios.qpamat.Qpamat")

    public:
        QpamatAdaptor(Qpamat *qpamat);

    private:
        virtual ~QpamatAdaptor() {}

    public slots:
        Q_NOREPLY void showHideApplication();

    private:
    	Qpamat *m_qpamat;
};


#endif /* Q_WS_X11 */
#endif /* QPAMATADAPTOR_H */

// vim: set sw=4 ts=4 et:
