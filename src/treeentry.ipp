/*
 * Id: $Id: treeentry.ipp,v 1.5 2003/12/29 15:12:27 bwalle Exp $
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


/*!
    Creates a new TreeEntry.
    \param parent the parent entry. There must be a parent, i.e. parent must not be 0.
    \param name the name of the entry. This property may be set later.
    \param isCategory whether the entry is a category
*/
template<class T>
TreeEntry::TreeEntry(T* parent, const QString& name, bool isCategory)
    : QListViewItem(parent), m_name(name), m_isCategory(isCategory), m_weak(false)
{
    setRenameEnabled(0, true);
    setDragEnabled(true);
    setDropEnabled(true);
    m_properties.setAutoDelete(true);
}


/*!
    Creates a Propery element from a XML \c property tag.
    \param parent the parent
    \param element the \c property or \c element tag
    \param enc the encryptor to use for decrypting passwords
    \return the appended value
*/
template<class T>
TreeEntry* TreeEntry::appendFromXML(T* parent, QDomElement& element, StringEncryptor& enc)
{
    QString name = element.attribute("name");
    bool isCategory = element.tagName() == "category";
    TreeEntry* returnvalue = new TreeEntry(parent, name, isCategory);
    QDomNode node = element.firstChild();
    QDomElement childElement;
    while ( !node.isNull() ) 
    {
        if (node.isElement())
        {
            childElement = node.toElement();
            if (isCategory)
            {
                TreeEntry::appendFromXML(returnvalue, childElement, enc);
                returnvalue->setOpen(element.attribute("wasOpen", "0") == "1");
            }
            else
            {
                Property::appendFromXML(returnvalue, childElement, enc);
            }
            node = node.nextSibling();
        }
    }
    return returnvalue;
}

