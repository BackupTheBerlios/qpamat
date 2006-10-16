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
#include <QApplication>
#include <QTextDocument>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QMenu>
#include <QTextStream>
#include <QToolBar>
#include <QMenuBar>
#include <QIcon>
#include <QPixmap>
#include <QAbstractTextDocumentLayout>
#include <QStatusBar>
#include <QMessageBox>
#include <QLabel>
#include <QSettings>
#include <QDockWidget>
#include <QPrinter>
#include <QPainter>
#include <QClipboard>
#include <QFont>
#include <QEventLoop>
#include <QCursor>
#include <QFileDialog>
#include <QCloseEvent>
#include <QDesktopWidget>


#include "qpamat.h"

#include "settings.h"
#include "datareadwriter.h"
#include "timerstatusmessage.h"
#include "dialogs/passworddialog.h"
#include "dialogs/newpassworddialog.h"
#include "dialogs/configurationdialog.h"
#include "util/docktimeoutapplication.h"
#include "tree.h"
#include "rightpanel.h"

#ifdef Q_WS_WIN
#  define TRAY_ICON_FILE_NAME ":/images/qpamat_16.png"
#else
#  define TRAY_ICON_FILE_NAME ":/images/qpamat_34.png"
#endif

#define CON_MM(x)( int( ( (x)/25.4)*dpiy ) )

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
    \version $Revision: 1.52 $
    \date $Date$
 */

/*! 
    \fn Qpamat::insertPassword(const QString&)
    
    This signal is emitted if a password was generated for inserting.
*/

/*!
    \fn Qpamat::settingsChanged()
    
    This signals is emitted if the settings have changed.
*/


/*!
    \fn Qpamat::quit()
    
    If the application should quit.
*/

// -------------------------------------------------------------------------------------------------

/*! 
    Creates a new instance of the application.
 */
Qpamat::Qpamat()
    : QMainWindow(0, "qpamat main window"), m_tree(0), m_treeContextMenu(0), m_message(0),
      m_rightPanel(0), m_searchCombo(0), m_randomPassword(0), m_trayIcon(0), 
      m_lastGeometry(0, 0, 0, 0)
{
    // Title and Icon
    setIcon(QPixmap(":/images/qpamat_48.png"));
    setCaption("QPaMaT");
    
    setIconSize(QSize(24, 24));
    
    // Random password, we need this for the tree
    m_randomPassword = new RandomPassword(this, "Random Password");
    
    // Tree on the left
    QDockWidget* dock = new QDockWidget(tr("Sites"), this);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock->setObjectName("Sites");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setMinimumWidth(int(qApp->desktop()->width() * 0.15));
    
    m_tree = new Tree(dock);
    dock->setWidget(m_tree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    
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
    restoreState(set().readByteArrayEntry("Main Window/layout")); 
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
    if (!rightpanel.isNull())
    {
        QTextStream rightpanelStream(&rightpanel, QIODevice::ReadOnly);
        rightpanelStream >> *m_rightPanel;
    }
    
    // tray icon
    if (set().readBoolEntry("Presentation/SystemTrayIcon"))
    {
        QMenu* trayPopup = new QMenu(this);
        trayPopup->addAction(m_actions.showHideAction);
        trayPopup->addAction(m_actions.quitActionNoKeyboardShortcut);

        m_trayIcon = new QSystemTrayIcon(QPixmap(TRAY_ICON_FILE_NAME), this);
        m_trayIcon->setToolTip(tr("QPaMaT"));
        m_trayIcon->setContextMenu(trayPopup);
        m_trayIcon->show();

        // hack to prevent icontray events to interfere with the timeout mechanism
        dynamic_cast<DockTimeoutApplication*>(qApp)->addReceiverToIgnore(m_trayIcon);

        connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                SLOT(handleTrayiconClick(QSystemTrayIcon::ActivationReason)));
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
    QToolBar* applicationToolbar = new QToolBar(tr("Application"), this);
    applicationToolbar->setObjectName("Application");
    addToolBar(Qt::TopToolBarArea, applicationToolbar);
    
    applicationToolbar->addAction(m_actions.newAction);
    applicationToolbar->addAction(m_actions.saveAction);
    applicationToolbar->addAction(m_actions.loginLogoutAction);
    applicationToolbar->addAction(m_actions.printAction);
    applicationToolbar->addAction(m_actions.settingsAction);
    
    // ----- Search --------------------------------------------------------------------------------
    QToolBar* searchToolbar = new QToolBar(tr("Search"), this);
    searchToolbar->setObjectName("Search");
    addToolBar(Qt::TopToolBarArea, searchToolbar);
    m_searchLabel = new QLabel(tr("Search:")+" ", searchToolbar);
    
    m_searchCombo = new QComboBox(true, searchToolbar);
    //m_searchCombo->setFixedWidth(120);
    m_searchCombo->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    m_searchCombo->setObjectName("SearchCombo");
    m_searchCombo->setDuplicatesEnabled(false);
    m_searchCombo->setFocusPolicy(Qt::ClickFocus);
    m_searchCombo->setInsertionPolicy(QComboBox::AtTop);
    m_searchCombo->setAutoCompletion(true);
    
    searchToolbar->addWidget(m_searchLabel);
    searchToolbar->addWidget(m_searchCombo);
    searchToolbar->addAction(m_actions.searchAction);

    
    // ---- Edit toolbar ---------------------------------------------------------------------------
    QToolBar* editToolbar = new QToolBar(tr("Edit"), this);
    editToolbar->setObjectName("Edit");
    addToolBar(Qt::TopToolBarArea, editToolbar);
    
    editToolbar->addAction(m_actions.addItemAction);
    editToolbar->addAction(m_actions.removeItemAction);

    searchToolbar->adjustSize();
    editToolbar->adjustSize();
}


/*!
    Initializes the menubar.
 */
void Qpamat::initMenubar()
{
     // ----- File ---------------------------------------------------------------------------------
     QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(m_actions.newAction);
     fileMenu->addAction(m_actions.loginLogoutAction);
     fileMenu->addAction(m_actions.saveAction);
     fileMenu->addAction(m_actions.exportAction);
     fileMenu->addAction(m_actions.printAction);
     fileMenu->insertSeparator();
     fileMenu->addAction(m_actions.quitAction);
     
     // ----- Options ------------------------------------------------------------------------------
     QMenu* optionsMenu = menuBar()->addMenu(tr("&Options"));
     optionsMenu->addAction(m_actions.changePasswordAction);
     optionsMenu->addAction(m_actions.settingsAction);
     
     // ----- Extras -------------------------------------------------------------------------------
     QMenu* extrasMenu = menuBar()->addMenu(tr("&Extras"));
     extrasMenu->addAction(m_actions.randomPasswordAction);
     extrasMenu->addAction(m_actions.passwordStrengthAction);
     extrasMenu->addAction(m_actions.clearClipboardAction);
     
     // ----- Help ---------------------------------------------------------------------------------
     
     // this is for Motif/CDE style, but doesn't work in Qt 4.1.0, so disable it
     // menuBar()->insertSeparator();
     QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(m_actions.helpAction);
     helpMenu->insertSeparator();
     helpMenu->addAction(m_actions.aboutQtAction);
     helpMenu->addAction(m_actions.aboutAction);
}


/*!
    Saves the data if the application is closes.
 */
void Qpamat::exitHandler()
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
    QString rightpanelLayout;
    QTextStream rightpanelStream(&rightpanelLayout, QIODevice::WriteOnly);
    rightpanelStream << *m_rightPanel;
    set().writeEntry("Main Window/layout", saveState());
    set().writeEntry("Main Window/width", size().width());
    set().writeEntry("Main Window/height", size().height());
    set().writeEntry("Main Window/maximized", isMaximized());
    set().writeEntry("Main Window/rightpanelLayout", rightpanelLayout);
    
    emit quit();
}


/*!
    Handles a closeEvent of the main window. If the tray icon is visible, the application
    is just hidden. If no tray icon is used, the application is exited.
    
    \param e the QCloseEvent object
*/
void Qpamat::closeEvent(QCloseEvent* e)
{
    if (m_trayIcon)
    {
        handleTrayiconClick();
        e->ignore();
    }
    else
    {
        exitHandler();
        e->accept();
        QWidget::closeEvent(e);
    }
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
    Needed to connect with a signal that has no ActivationReason parameter.
*/
void Qpamat::handleTrayiconClick()
{
    handleTrayiconClick(QSystemTrayIcon::Trigger);
}

/*!
    Handles single click on the tray icon. If the window is shown, it is hidden. Else, it is
    shown again.
*/
void Qpamat::handleTrayiconClick(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        if (isShown())
        {
            m_lastGeometry = geometry();
            hide();
            m_actions.showHideAction->setMenuText(tr("&Show"));
        }
        else
        {
            if (!(m_lastGeometry.x() == 0 && m_lastGeometry.y() == 0
                        && m_lastGeometry.width() == 0 && m_lastGeometry.height() == 0))
            {
                setGeometry(m_lastGeometry);
            }
            show();
            m_actions.showHideAction->setMenuText(tr("&Hide")); 
        }
    }
}


/*!
    This function is called if the dock is activated (or destroyed). Don't really know
    what "activated" means here, just copied this from Psi.
*/
void Qpamat::dockActivated()
{
    if (isHidden())
    {
        show();
        dynamic_cast<DockTimeoutApplication*>(qApp)->clearReceiversToIgnore();
        delete m_trayIcon;
        m_trayIcon = 0;
    }
}


/*!
    Does the login.
 */
void Qpamat::login()
{
    PRINT_TRACE("Calling login()");
    
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
                    QMessageBox::NoButton, this, "qt_msgbox_information", true, 
                        Qt::WDestructiveClose);
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
    PRINT_TRACE("Caling setLogin = %d", loggedIn);
    m_loggedIn = loggedIn;
    
    // toggle action
    disconnect(m_actions.loginLogoutAction, SIGNAL(activated()), 0, 0 );
    if (loggedIn)
    {
        QIcon loginLogoutActionIcon(QPixmap(":/images/logout_16.png"));
        loginLogoutActionIcon.addPixmap(QPixmap(":/images/logout_24.png"));
        m_actions.loginLogoutAction->setIcon(loginLogoutActionIcon);
        m_actions.loginLogoutAction->setMenuText(tr("&Logout"));
        m_actions.loginLogoutAction->setToolTip(tr("Logout"));
        connect(m_actions.loginLogoutAction, SIGNAL(activated()), SLOT(logout()));
    }
    else
    {
        QIcon loginLogoutActionIcon(QPixmap(":/images/login_16.png"));
        loginLogoutActionIcon.addPixmap(QPixmap(":/images/login_24.png"));
        m_actions.loginLogoutAction->setIconSet(loginLogoutActionIcon);
        m_actions.loginLogoutAction->setMenuText(tr("&Login"));
        m_actions.loginLogoutAction->setToolTip(tr("Login"));
        connect(m_actions.loginLogoutAction, SIGNAL(activated()), SLOT(login()));
    }
    
    m_actions.saveAction->setEnabled(loggedIn);
    m_actions.changePasswordAction->setEnabled(loggedIn);
    m_actions.printAction->setEnabled(loggedIn);
    m_searchCombo->setEnabled(loggedIn);
    m_actions.searchAction->setEnabled(loggedIn);
    m_searchLabel->setEnabled(loggedIn);
    m_actions.addItemAction->setEnabled(loggedIn);
    m_actions.removeItemAction->setEnabled(loggedIn);
    m_actions.passwordStrengthAction->setEnabled(loggedIn);
    m_actions.exportAction->setEnabled(loggedIn);
    
    if (loggedIn)
    {
        dynamic_cast<DockTimeoutApplication*>(qApp)->setTimeout(
            set().readNumEntry("Security/AutoLogout")
        );
    }
    else
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
                QMessageBox::NoButton, this, "qt_msgbox_information", true, Qt::WDestructiveClose);

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
    QString fileName;
    
    QFileDialog* fd = new QFileDialog(this, tr("QPaMaT"), QDir::homeDirPath(), 
        tr("QPaMaT XML files (*.xml);;Text files with cleartext password (*.txt)"));
    fd->setMode(QFileDialog::AnyFile);
    bool oldCard = set().readBoolEntry("Smartcard/UseCard");
    
    if (fd->exec() == QDialog::Accepted)
    {
        fileName = fd->selectedFile();
    }
    else
    {
        return;
    }
    
    if (QFile::exists(fileName))
    {
        if (QMessageBox::question(this, tr("QPaMaT"), 
            tr("The file you've choosen exists. Do you overwrite it?"),
            tr("&Overwrite"), tr("&Don't export"), QString::null, 1, 1) == 1)
        {
            return;
        }
    }
    
    // XML or text?
    if (fd->selectedFilter().endsWith("(*.xml)"))
    {
        set().writeEntry("General/Datafile", fileName);
        set().writeEntry("Smartcard/UseCard", false);
        if (m_loggedIn && exportOrSave())
        {
            message("Wrote data successfully.");
        }
        set().writeEntry("General/Datafile", oldFilename);
        set().writeEntry("Smartcard/UseCard", oldCard);
    }
    else
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) 
        {
            QTextStream stream(&file);
            m_tree->appendTextForExport(stream);
            file.close();
            message("Wrote data successfully.");
        }
        else
        {
            QMessageBox::warning(this, tr("QPaMaT"),
               tr("An error occured while saving the file."),
               QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton);
        }
    }
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
        PRINT_TRACE("Disable timeout action temporary");
        dynamic_cast<DockTimeoutApplication*>(qApp)->setTemporaryDisabled(true);
        
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
        
        PRINT_TRACE("Enable timeout action again");
        dynamic_cast<DockTimeoutApplication*>(qApp)->setTemporaryDisabled(false);
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
         message(tr("Please enter a search criterion in the text field!"));
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
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage( TRUE );
    if ( printer.setup( this ) ) 
    {
        QPainter p(&printer);
        // Check that there is a valid device to print to.
        if (!p.device())
        {
            return;
        }

        QFont serifFont;
        QFont sansSerifFont;
        serifFont.fromString(set().readEntry("Presentation/NormalFont"));
        sansSerifFont.fromString(set().readEntry("Presentation/FooterFont"));
        p.setFont(sansSerifFont);
        
        qApp->setOverrideCursor( QCursor( Qt::WaitCursor ) );
        qApp->processEvents( QEventLoop::ExcludeUserInput );
        
        const int dpiy = p.device()->logicalDpiY();
        const int margin = (int) ((2/2.54)*dpiy); // 2 cm margins
        
        QRectF body(margin, margin, p.device()->width() - 2*margin, p.device()->height() - 2*margin);
        
        QTextDocument* doc = new QTextDocument();
        doc->setHtml(m_tree->toRichTextForPrint());
        doc->setDefaultFont(serifFont);
        QAbstractTextDocumentLayout* layout = doc->documentLayout();
        layout->setPaintDevice(&printer);
        doc->setPageSize(body.size());
        
        QRectF view(0, 0, body.width(), body.height());
        p.translate(body.left(), body.top());
        
        QString programString = tr("QPaMaT - Password managing tool for Unix, Windows and MacOS X");
        QString dateString = QDate::currentDate().toString(Qt::ISODate) + " / " +
            QTime::currentTime().toString("hh:mm");
    
        int page = 1;
        do 
        {
            qApp->processEvents( QEventLoop::ExcludeUserInput );
            QString pageS = tr("page") + " " + QString::number(page);
            
            QAbstractTextDocumentLayout::PaintContext ctx;
            p.setClipRect(view);
            ctx.clip = view;
            layout->draw(&p, ctx);
    
            p.setClipping(false);
            p.setFont(sansSerifFont);
            
            int x_pos = qRound(( view.left() + p.fontMetrics().width(programString)
                + view.right() - p.fontMetrics().width( pageS ) ) / 2.0
                - p.fontMetrics().width( dateString ) / 2.0 );
            int y_pos = qRound(view.bottom() + p.fontMetrics().ascent() + CON_MM(3));
            p.drawLine(qRound(view.left()), qRound(view.bottom()+CON_MM(3)), 
                qRound(view.right()), qRound(view.bottom()+CON_MM(3)));
            p.drawText( qRound(view.left()), y_pos, programString);
            p.drawText( x_pos, y_pos, dateString );
            p.drawText( qRound(view.right() - p.fontMetrics().width(pageS)), y_pos, pageS);
    
            view.translate(0, body.height());
            p.translate(0 , -body.height());
    
            if (view.top() >= layout->documentSize().height())
            {
                break;
            }
    
            printer.newPage();
            page++;
        }
        while (true);
        Q_ASSERT(page == doc->pageCount());
    
        delete doc;
        
        qApp->processEvents( QEventLoop::ExcludeUserInput );
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
    connect(m_tree, SIGNAL(selectionChanged(Q3ListViewItem*)),
        m_rightPanel, SLOT(setItem(Q3ListViewItem*)));
    connect(m_tree, SIGNAL(selectionCleared()), m_rightPanel, SLOT(clear()));
    
    
    // Actions
    connect(m_actions.newAction, SIGNAL(activated()), this, SLOT(newFile()));
    connect(m_actions.quitAction, SIGNAL(activated()), this, SLOT(exitHandler()));
    connect(m_actions.quitActionNoKeyboardShortcut, SIGNAL(activated()), this, SLOT(exitHandler()));
    connect(m_actions.showHideAction, SIGNAL(activated()), this, SLOT(handleTrayiconClick()));
    connect(m_actions.printAction, SIGNAL(activated()), this, SLOT(print()));
    connect(m_actions.saveAction, SIGNAL(activated()), this, SLOT(save()));
    connect(m_actions.exportAction, SIGNAL(activated()), this, SLOT(exportData()));
    
    connect(m_actions.changePasswordAction, SIGNAL(activated()), this, SLOT(changePassword()));
    connect(m_actions.settingsAction, SIGNAL(activated()), this, SLOT(configure()));
    
    connect(m_actions.helpAction, SIGNAL(activated()), &m_help, SLOT(showHelp()));
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
    connect(m_actions.searchAction, SIGNAL(activated()), this, SLOT(search()));
    
    // modified
    connect(m_tree, SIGNAL(stateModified()), SLOT(setModified()));
    connect(m_rightPanel, SIGNAL(stateModified()), SLOT(setModified()));
    
    // random password
    connect(m_rightPanel, SIGNAL(passwordLineEditGotFocus(bool)), m_randomPassword,
        SLOT(setInsertEnabled(bool)));
    connect(m_randomPassword, SIGNAL(insertPassword(const QString&)), 
        SIGNAL(insertPassword(const QString&)));
        
    // auto logout
    connect(dynamic_cast<DockTimeoutApplication*>(qApp), SIGNAL(timedOut()), SLOT(logout()));
    
    // tray icon
    if (set().readBoolEntry("Presentation/SystemTrayIcon") && 
            QSystemTrayIcon::isSystemTrayAvailable())
    {
        connect(qApp, SIGNAL(dockActivated()), SLOT(dockActivated()));
        connect(qApp, SIGNAL(trayOwnerDied()), SLOT(dockActivated()));
    }
    
    // previously I used a hidden action for this, but this doesn't work in Qt4 any more
    QShortcut* focusSearch = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_G), this);
    connect(focusSearch, SIGNAL(activated()), m_searchCombo, SLOT(setFocus()));
}


/*!
    Initializes the actions.
 */
void Qpamat::initActions()
{
    // ----- File ----------------------------------------------------------------------------------
    QIcon newActionIcon(QPixmap(":/images/stock_new_16.png"));
    newActionIcon.addPixmap(QPixmap(":/images/stock_new_24.png"));
    m_actions.newAction = new QAction(newActionIcon, tr("&New"), this);
    m_actions.newAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_N));
    
    QIcon quitActionIcon(QPixmap(":/images/stock_exit_16.png"));
    quitActionIcon.addPixmap(QPixmap(":/images/stock_exit_24.png"));
    m_actions.quitAction = new QAction(quitActionIcon, tr("E&xit"),this);
    m_actions.quitAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Q));
    
    QIcon quitActionNoKeyboardShortcutIcon(QPixmap(":/images/stock_exit_16.png"));
    quitActionNoKeyboardShortcutIcon.addPixmap(QPixmap(":/images/stock_exit_24.png"));
    m_actions.quitActionNoKeyboardShortcut = new QAction(quitActionNoKeyboardShortcutIcon,
        tr("E&xit"), this);
    
    QIcon loginLogoutActionIcon(QPixmap(":/images/login_16.png"));
    loginLogoutActionIcon.addPixmap(QPixmap(":/images/login_24.png"));
    m_actions.loginLogoutAction = new QAction(loginLogoutActionIcon, tr("&Login"), this);
    m_actions.loginLogoutAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_L));
    
    QIcon saveActionIcon(QPixmap(":/images/stock_save_16.png"));
    saveActionIcon.addPixmap(QPixmap(":/images/stock_save_24.png"));
    m_actions.saveAction = new QAction(saveActionIcon, tr("&Save"), this);
    m_actions.saveAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_S));
    
    QIcon exportActionIcon(QPixmap(":/images/export_16.png"));
    exportActionIcon.addPixmap(QPixmap(":/images/export_24.png"));
    m_actions.exportAction = new QAction(exportActionIcon, tr("&Export..."), this);
    
    QIcon printActionIcon(QPixmap(":/images/stock_print_16.png"));
    printActionIcon.addPixmap(QPixmap(":/images/stock_print_24.png"));
    m_actions.printAction = new QAction(printActionIcon, tr("&Print..."), this);
    m_actions.printAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_P));
    
    // ----- Options -------------------------------------------------------------------------------
    m_actions.changePasswordAction = new QAction(tr("&Change Password..."), this);
    
    QIcon settingsActionIcon(QPixmap(":/images/stock_preferences_16.png"));
    settingsActionIcon.addPixmap(QPixmap(":/images/stock_preferences_24.png"));
    m_actions.settingsAction = new QAction(settingsActionIcon, tr("&Settings..."), this);
    m_actions.settingsAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_T));
        
    // ----- Extras --------------------------------------------------------------------------------
    m_actions.randomPasswordAction = new QAction(tr("&Random Password..."), this);
    m_actions.randomPasswordAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_R));
    
    m_actions.passwordStrengthAction = new QAction(tr("&Show password strength"),this);
    m_actions.passwordStrengthAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_W));
    m_actions.passwordStrengthAction->setToggleAction(true);
    
    QIcon clearClipboardActionIcon(QPixmap(":/images/clear_clipboard_16.png"));
    clearClipboardActionIcon.addPixmap(QPixmap(":/images/clear_clipboard_24.png"));
    m_actions.clearClipboardAction = new QAction(clearClipboardActionIcon,
        tr("&Clear clipboard"), this);
    m_actions.clearClipboardAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_E));
    
    // ----- Help ----------------------------------------------------------------------------------
    QIcon helpActionIcon(QPixmap(":/images/stock_help_16.png"));
    helpActionIcon.addPixmap(QPixmap(":/images/stock_help__24.png"));
    m_actions.helpAction = new QAction(helpActionIcon, tr("&Help"), this);
    m_actions.helpAction->setShortcut(QKeySequence(Qt::Key_F1));
    
    QIcon aboutActionIcon(QPixmap(":/images/info_16.png"));
    aboutActionIcon.addPixmap(QPixmap(":/images/info_24.png"));
    m_actions.aboutAction = new QAction(aboutActionIcon, tr("&About..."), this);
    
    QIcon aboutQtActionIcon(QPixmap(":/images/qt_16.png"));
    m_actions.aboutQtAction = new QAction(aboutQtActionIcon,  tr("About &Qt..."), this);
    
    // ------ Toolbar ------------------------------------------------------------------------------
    QIcon addItemActionIcon(QPixmap(":/images/stock_add_16.png"));
    addItemActionIcon.addPixmap(QPixmap(":/images/stock_add_24.png"));
    m_actions.addItemAction = new QAction(addItemActionIcon, tr("Add item"), this);
    m_actions.addItemAction->setShortcut(QKeySequence(Qt::Key_Insert));
    
    QIcon removeItemActionIcon(QPixmap(":/images/stock_remove_16.png"));
    removeItemActionIcon.addPixmap(QPixmap(":/images/stock_remove_24.png"));
    m_actions.removeItemAction = new QAction(removeItemActionIcon, tr("Remove item"), this);
        
    // ------ Misc ---------------------------------------------------------------------------------
    QIcon searchActionIcon(QPixmap(":/images/stock_search_16.png"));
    searchActionIcon.addPixmap(QPixmap(":/images/stock_search_24"));
    m_actions.searchAction = new QAction(searchActionIcon, tr("Search"), this);
    
    m_actions.showHideAction = new QAction(tr("&Show"), this);
}

