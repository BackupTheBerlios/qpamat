/*
 * Id: $Id: listboxdialog.h,v 1.1 2004/01/07 23:55:11 bwalle Exp $
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
#ifndef LISTBOXDIALOG_H
#define LISTBOXDIALOG_H

#include <set>

#include <qdialog.h>
#include <qwidget.h>
#include <qlistbox.h>
#include <qwidgetstack.h>

class ListBoxDialogPage : public QWidget
{
    friend class ListBoxDialog;
    
    public:
        ListBoxDialogPage(QWidget* parent, const char* name = 0) : QWidget(parent, name) { }
        
    protected:
        virtual void fillSettings() = 0;
        virtual void applySettings() = 0;
};

// -------------------------------------------------------------------------------------------------

class ListBoxDialog : public QDialog 
{
    Q_OBJECT
    
    public:
        ListBoxDialog(QWidget* parent, const char* name = 0);
        
    public:
        void addPage(ListBoxDialogPage* widget, const QPixmap& pixmap, const QString& text);
        
    protected slots:
        void accept();
        virtual void polish();
        void aboutToShowHandler(QWidget* widget);
        
    private:
        ListBoxDialog(const ListBoxDialog&);
        ListBoxDialog& operator=(const ListBoxDialog&);
        
    private:
        QListBox*                     m_listBox;
        QWidgetStack*                 m_widgetStack;
        std::set<ListBoxDialogPage*>  m_filledTabs;
};

#endif // LISTBOXDIALOG_H
