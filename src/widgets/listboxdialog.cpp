/*
 * Id: $Id: listboxdialog.cpp,v 1.2 2004/01/08 23:09:00 bwalle Exp $
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
#include <qwidget.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qcursor.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "global.h"
#include "listboxdialog.h"
#include "widgets/listboxlabeledpict.h"

/*!
    \class ListBoxDialogPage
    
    \brief Represents a page in a ListBoxDialog
    
    Each page of the ListBoxDialog must be a subclass of this class. The applySettings() and
    fillSettings() method gets called if necessary.
    
    \ingroup widgets
    \author $Author: bwalle $
    \version $Revision: 1.2 $
    \date $Date: 2004/01/08 23:09:00 $
*/

/*!
    \fn ListBoxDialogPage::ListBoxDialogPage(QWidget*, const char*)
    
    Creates a new instance of a ListBoxDialogPage.
    
    \param parent the parent widget
    \param name the name of the widget
*/

/*!
    \fn ListBoxDialogPage::applySettings()
    
    This method is called if the user pressed Ok on the dialog. It should store the settings.
*/

/*!
    \fn ListBoxDialogPage::fillSettings()
    
    This method is called if the page should be shown. It should fill the page with the current
    settings.
*/

/*!
    \class ListBoxDialog 
    
    \brief Dialog with several pages that are choosen in a listbox
    
    This dialog normally is used for configuration issues. It has a listbox with big icons
    and labels on the left and pages on the right, just like normal KDE configuration dialogs.
    
    Each page on the right must be a subclass of ListBoxDialogPage and must be added with
    the addPage() method in the constructor of the subclassing dialog. The fillSettings()
    method of the page is called before it is shown and the applySettings() method is called
    if the user pressed Ok and the page was shown previously, so it's never called if the
    fillSettings() was not called. This increases performance.
    
    \ingroup widgets
    \author $Author: bwalle $
    \version $Revision: 1.2 $
    \date $Date: 2004/01/08 23:09:00 $
    
*/

/*!
    Creates a new instance of a ListBoxDialog.
    \param parent the parent widget
    \param name the name of the widget
*/
ListBoxDialog::ListBoxDialog(QWidget* parent, const char* name)
    : QDialog(parent, name)
{
    QVBoxLayout* vboxLayout = new QVBoxLayout(this, 8, 2, "ConfDlg-Vbox");
    QHBox* mainHBox = new QHBox(this, "ConfDlg-MainHBox");
    QHBox* buttonHBox = new QHBox(this, "ConfDlg-ButtonHBox");
    
    m_listBox = new QListBox(mainHBox, "ConfDlg-Listbox");
    QFont f = m_listBox->font();
    f.setBold(true);
    m_listBox->setFont(f);
    m_listBox->setCursor(PointingHandCursor);
    m_widgetStack = new QWidgetStack(mainHBox, "ConfDlg-Widget");
    
    // buttons
    QWidget* filler = new QWidget(buttonHBox);
    buttonHBox->setSpacing(7);
    QPushButton* okButton = new QPushButton(tr("OK"), buttonHBox, "OkButton");
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), buttonHBox, "CancelButton");
    okButton->setDefault(true);
    buttonHBox->setStretchFactor(filler, 2);
    buttonHBox->setStretchFactor(okButton, 0);
    buttonHBox->setStretchFactor(cancelButton, 0);
    
    // horizontal line
    QLabel* horizontalLine = new QLabel(this, "ConfDlg-Hline");
    horizontalLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    // layout
    mainHBox->setStretchFactor(m_listBox, 0);
    mainHBox->setStretchFactor(m_widgetStack, 2);
    mainHBox->setSpacing(12);
    vboxLayout->addWidget(mainHBox);
    vboxLayout->addWidget(horizontalLine);
    vboxLayout->addWidget(buttonHBox);
    vboxLayout->setStretchFactor(mainHBox, 2);
    vboxLayout->setStretchFactor(buttonHBox, 0);
    
    // signals & slots
    connect(okButton, SIGNAL(clicked()), SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject())); 
    connect(m_listBox, SIGNAL(highlighted(int)), m_widgetStack, SLOT(raiseWidget(int)));
    connect(m_widgetStack, SIGNAL(aboutToShow(QWidget*)), SLOT(aboutToShowHandler(QWidget*)));
}


/*!
    Adds a page at the bottom of the list.
    \param widget the page to add
    \param pixmap the pixmap (must exist)
    \param text the text that is displayed under the pixmap (must exist, one line only)
*/
void ListBoxDialog::addPage(ListBoxDialogPage* widget, const QPixmap& pixmap, const QString& text)
{
    m_widgetStack->addWidget(widget, m_listBox->count());
    new ListBoxLabeledPict(m_listBox, pixmap, text);
}


/*!
    Polishes the dialog, i.e. shows the first page.
*/
void ListBoxDialog::polish()
{
    QDialog::polish();
    QWidget* w = m_widgetStack->widget(0);
    if (w)
    {
        aboutToShowHandler(w);
        m_listBox->setSelected(0, true);
    } 
}


/*!
    Handles aboutToShow() signals from the widget stack. The widget gets filled, i.e. the 
    fillSettings() method of the corresponding page is called.
    \param widget the widget which gets shown
*/
void ListBoxDialog::aboutToShowHandler(QWidget* widget)
{
    if (ListBoxDialogPage* t = static_cast<ListBoxDialogPage*>(widget))
    {
        if ( m_filledTabs.find(t) == m_filledTabs.end() )
        {
            t->fillSettings();
            m_filledTabs.insert(t);
        }
    }
}


/*!
    Handles the accept() signal of the dialog. Calls the applySettings() method of each page
    that has been shown previously.
*/
void ListBoxDialog::accept()
{
    typedef std::set<ListBoxDialogPage*>::iterator ListBoxDlgPgSetIt;
    for (ListBoxDlgPgSetIt it = m_filledTabs.begin(); it != m_filledTabs.end(); ++it)
    {
        (*it)->applySettings();
    }
    QDialog::accept();
}
