/*
 * Id: $Id: showpassworddialog.cpp,v 1.4 2004/01/08 23:40:23 bwalle Exp $
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
#include <qdialog.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qpushbutton.h>

#include "qpamat.h"
#include "showpassworddialog.h"

/*!
    \class ShowPasswordDialog
    
    \brief Dialog which displayes a password.
    
    This dialog displays a password. It can display the password as cleartext and with stars.
    The user has the ability to copy it using a special button. He can also insert the password
    at the current position. For this, the dialog can show a Insert button and the dialog 
    emits the insertPassword() signal.
    
    It depends on the global property <tt>Presentation/HideRandomPass</tt> and on the constructor
    parameter if the dialog shows the insert button or not. 
    
    \ingroup dialogs
    \author Bernhard Walle
    \version $Revision: 1.4 $
    \date $Date: 2004/01/08 23:40:23 $
*/

/*!
    \enum ShowPasswordDialog::DialogType
    
    Specifies the type of the dialog. Use \c TRandomPasswordDlg if the dialog should show
    a random password, \c TRandomPasswordDlg if it should show a random password with an insert
    button or \c TNormalPasswordDlg if it should show a normal password. The message text changes
    between random password and normal password.
*/

/*!
    \fn ShowPasswordDialog::insertPassword(const QString&)
    
    Signal that is emitted if the user wants to insert a password.
    \param password the password the user wants to insert 
*/

/*!
    Creates a new instance of the password dialog.
    \param parent the parent widget
    \param type the type of the dialog
    \param name the name of the object
*/
ShowPasswordDialog::ShowPasswordDialog(QWidget* parent, DialogType type, const char* name)
    : QDialog(parent, name)
{
    setCaption("QPaMaT");
    
    // create elements
    QString labelText = type == TNormalPasswordDlg 
        ? tr("The password is:") 
        : tr("The random password is:");
    
    QLabel* label = new QLabel(labelText, this);
    
    m_passwordEdit = new CopyLabel(qpamat->set().readBoolEntry("Presentation/HideRandomPass"), 
        this);
    m_passwordEdit->setMinimumWidth(250);
    m_passwordEdit->setFocusPolicy(NoFocus);
    
    QPushButton* closeButton = new QPushButton(tr("&Close"), this);
    closeButton->setDefault(true);
    
    QPushButton* insertButton = 0;
    
    if (type == TRandomPasswordDlgInsert)
    {
        insertButton = new QPushButton(tr("&Insert"), this);
    }
    
    // create layouts
    QVBoxLayout* layout = new QVBoxLayout(this, 6, 6);
    QHBoxLayout* buttonLayout = new QHBoxLayout(0, 0, 6);
    
    // layout elements
    buttonLayout->addStretch(10);
    if (insertButton)
    {
        buttonLayout->addWidget(insertButton);
    }
    buttonLayout->addWidget(closeButton);
    
    layout->addWidget(label);
    layout->addWidget(m_passwordEdit);
    layout->addStretch(10);
    layout->addSpacing(7);
    layout->addLayout(buttonLayout);
    
    
    // communication
    connect(closeButton, SIGNAL(clicked()), SLOT(reject()));
    if (insertButton)
    {
        connect(insertButton, SIGNAL(clicked()), SLOT(insertButtonHandler()));
    }
}


/*!
    Returns the password which the user entered.
    \return the password
*/
QString ShowPasswordDialog::getPassword() const
{
    return m_passwordEdit->getContent();
}


/*!
    Sets the displayed password.
    \param newPassword the new password
*/
void ShowPasswordDialog::setPassword(const QString& newPassword)
{
    m_passwordEdit->setContent(newPassword);
}


/*!
    Slot that is called if the user pressed the Insert button.
*/
void ShowPasswordDialog::insertButtonHandler()
{
    emit insertPassword(m_passwordEdit->getContent());
}

