/*
 * Id: $Id: qpamat.cpp,v 1.47 2004/12/26 17:44:38 bwalle Exp $
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
#include <qapplication.h>
#include <qmainwindow.h>
#include <qaction.h>
#include <qkeysequence.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qmenubar.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qdockwindow.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qsettings.h>
#include <qscrollview.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qclipboard.h>
#include <qfont.h>
#include <qeventloop.h>
#include <qpaintdevicemetrics.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qsimplerichtext.h> 


#include "qpamat.h"
#include "settings.h"
#include "datareadwriter.h"
#include "timerstatusmessage.h"
#include "dialogs/passworddialog.h"
#include "dialogs/newpassworddialog.h"
#include "dialogs/configurationdialog.h"
#include "tree.h"
#include "rightpanel.h"

/*! 
    \class Qpamat
    
    \brief This is the main class of the QPaMaT application.
    
    This class represents the main window, so it initializes actions, menubars and toolbars,
    creates the Tree and the RightPanel, loads and saves setting. It handles some actions
    like printing directly by calling the right functions from the other classes and delegates
    some actions to other classes.
    
    It makes no sense to have more than one instances of this class. There's a global pointer
    \c qpamat which holds the address of the qpamat. This pointer is initialized \em after
    the full initialization of Qpamat, i.e. don't use it in constructors of Tree and RightPanel
    because theses are generated by Qpamat and therefore \c qpamat is not initialized (it's just
    \c 0).
    
    \ingroup gui
    \author Bernhard Walle
    \version $Revision: 1.47 $
    \date $Date: 2004/12/26 17:44:38 $
 */

/*! 
    \fn Qpamat::insertPassword(const QString&)
    
    This signal is emitted if a password was generated for inserting.
*/

/*!
    \fn Qpamat::settingsChanged()
    
    This signals is emitted if the settings have changed.
*/

// -------------------------------------------------------------------------------------------------

/*! 
    Creates a new instance of the application.
 */
Qpamat::Qpamat()
    : QMainWindow(0, "qpamat main window"), m_tree(0), m_treeContextMenu(0), m_message(0), m_rightPanel(0), m_searchCombo(0), 
      m_searchToolbar(0), m_randomPassword(0)
{
    // Title and Icon
    setIcon(QPixmap::fromMimeSource("stock_dialog_authentication_48.png"));
    setCaption("QPaMaT");
    
    setUsesBigPixmaps(true);
    QIconSet::setIconSize(QIconSet::Small, QSize(16, 16));
    QIconSet::setIconSize(QIconSet::Large, QSize(22, 22));
    
    // Random password, we need this for the tree
    m_randomPassword = new RandomPassword(this, "Random Password");
    
    // Tree on the left
    QDockWindow* dock = new QDockWindow(this);
    dock->setVerticallyStretchable(true);
    dock->setResizeEnabled(true);
    dock->setCloseMode(QDockWindow::Always);
    dock->setCaption(tr("Sites"));
    dock->setFixedExtentWidth(int(qApp->desktop()->width() * 0.15));
    addDockWindow(dock, Qt::DockLeft);
    
    m_tree = new Tree(dock);
    dock->setWidget(m_tree);
    
    // main widget in the center
    m_rightPanel = new RightPanel(this);
    setCentralWidget(m_rightPanel);
    
    // Initialization of menu
    initActions();
    initMenubar();
    initToolbar();
    
    // display statusbar
    statusBar();
    m_message = new TimerStatusmessage(statusBar());
    setLogin(false);
    
    // restore history
    QStringList list = QStringList::split (" | ", set().readEntry("Main Window/SearchHistory") );
    m_searchCombo->insertStringList(list);
    m_searchCombo->clearEdit();
    
    // restore the layout
    QString layout = set().readEntry("Main Window/layout");
    QTextStream layoutStream(&layout, IO_ReadOnly);
    layoutStream >> *this;
    if (set().readBoolEntry("Main Window/maximized"))
    {
        showMaximized();
    }
    else
    {
        resize(
            set().readNumEntry("Main Window/width", int(qApp->desktop()->width() * 0.6) ),
            set().readNumEntry("Main Window/height", int(qApp->desktop()->height() / 2.0) )
        );
    }
    QString rightpanel = set().readEntry("Main Window/rightpanelLayout");
    if (rightpanel)
    {
        QTextStream rightpanelStream(&rightpanel, IO_ReadOnly);
        rightpanelStream >> *m_rightPanel;
    }
    
    connectSignalsAndSlots();
    
    if (set().readBoolEntry("General/AutoLogin"))
    {
        if (QFile::exists(set().readEntry("General/Datafile")) )
        {
            QTimer::singleShot( 0, this, SLOT(login()) );
        }
        else
        {
            QTimer::singleShot( 0, this, SLOT(newFile()) );
        }
    }
}


/*!
    Deletes the application.
 */
Qpamat::~Qpamat()
{
    delete m_message;
}


/*!
   Returns the settings object.
   \return a reference to the object
 */
Settings& Qpamat::set()
{
    return m_settings;
}


/*!
    Prints a message in the statusbar.
    \param message the message string
    \param warning \c true if it's a warning, \c false if it's just a information
           Currently the time is determined by this argument.
 */
void Qpamat::message(const QString& message, bool)
{
    m_message->message(message, /*warning ? 3000 : */ 1500);
}


/*!
    Initializes the toolbar.
 */
void Qpamat::initToolbar()
{
    // ----- Application ---------------------------------------------------------------------------
    QToolBar* applicationToolbar = new QToolBar(this);
    applicationToolbar->setLabel(tr("Application"));
    
    m_actions.newAction->addTo(applicationToolbar);
    m_actions.saveAction->addTo(applicationToolbar);
    m_actions.printAction->addTo(applicationToolbar);
    m_actions.settingsAction->addTo(applicationToolbar);
    
    // ----- Search --------------------------------------------------------------------------------
    m_searchToolbar = new QToolBar(this);
    m_searchToolbar->setLabel(tr("Search"));
    new QLabel(tr("Search:")+" ", m_searchToolbar);
    
    m_searchCombo = new QComboBox(true, m_searchToolbar);
    m_searchCombo->setMinimumWidth(120);
    m_searchCombo->setDuplicatesEnabled(false);
    m_searchCombo->setFocusPolicy(QWidget::ClickFocus);
    m_searchCombo->setInsertionPolicy(QComboBox::AtTop);
    m_searchCombo->setAutoCompletion(true);
    
    m_toolButtons.search = new QToolButton(
        QIconSet(QPixmap::fromMimeSource("stock_search_16.png"),
        QPixmap::fromMimeSource("stock_search_24.png")), 0, 0, 0, 0,
        m_searchToolbar, "Search Toolbutton");
    
    // ---- Edit toolbar ---------------------------------------------------------------------------
    QToolBar* editToolbar = new QToolBar(this);
    editToolbar->setLabel(tr("Edit"));
    m_actions.addItemAction->addTo(editToolbar);
    m_actions.removeItemAction->addTo(editToolbar);
}


/*!
    Initializes the menubar.
 */
void Qpamat::initMenubar()
{
     // ----- File ---------------------------------------------------------------------------------
     QPopupMenu* fileMenu = new QPopupMenu(this);
     menuBar()->insertItem(tr("&File"), fileMenu);
     
     m_actions.newAction->addTo(fileMenu);
     m_actions.loginAction->addTo(fileMenu);
     m_actions.logoutAction->addTo(fileMenu);
     m_actions.saveAction->addTo(fileMenu);
     m_actions.exportAction->addTo(fileMenu);
     m_actions.printAction->addTo(fileMenu);
     fileMenu->insertSeparator();
     m_actions.quitAction->addTo(fileMenu);
     
     // ----- View ---------------------------------------------------------------------------------
     menuBar()->insertItem(tr("&View"), createDockWindowMenu(QMainWindow::NoToolBars));
     
     // ----- Options ------------------------------------------------------------------------------
     QPopupMenu* optionsMenu = new QPopupMenu(this);
     menuBar()->insertItem(tr("&Options"), optionsMenu);
     
     m_actions.changePasswordAction->addTo(optionsMenu);
     m_actions.settingsAction->addTo(optionsMenu);
     
     // ----- Extras -------------------------------------------------------------------------------
     QPopupMenu* extrasMenu = new QPopupMenu(this);
     menuBar()->insertItem(tr("&Extras"), extrasMenu);
     
     m_actions.randomPasswordAction->addTo(extrasMenu);
     m_actions.passwordStrengthAction->addTo(extrasMenu);
     m_actions.clearClipboardAction->addTo(extrasMenu);
     
     // ----- Help ---------------------------------------------------------------------------------
     menuBar()->insertSeparator();
     QPopupMenu* helpMenu = new QPopupMenu(this);
     menuBar()->insertItem(tr("&Help"), helpMenu);
     
     m_actions.helpAction->addTo(helpMenu);
     m_actions.whatsThisAction->addTo(helpMenu);
     helpMenu->insertSeparator();
     m_actions.aboutQtAction->addTo(helpMenu);
     m_actions.aboutAction->addTo(helpMenu);
}


/*!
    Saves the data if the application is closes.
 */
void Qpamat::closeEvent(QCloseEvent* e)
{
    if (!logout())
    {
        return;
    }
    
    // write the history
    QStringList list;
    int max = QMIN(m_searchCombo->count(), 10);
    for (int i = 0; i < max; ++i)
    {
        list.append(m_searchCombo->text(i));
    }
    set().writeEntry("Main Window/SearchHistory", list.join(" | "));
    
    // write window layout
    QString layout;
    QTextStream layoutStream(&layout, IO_WriteOnly);
    layoutStream << *this;
    QString rightpanelLayout;
    QTextStream rightpanelStream(&rightpanelLayout, IO_WriteOnly);
    rightpanelStream << *m_rightPanel;
    set().writeEntry("Main Window/layout", layout);
    set().writeEntry("Main Window/width", size().width());
    set().writeEntry("Main Window/height", size().height());
    set().writeEntry("Main Window/maximized", isMaximized());
    set().writeEntry("Main Window/rightpanelLayout", rightpanelLayout);
    
    e->accept();
}


/*!
    Sets the document status to modified or not modified.
    \param modified \c true if the document is modified, \c false if it is not modified
 */
void Qpamat::setModified(bool modified)
{
    m_modified = modified;
    m_actions.saveAction->setEnabled(modified);
}


/*!
    Does the login.
 */
void Qpamat::login()
{
    std::auto_ptr<PasswordDialog> dlg(new PasswordDialog(this));
    QDomDocument doc;
    bool ok = false;
    
    while (!ok)
    {
        if (dlg->exec() == QDialog::Accepted)
        {
            m_password = dlg->getPassword();
        }
        else
        {
            return;
        }
        
        DataReadWriter reader(this);
        while (!ok)
        {
            try
            {
                doc = reader.readXML(m_password);
                ok = true;
            }
            catch (const ReadWriteException& e)
            {
                // type of the message
                QMessageBox::Icon icon = e.getSeverity() == ReadWriteException::WARNING
                    ? QMessageBox::Warning
                    : QMessageBox::Critical;
                // category: different behaviour
                ReadWriteException::Category cat = e.getCategory();
                
                if (cat == ReadWriteException::CAbort)
                {
                    return;
                }
                
                bool retry = e.retryMakesSense();
                
                QMessageBox *mb = new QMessageBox( "QPaMaT", e.getMessage(), icon, 
                    (retry ? QMessageBox::Retry : QMessageBox::Ok) | QMessageBox::Default, 
                    (retry ? QMessageBox::Abort : QMessageBox::NoButton), 
                    QMessageBox::NoButton, this, "qt_msgbox_information", true, WDestructiveClose);
                if (mb->exec() != QMessageBox::Retry)
                {
                    return;
                }

                // the user wants to continue, now we discriminate if we just retry
                // or show the password dialog again
                if (cat == ReadWriteException::CWrongPassword)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
    }

    m_tree->readFromXML(doc.documentElement().namedItem("passwords").toElement());
    
    setLogin(true);
}


/*!
    Helping function for setting the login status
    \param loggedIn \c true if the user has logged in, \c false if he has logged out
 */
void Qpamat::setLogin(bool loggedIn)
{
    m_loggedIn = loggedIn;
    
    m_actions.loginAction->setEnabled(!loggedIn);
    m_actions.logoutAction->setEnabled(loggedIn);
    m_actions.saveAction->setEnabled(loggedIn);
    m_actions.changePasswordAction->setEnabled(loggedIn);
    m_actions.printAction->setEnabled(loggedIn);
    m_searchCombo->setEnabled(loggedIn);
    m_toolButtons.search->setEnabled(loggedIn);
    m_actions.addItemAction->setEnabled(loggedIn);
    m_actions.removeItemAction->setEnabled(loggedIn);
    m_actions.passwordStrengthAction->setEnabled(loggedIn);
    m_actions.exportAction->setEnabled(loggedIn);
    
    if (!loggedIn)
    {
        m_actions.passwordStrengthAction->setOn(false);
        m_tree->clear();
        m_rightPanel->clear();
        this->setFocus();
    }
    m_tree->setEnabled(loggedIn);
    setModified(false);
}


/*!
    Performs the save operation.
 */
void Qpamat::save()
{
    if (m_loggedIn && exportOrSave())
    {
        setModified(false);
        message("Wrote data successfully.");
    }
}


/*!
    Exports or saves the data. The file name is determined by the global property
    General/Datafile and the property Smartcard/UseCard is considered for smartcard
    writing.
    \return \c true if the action was successful, \c false otherwise
*/
bool Qpamat::exportOrSave()
{
    DataReadWriter writer(this);
    QDomDocument doc = writer.createSkeletonDocument();
    m_tree->appendXML(doc);
    bool success = false;
    while (!success)
    {
        try
        {
            writer.writeXML(doc, m_password);
            success = true;
        }
        catch (const ReadWriteException& e)
        {   
            QMessageBox::Icon icon = e.getSeverity() == ReadWriteException::WARNING
                ? QMessageBox::Warning
                : QMessageBox::Critical;
            bool retry = e.retryMakesSense();
            QMessageBox *mb = new QMessageBox( "QPaMaT", e.getMessage(), icon, 
                (retry ? QMessageBox::Retry : QMessageBox::Ok) | QMessageBox::Default, 
                (retry ? QMessageBox::Abort : QMessageBox::NoButton),     
                QMessageBox::NoButton, this, "qt_msgbox_information", true, WDestructiveClose);

            // ask the user if the exception type is not "abort"
            if (e.getCategory() == ReadWriteException::CAbort 
                    || mb->exec() != QMessageBox::Retry)
            {
                break;
            }
        }
    }
    return success;
}


/*!
    Exports the data.
*/
void Qpamat::exportData()
{
    QString oldFilename = set().readEntry("General/Datafile");
    QString file = QFileDialog::getSaveFileName(
        QDir::homeDirPath(), tr("QPaMaT XML files (*.xml)"), this, "file dialog", tr("QPaMaT") );
    bool oldCard = set().readBoolEntry("Smartcard/UseCard");
    if (!file)
    {
        return;
    }
    
    set().writeEntry("General/Datafile", file);
    set().writeEntry("Smartcard/UseCard", false);
    if (m_loggedIn && exportOrSave())
    {
        message("Wrote data successfully.");
    }
    set().writeEntry("General/Datafile", oldFilename);
    set().writeEntry("Smartcard/UseCard", oldCard);
}


/*!
    Performs the logout operation.

    \return \c true if logout should be continued, \c false otherwise
 */
bool Qpamat::logout()
{
    // save the data
    if (m_modified)
    {
        int ret = QMessageBox::question(this, "QPaMaT", tr("There is modified data that was not saved."
            "\nDo you want to save it now?"), QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No, QMessageBox::Cancel) ;

        switch (ret)
        {
            case QMessageBox::Yes:
                save();
                break;

            case QMessageBox::Cancel:
                return false;

            default:
                break;
        }
    }
    
    setLogin(false);

    return true;
}


/*!
    Creates a new file.
 */
void Qpamat::newFile()
{
    std::auto_ptr<NewPasswordDialog> dialog(new NewPasswordDialog(this));
    
    if (dialog->exec() == QDialog::Accepted)
    {
        if (m_loggedIn)
        {
            logout();
        }
        
        m_password = dialog->getPassword();
        setLogin(true);
        setModified();
    }
}


/*!
    Changes the password of the current session.
 */
void Qpamat::changePassword()
{
    std::auto_ptr<NewPasswordDialog> dlg(new NewPasswordDialog(this, m_password));
    if (dlg->exec() == QDialog::Accepted)
    {
        m_password = dlg->getPassword();
        setModified();
    }
}


/*!
    Displays the configuration dialog.
 */
void Qpamat::configure()
{
    std::auto_ptr<ConfigurationDialog> dlg(new ConfigurationDialog(this));
    if (dlg->exec() == QDialog::Accepted)
    {
        emit settingsChanged();
    }
}


/*!
    Does the search.
 */
void Qpamat::search()
{
    const QString text = m_searchCombo->currentText();
    
    if (text.length() == 0)
    {
         QMessageBox::warning(this, QObject::tr("QPaMaT"),
               tr("Please enter a search criterion in the text field!"),
               QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton);
    }
    else
    {
        m_tree->searchFor(text);
    }
}


/*!
    Handles the passowrd strength toggle action.
    \param enabled \c true if the password strength should be displayed, \c false otherwise
 */
void Qpamat::passwordStrengthHandler(bool enabled)
{
    m_tree->setShowPasswordStrength(enabled);
    bool error = false;
    if (enabled)
    {
        m_tree->recomputePasswordStrength(&error);
        if (error)
        {
            m_actions.passwordStrengthAction->setOn(false);
        }
    }
    
    if (!error)
    {
        m_tree->updatePasswordStrengthView();
    }
}


/*!
    Prints the current document. Displayes the print dialog for that reason.
 */
void Qpamat::print()
{
    QPrinter printer( QPrinter::HighResolution );
    printer.setFullPage( TRUE );
    if ( printer.setup( this ) ) 
    {
        QPainter p;
        if ( !p.begin( &printer ) )
        {
            return;
        }
        
        QFont serifFont;
        QFont sansSerifFont;
        serifFont.fromString(set().readEntry("Presentation/NormalFont"));
        sansSerifFont.fromString(set().readEntry("Presentation/FooterFont"));
        
        p.setFont(sansSerifFont);
        
        qApp->setOverrideCursor( QCursor( Qt::WaitCursor ) );
        qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
        
        QPaintDeviceMetrics metrics(p.device());
        int dpiy = metrics.logicalDpiY();
#define CON_MM(x)( int( ( (x)/25.4)*dpiy ) )
        int margin = CON_MM(20);
        QRect body( margin, margin, metrics.width() - 2 * margin, 
            metrics.height() - 2 * margin - CON_MM(8) );
        QSimpleRichText richText(m_tree->toRichTextForPrint(), serifFont, QString::null, 0, 
                QMimeSourceFactory::defaultFactory(), body.height(), Qt::black, false );
        richText.setWidth( &p, body.width() );
        QRect view( body );
        QString programString = tr("QPaMaT - Password managing tool for Unix, Windows and MacOS X");
        QString dateString = QDate::currentDate().toString(Qt::ISODate) + " / " +
            QTime::currentTime().toString("hh:mm");
        
        for (int page = 1; ; ++page)
        {
            qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
            QString pageS = tr("page") + " " + QString::number(page);
            
            richText.draw( &p, body.left(), body.top(), view, colorGroup() );
            view.moveBy(0, body.height() );
            p.translate(0 , -body.height() );
            
            int x_pos = int(( view.left() + p.fontMetrics().width(programString)
                + view.right() - p.fontMetrics().width( pageS ) ) / 2.0
                - p.fontMetrics().width( dateString ) / 2.0 );
            int y_pos = view.bottom() + p.fontMetrics().ascent() + CON_MM(3);
            p.drawLine(view.left(), view.bottom()+CON_MM(3), view.right(), view.bottom()+CON_MM(3));
            p.drawText( view.left(), y_pos, programString);
            p.drawText( x_pos, y_pos, dateString );
            p.drawText( view.right() - p.fontMetrics().width(pageS), y_pos, pageS);
            if ( view.top() >= richText.height() )
            {
                break;
            }
            printer.newPage();
        }
#undef CONVERT_MM
        
        qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
        qApp->restoreOverrideCursor();
    }
}


/*!
    Clears the clipboard.
 */
void Qpamat::clearClipboard()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setText(QString::null, QClipboard::Clipboard);
    cb->setText(QString::null, QClipboard::Selection);
}


/*!
    Connects all signals with slots.
 */
void Qpamat::connectSignalsAndSlots()
{
    connect(m_tree, SIGNAL(selectionChanged(QListViewItem*)),
        m_rightPanel, SLOT(setItem(QListViewItem*)));
    connect(m_tree, SIGNAL(selectionCleared()), m_rightPanel, SLOT(clear()));
    
    
    // Actions
    connect(m_actions.newAction, SIGNAL(activated()), this, SLOT(newFile()));
    connect(m_actions.quitAction, SIGNAL(activated()), this, SLOT(close()));
    connect(m_actions.loginAction, SIGNAL(activated()), this, SLOT(login()));
    connect(m_actions.logoutAction, SIGNAL(activated()), this, SLOT(logout()));
    connect(m_actions.printAction, SIGNAL(activated()), this, SLOT(print()));
    connect(m_actions.saveAction, SIGNAL(activated()), this, SLOT(save()));
    connect(m_actions.exportAction, SIGNAL(activated()), this, SLOT(exportData()));
    
    connect(m_actions.changePasswordAction, SIGNAL(activated()), this, SLOT(changePassword()));
    connect(m_actions.settingsAction, SIGNAL(activated()), this, SLOT(configure()));
    
    connect(m_actions.helpAction, SIGNAL(activated()), &m_help, SLOT(showHelp()));
    connect(m_actions.whatsThisAction, SIGNAL(activated()), this, SLOT(whatsThis()));
    connect(m_actions.aboutAction, SIGNAL(activated()) , &m_help, SLOT(showAbout()));
    connect(m_actions.aboutQtAction, SIGNAL(activated()), qApp, SLOT(aboutQt()));
    
    connect(m_actions.randomPasswordAction, SIGNAL(activated()),
        m_randomPassword, SLOT(requestPassword()));
    connect(m_actions.clearClipboardAction, SIGNAL(activated()), SLOT(clearClipboard()));
    
    // password strength
    connect(m_actions.passwordStrengthAction, SIGNAL(toggled(bool)), SLOT(passwordStrengthHandler(bool)));
    connect(this, SIGNAL(settingsChanged()), m_tree, SLOT(recomputePasswordStrength()));
    connect(m_rightPanel, SIGNAL(passwordStrengthUpdated()), m_tree, SLOT(updatePasswordStrengthView()));
        
    // edit toolbar
    connect(m_actions.addItemAction, SIGNAL(activated()), m_tree, SLOT(insertAtCurrentPos()));
    connect(m_actions.addItemAction, SIGNAL(activated()), m_rightPanel, SLOT(insertAtCurrentPos()));
    connect(m_actions.removeItemAction, SIGNAL(activated()), m_tree, SLOT(deleteCurrent()));
    connect(m_actions.removeItemAction, SIGNAL(activated()), m_rightPanel, SLOT(deleteCurrent()));
    
    // search function
    connect(m_searchCombo, SIGNAL(activated(int)), this, SLOT(search()));
    connect(m_toolButtons.search, SIGNAL(clicked()), this, SLOT(search()));
    connect(m_actions.focusSearch, SIGNAL(activated()), m_searchCombo, SLOT(setFocus()));
    
    // modified
    connect(m_tree, SIGNAL(stateModified()), SLOT(setModified()));
    connect(m_rightPanel, SIGNAL(stateModified()), SLOT(setModified()));
    
    // random password
    connect(m_rightPanel, SIGNAL(passwordLineEditGotFocus(bool)), m_randomPassword,
        SLOT(setInsertEnabled(bool)));
    connect(m_randomPassword, SIGNAL(insertPassword(const QString&)), 
        SIGNAL(insertPassword(const QString&)));
}


/*!
    Initializes the actions.
 */
void Qpamat::initActions()
{
    // ----- File ----------------------------------------------------------------------------------
    m_actions.newAction = new QAction(QIconSet(QPixmap::fromMimeSource("stock_new_16.png"),
        QPixmap::fromMimeSource("stock_new_24.png")), tr("&New"), 
        QKeySequence(CTRL|Key_N), this);
    m_actions.quitAction = new QAction(QIconSet( QPixmap::fromMimeSource("stock_exit_16.png"),
        QPixmap::fromMimeSource("stock_exit_24.png") ), tr("E&xit"),
        QKeySequence(CTRL|Key_Q), this);
    m_actions.loginAction = new QAction(tr("&Login"), QKeySequence(CTRL|Key_L), this);
    m_actions.logoutAction = new QAction(tr("&Logout"), QKeySequence(CTRL|Key_L), this);
    m_actions.saveAction = new QAction(QIconSet(QPixmap::fromMimeSource("stock_save_16.png"),
        QPixmap::fromMimeSource("stock_save_24.png")), tr("&Save"), 
        QKeySequence(CTRL|Key_S), this);
    m_actions.exportAction = new QAction(tr("&Export..."), QKeySequence(), this);
    m_actions.printAction = new QAction(QIconSet(
        QPixmap::fromMimeSource("stock_print_16.png"), 
        QPixmap::fromMimeSource("stock_print_24.png")),
        tr("&Print..."), QKeySequence(CTRL|Key_P), this);
    
    // ----- Options -------------------------------------------------------------------------------
    m_actions.changePasswordAction = new QAction(tr("&Change Password..."), QKeySequence(), this);
    m_actions.settingsAction = new QAction(QIconSet(
        QPixmap::fromMimeSource("stock_preferences_16.png"),
        QPixmap::fromMimeSource("stock_preferences_24.png")), 
        tr("&Settings..."), QKeySequence(CTRL|Key_T), this);
        
    // ----- Extras --------------------------------------------------------------------------------
    m_actions.randomPasswordAction = new QAction(tr("&Random Password..."), 
        QKeySequence(CTRL|Key_R), this);
    m_actions.passwordStrengthAction = new QAction(tr("&Show password strength"),
        QKeySequence(CTRL|Key_W), this);
    m_actions.passwordStrengthAction->setToggleAction(true);
    m_actions.clearClipboardAction = new QAction(QIconSet(
        QPixmap::fromMimeSource("clear_clipboard_16.png")), 
        tr("&Clear clipboard"), QKeySequence(CTRL|Key_E), this);
    
    // ----- Help ----------------------------------------------------------------------------------
    m_actions.helpAction = new QAction(QIconSet(QPixmap::fromMimeSource("stock_help_16.png"),
        QPixmap::fromMimeSource("stock_help_24.png")), tr("&Help"), QKeySequence(Key_F1), this);
    m_actions.whatsThisAction = new QAction(QPixmap(QPixmap::fromMimeSource("whats_this.png")), 
        tr("&What's this"), QKeySequence(SHIFT|Key_F1), this);
    m_actions.aboutAction = new QAction(tr("&About..."), 0, this);
    m_actions.aboutQtAction = new QAction(QPixmap::fromMimeSource("qt_16.png"), 
        tr("About &Qt..."), QKeySequence(), this);
    
    // ------ Toolbar ------------------------------------------------------------------------------
    m_actions.addItemAction = new QAction(QIconSet(QPixmap::fromMimeSource("stock_add_16.png"),
        QPixmap::fromMimeSource("stock_add_24.png")),
        tr("Add item"), QKeySequence(Key_Insert), this, "Add item action");
    m_actions.removeItemAction = new QAction(QIconSet(QPixmap::fromMimeSource("stock_remove_16.png"),
        QPixmap::fromMimeSource("stock_remove_24.png")),
        tr("Remove item"), QKeySequence(), this, "Remove item action");
        
    // ------ Misc ---------------------------------------------------------------------------------
    m_actions.focusSearch = new QAction(tr("Focus &search"), QKeySequence(CTRL|Key_G), this);
}

