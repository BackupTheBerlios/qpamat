/*
 * Id: $Id: configurationdialog.h,v 1.12 2003/12/30 00:28:22 bwalle Exp $
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
#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <set>

#include <qdialog.h>
#include <qlistbox.h>
#include <qwidgetstack.h>

class ConfDlgTab;

class ConfigurationDialog : public QDialog
{
    Q_OBJECT
    
    public:
        ConfigurationDialog(QWidget* parent);
        
    private:
        ConfigurationDialog(const ConfigurationDialog&);
        ConfigurationDialog& operator=(const ConfigurationDialog&);
        
    protected slots:
        void accept();
        void aboutToShowHandler(QWidget*);
        
    private:
        QListBox*               m_listBox;
        QWidgetStack*           m_widgetStack;
        std::set<ConfDlgTab*>   m_filledTabs;
};

#endif // CONFIGURATIONDIALOG_H
