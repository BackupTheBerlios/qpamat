/*
 * Id: $Id: treeentry.h,v 1.9 2003/12/21 20:31:00 bwalle Exp $
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
#ifndef TREEENTRY_H
#define TREEENTRY_H

#include <qstring.h>
#include <qptrlist.h>
#include <qlistview.h>
#include <qmime.h>
#include <qevent.h>
#include <qvaluelist.h>

#include "property.h"
#include "security/stringencryptor.h"

/*!
 * Vector of Properties.
 */
typedef QPtrList<Property> PropertyPtrList;


/*!
 * \brief Represents an entry in the tree.
 *
 * \ingroup gui
 * \author Bernhard Walle
 * \version $Revision: 1.9 $
 * \date $Date: 2003/12/21 20:31:00 $
 */
class TreeEntry : public QObject, public QListViewItem
{
    Q_OBJECT
    
    using QListViewItem::parent;
    
    public:
        
        /*!
         * The iterator for the proeprties
         */
        typedef QPtrListIterator<Property> PropertyIterator;
        
        /*!
         * Creates a new TreeEntry.
         * \param parent the parent entry. There must be a parent, i.e. parent must not be 0.
         * \param name the name of the entry. This property may be set later.
         * \param isCategory whether the entry is a category
         */
        template<class T>
        TreeEntry(T* parent, const QString& name = QString::null, bool isCategory = false);
        
        /*!
         * Deletes a property
         */
        virtual ~TreeEntry() { }
        
        /*!
         * Returns the name of the entry.
         */
        QString getName() const;
        
        /*!
         * Returns an iterator to the first entry of the properties list.
         * \return the iterator
         */
        Property* getProperty(uint index);
        
        /*!
         * Inserts a new property at the end.
         * \param property the property
         */
        void appendProperty(Property* property);
        
        /*!
         * Returns whether the entry is a categroy.
         */
        bool isCategory() const;
        
        /*!
         * Returns an iterator for the list
         */
        PropertyIterator propertyIterator() const;
        
        /*!
         * Checks if the children contain week passwords.
         * \return \c true if they do contain week passwords, \c false otherwise
         */
        bool hasWeakChildren() const;
        
         /*!
         * Appends the treeentry as \c category or \c entry tag in the XML structure.
         * \param document the document needed to create new elements
         * \param parent the parent to which the new created element should be attached
         * \param enc the encryptor to use for encrypting passwords
         */
        void appendXML(QDomDocument& document, QDomNode& parent, StringEncryptor& enc) const;
        
        /*!
         * Creates a Propery element from a XML \c property tag.
         * \param parent the parent
         * \param element the \c property or \c element tag
         * \param enc the encryptor to use for decrypting passwords
         * \return the appended value
         */
        template<class T>
        static TreeEntry* appendFromXML(T* parent, QDomElement& element, StringEncryptor& enc);
        
        /*!
         * Returns the name of the entry.
         * \param column the column
         * \return the name
         * @sa QListViewItem::text
         */
        QString text(int column) const;
        
        /*!
         * Sets the text of the entry. Sets the name internally.
         * \param column the column
         * \param text the new text
         */
        void setText(int column, const QString& text);
        
        /*!
         * This function converts a tree entry to HTML for printing. A TreeEntry represents one
         * big table.
         * \return the RichText
         */
        QString toRichTextForPrint() const;
        
        /*!
         * Converts this TreeEntry to XML. This XML is used for drag and drop. It contains one
         * \<entry\> or \<category\> tag and this tag contains also an attribute named
         * \c memoryAddress which holds the memory address for this item. This can be used
         * for deleting the object or for determine whether the user is dragging to iself.
         * \return the XML string
         */
        QString toXML() const;
        
        /*!
         * Checks if the item can accept drops of the type QMimeSource. The MIME type accepted is
         * <tt>application/x-qpamat</tt>. Only categories accept drops.
         * \param mime the QMimeSource object
         * \return \c true if the item can accept drops of type QMimeSource mime; otherwise 
         *         \c false. 
         */
        bool acceptDrop(const QMimeSource* mime) const;
        
    public slots:
    
        /*!
         * Moves the given property one step up.
         * \param index the index of the property
         */
        void movePropertyOneUp(uint index);
        
        /*!
         * Moves the given property one step down.
         * \param index the index of the property
         */
        void movePropertyOneDown(uint index);
        
        /*!
         * Deletes the property with the specified index
         * \param index the index
         */
        void deleteProperty(uint index);
        
        /*!
         * Removes all properties.
         */
        void deleteAllProperties();
        
    signals:
        
        /*!
         * Fired is a property was added.
         */
        void propertyAppended();
        
    protected:
        
        /*!
         * Overwritten drop handler
         * \param evt the event
         */
        void dropped(QDropEvent *evt);
        
        /*!
         * Reimplemented to draw red text
         * \param p the painter
         * \param cg the color group
         * \param col the column
         * \param width the width
         * \param align the alignment
         */
        //void paintCell(QPainter* p, const QColorGroup& cg, int col, int width, int align);
        
        
    private:
        QString             m_name;
        PropertyPtrList     m_properties;
        bool                m_isCategory;
        bool                m_weak;
        
    private:
        void init();
};

#include "treeentry.ipp"

#endif // TREEENTRY_H
