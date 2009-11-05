/*
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
#include <QLayout>
#include <QFont>
#include <Q3FileDialog>
#include <QPushButton>
#include <QPixmap>
#include <QApplication>
#include <QFontDialog>
#include <QHBoxLayout>

#include "fontchoosebox.h"

/**
 * @class FontChooseBox
 *
 * @brief This class provides a not-editable QLineEdit combined with a button.
 *
 * The additional button lets the user choose a font which is available on the system.
 *
 * @ingroup widgets
 * @author Bernhard Walle
 */

/**
 * @fn FontChooseBox::fontChanged(const QFont&)
 *
 * This signal is emitted if the font has changed.
 *
 * @param font the new font
 */

/**
 * @brief Creates a new instance of a FontChooseBox widget.
 *
 * @param parent the parent widget
 * @param name the name of the widget which can be NULL.
 */
FontChooseBox::FontChooseBox(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    QHBoxLayout* boxLayout = new QHBoxLayout(this, 0, 2);
    m_lineEdit = new QLineEdit(this);

    // we need a tool button because it has no border and it looks better in this size
    // in Platin style
    m_fileDialogButton = new QToolButton(this);
    m_fileDialogButton->setIconSet(QIcon(QPixmap(":/images/stock_font_16.png")));

    boxLayout->addWidget(m_lineEdit);
    boxLayout->addWidget(m_fileDialogButton);

    connect(m_fileDialogButton, SIGNAL(clicked()), this, SLOT(chooseFont()));

    setFocusProxy(m_lineEdit);

    // make it read-only
    m_lineEdit->setReadOnly(true);

    m_fileDialogButton->setFocusPolicy(Qt::TabFocus);

    // connect
    connect(this, SIGNAL(fontChanged(const QFont&)), this, SLOT(setDisplayFont(const QFont&)));
}


/**
 * @property FontChooseBox::font
 *
 * Current font.
 */

/**
 * @brief Sets the font.
 *
 * The font is displayed in itself.
 *
 * @param font content the content string
 */
void FontChooseBox::setFont(const QFont& font)
{
    m_font = font;
    emit fontChanged(m_font);
}


/**
 * @brief This returns the current font.
 *
 * @return the font
 */
QFont FontChooseBox::getFont() const
{
    return m_font;
}


/**
 * @brief Sets the font to display the line edit
 *
 * @param font the font object
 */
void FontChooseBox::setDisplayFont(const QFont& font)
{
    QFont displayFont(font.family(), QApplication::font().pointSize());
    m_lineEdit->setFont(displayFont);
    m_lineEdit->setText(font.family() + " " + QString::number(font.pointSize()));
}


/**
 * @brief Slot that is called if the user pressed the button.
 *
 * It displays a QFontDialog and lets the user choose the font.
 */
void FontChooseBox::chooseFont()
{
     bool ok;
     QFont font = QFontDialog::getFont(&ok, m_font, this);

     if (ok)
         setFont(font);
}

// vim: set sw=4 ts=4 et: :tabSize=4:indentSize=4:maxLineLen=100:
