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
#ifndef MEMORYCARD_H
#define MEMORYCARD_H

#include <stdexcept>

#include <qstring.h>
#include <qlibrary.h>
#include <qobject.h>

#include "global.h"
#include "nosuchlibraryexception.h"
#include "notinitializedexception.h"
#include "cardexception.h"

class MemoryCard
{
    public:
        typedef char (*CT_init_ptr) (unsigned short ctn, unsigned short pn);
        typedef char (*CT_data_ptr) (unsigned short ctn, unsigned char* dad, unsigned char* sad,
                                     unsigned short lenc, unsigned char* command,
                                     unsigned short* lenr, unsigned char* response);
        typedef char (*CT_close_ptr) (unsigned short ctn);

        enum CardSlot {
            ICC1 = 0x00,
            ICC2 = 0x02,
            ICC3,
            ICC4,
            ICC5,
            ICC6,
            ICC7,
            ICC8,
            ICC9,
            ICC10,
            ICC11,
            ICC12,
            ICC13,
            ICC14
        };

        enum CardType {
            TMemoryCard,
            TProcessorCard,
            OtherResponse
        };

        enum ProtocolType {
            TI2C,
            T2Wire,
            T3Wire,
            TISOProtocol,
            TOther
        };

    public:
        MemoryCard(QString library) throw (NoSuchLibraryException);
        virtual ~MemoryCard();

        void setWaitTime(unsigned char newTime);
        unsigned char getWaitTime() const;

        void init(int portNumber) throw (CardException);
        void close() throw (CardException, NotInitializedException);

        CardType getType() const throw (CardException, NotInitializedException);

        void getStatusInformation(QString* manufacturer, QString* terminalType = 0,
            QString* software = 0, QString* discrData = 0)
            throw (CardException, NotInitializedException);

        void resetCard(int* capacity = 0, ProtocolType* protocolType = 0) const
            throw (NotInitializedException, CardException);;

        bool selectFile() const
            throw (NotInitializedException, CardException);

        void verify(const QString& pin) const
            throw (std::invalid_argument, NotInitializedException, CardException);

        void changeVerificationData(const QString& oldPin, const QString& newPin)
            throw (NotInitializedException, CardException);

        ByteVector read(unsigned short offset, unsigned short length)
            throw (CardException, NotInitializedException);

        void write(unsigned short offset, const ByteVector& data)
            throw (CardException, NotInitializedException);

    private:
        void checkInitialzed(const QString& = QString::null) const
        throw (NotInitializedException);

        void createPIN(QString pin, unsigned char* pinBytes) const
        throw (std::invalid_argument);

    private:
        QLibrary        m_library;
        CT_init_ptr     m_CT_init_function;
        CT_data_ptr     m_CT_data_function;
        CT_close_ptr    m_CT_close_function;
        unsigned short  m_portNumber;           // pn in CT-API jargon
        unsigned short  m_cardTerminalNumber;   // ctnin CT-API jargon
        bool            m_initialized;
        unsigned char   m_waitTime;

    private:
        static int      m_lastNumber;
};

#endif // MEMORYCARD_H

// vim: set sw=4 ts=4 et: :tabSize=4:indentSize=4:maxLineLen=100:
