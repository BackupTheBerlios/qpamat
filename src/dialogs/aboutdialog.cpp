/*
 * Id: $Id: aboutdialog.cpp,v 1.3 2004/07/23 13:37:05 bwalle Exp $
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
#include "aboutdialog.h"

#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qfile.h>
#include <qtextedit.h>
#include <qapplication.h>
#include <qdir.h>
#include <qpushbutton.h>

/*!
    \class AboutDialog
    
    \brief The "about" dialog of QPaMaT.
    
    \ingroup gui
    \author Bernhard Walle
    \version $Revision: 1.3 $
    \date $Date: 2004/07/23 13:37:05 $
*/

/*!
    Creates a new instance of a AboutDialog object.
    
    \param parent the parent widget
    \param name the name of the widget
*/
AboutDialog::AboutDialog(QWidget* parent, const char* name = 0)
    : QDialog(parent, name)
{
    m_mainLayout = new QVBoxLayout(this, 6, 6);
    setCaption(tr("About QPaMaT"));
    
    // the top of the dialog
    QHBox* titleBox = new QHBox(this);
    QLabel* titleIcon = new QLabel(titleBox);
    titleIcon->setPixmap(QPixmap::fromMimeSource("stock_dialog_authentication_48.png"));
    QLabel* titleText = new QLabel(tr("<p><b>QPaMaT "VERSION_STRING"</b></p>"), titleBox);
    titleBox->setStretchFactor(titleIcon, 0);
    titleBox->setStretchFactor(titleText, 5);
    titleBox->setSpacing(5);
    
    // the tab
    m_tabWidget = new QTabWidget(this);
    setupAboutTab();
    setupLicenseTab();
    
    // the ok Button
    QPushButton* okButton = new QPushButton(tr("&Close"), this, "Close button");
    okButton->setDefault(true);
    QWidget* filler = new QWidget(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout(0, 0, 6);
    buttonLayout->addWidget(filler);
    buttonLayout->addWidget(okButton);
    buttonLayout->setStretchFactor(filler, 1);
    buttonLayout->setStretchFactor(okButton, 0);
    
    // main layout
    m_mainLayout->addWidget(titleBox);
    m_mainLayout->addWidget(m_tabWidget);
    m_mainLayout->addStretch(5);
    m_mainLayout->addLayout(buttonLayout);
    
    connect(okButton, SIGNAL(clicked()), SLOT(accept()));
    
}

/*!
    Sets up the about tab.
*/
void AboutDialog::setupAboutTab()
{
    QHBox* aboutTab = new QHBox(this);
    aboutTab->setMargin(15);
    
    (void)new QLabel(
        tr("<p><nobr>This is a password managing tool for Unix, Windows and "
            "MacOS X</nobr> written in C++ using the Qt library.</p>"
            "<p><b>Thanks to:</b>"
            "<ul><li>Trolltech for the Qt library</li>"
            "<li>OpenSSL team for the OpenSSL library</li>"
            "<li>Gtk+ artists for the nice stock icons</li>"
            "</ul></p>"
            "<p><b>Homepage:</b> <tt>http://qpamat.berlios.de</tt></p>"), aboutTab);
            
    m_tabWidget->addTab(aboutTab, tr("&About"));
    
}

/*!
    Sets up the license tab. Tries to search the \c LICENSE file.
*/
void AboutDialog::setupLicenseTab()
{
    QVBox* licenseTab = new QVBox(this);
    
    QTextEdit* textEdit = new QTextEdit(licenseTab);
    textEdit->setReadOnly(true);
    textEdit->setWordWrap(QTextEdit::FixedColumnWidth);
    textEdit->setWrapColumnOrWidth(100);
    
    QString fileName = qApp->applicationDirPath() + "/../share/qpamat/COPYING";
    if (QFile::exists(fileName))
    {
        QFile file(fileName);
        if (file.open( IO_ReadOnly )) 
        {
            QTextStream stream(&file);
            textEdit->setText("<pre>" + stream.read() + "</pre>");
        }
    }
    
    m_tabWidget->addTab(licenseTab, tr("&License"));
}

