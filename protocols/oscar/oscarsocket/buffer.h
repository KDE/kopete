/***************************************************************************
                          buffer.h  -  description
                             -------------------
    begin                : Thu Jun 6 2002

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
	Copyright (c) 2003 by Matt Rogers <matt.rogers@kdemail.net>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef BUFFER_H
#define BUFFER_H

#include "oscardebug.h"
#include "oscartypes.h"

#include <qptrlist.h>
#include <qcstring.h>

/**
 * A SNAC is the basic communication unit exchanged between client and server
 * and sits on top of the FLAP layer. There can be only one SNAC per FLAP frame.
 * SNACs are always sent on FLAP channel 0x02.
 */
struct SNAC
{
	/** Family ID number */
	WORD family;
	/** Family subtype ID number */
	WORD subtype;
	/** SNAC Flags */
	WORD flags;
	/** SNAC Request ID */
	DWORD id;
};

struct TLV
{
	WORD type;
	WORD length;
	char *data;
};


class QString;

class Buffer
{
	public:
		/** Default constructor */
		Buffer();
		/**
		 * \brief Creates a prefilled buffer
		 *
		 * Constructor that creates a prefilled buffer of @p len length
		 * that contains the data from @p b
		 */
		Buffer(char *b, Q_ULONG len);
		/** Default destructor */
		~Buffer();

		/**
		 * returns the raw buffer
		 */
		char *buffer() const;

		/**
		 * returns the length of the buffer
		 */
		int length() const;

		/**
		 * adds the given string to the buffer (make sure it's NULL-terminated)
		 */
		int addString(const char *, const DWORD);
		int addString(const unsigned char *, const DWORD);

		/**
		 * Little-endian version of addString
		 */
		int addLEString(const char *, const DWORD);

		/**
		 * adds the given string to the buffer with the length in front of it
		 * (make sure it's NULL-terminated)
		 */
		int addLNTS(const char * s);
		/**
		 * Little-endian version of addLNTS
		 */
		int addLELNTS(const char * s);

		/**
		 * adds the given DWord to the buffer
		 */
		int addDWord(const DWORD);

		/**
		 * adds the given word to the buffer
		 */
		int addWord(const WORD);

		/**
		 * adds the given word to the buffer in
		 * little-endian format as needed by old icq server
		 */
		int addLEWord(const WORD w);

		/**
		 * adds the given DWord to the buffer in
		 * little-endian format as needed by old icq server
		 */
		int addLEDWord(const DWORD dw);

		/**
		 * adds the given byte to the buffer
		 */
		int addByte(const BYTE);
		int addLEByte(const BYTE);

		/**
		 * deletes the current buffer
		 */
		void clear();

		/**
		 * Adds a TLV with the given type and data
		 */
		int addTLV(WORD, WORD, const char *);

		/**
		 * adds the given flap header to the beginning of the buffer,
		 * returns new buffer length
		 */
		int addFlap(const BYTE channel, const WORD flapSequenceNum);
		
		/**
		 * changes the flap sequence number
		 * (assumed to be already included in the buffer) to the
		 * passed parameter value, returns length
		 */
		int changeSeqNum(const WORD flapSequenceNum);

		/**
		 * Prints out the buffer, just for debug reasons
		 */
//		void print() const;

		/**
		 * Returns a QString representation of the buffer
		 */
		QString toString() const;

		/**
		 * Adds a SNAC to the end of the buffer with given
		 * family, subtype, flags, and request ID
		 */
		DWORD addSnac(const WORD family, const WORD subtype,
			const WORD flags = 0, DWORD id = 0);

		/**
		 * gets a DWord out of the buffer
		 */
		DWORD getDWord();

		/**
		 * Gets a word out of the buffer
		 */
		WORD getWord();

		/**
		 * Gets a byte out of the buffer
		 * It's not a constant method. It advances the buffer
		 * to the next BYTE after returning one.
		 */
		BYTE getByte();

		/**
		 * Same as above but returns little-endian
		 */
		WORD getLEWord();
		DWORD getLEDWord();
		BYTE getLEByte();

		/**
		 * Gets a SNAC header from the head of the buffer
		 */
		SNAC getSnacHeader();

		/**
		 * Reads the SNAC header from the buffer without removing it 
		 */
		SNAC readSnacHeader();

		/**
		 * sets the buffer and length to the given values
		 */
		void setBuf(char *, const WORD);

		/**
		 * Allocates memory for and gets a block of buffer bytes
		 */
		char *getBlock(WORD len);

		/**
		 * Allocates memory for and gets a block of buffer words
		 */
		WORD *getWordBlock(WORD len);

		/**
		 * Same as above but returning little-endian
		 */
		char *getLEBlock(WORD len);

		/**
		 * Convenience function that gets a LNTS (long null terminated string)
		 * from the buffer. Otherwise you'd need a getWord() + getBlock() call :)
		 */
		char *getLNTS();
		char *getLELNTS();

		/**
		 * adds a 16-bit long TLV
		 */
		int addTLV16(const WORD type, const WORD data);

		/**
		 * adds the given byte to a TLV
		 */
		int addTLV8(const WORD type, const BYTE data);

		/**
		 * Gets a TLV, storing it in a struct and returning it
		 */
		TLV getTLV();

		/**
		 * Gets a list of TLV's
		 */
		QPtrList<TLV> getTLVList(bool debug=false);

		/**
		 * Appends a FLAP header to the end of the buffer
		 * \param len - length of FLAP contents
		 * \param chan - FLAP channel
		 * \param flapSequenceNum - sequence for this FLAP
		 */
		int appendFlap(const BYTE chan, const WORD len, const WORD flapSequenceNum);

		/**
		 * Creates a chat data segment for a tlv and calls addTLV with that data
		 */
		int addChatTLV(const WORD, const WORD, const QString &, const WORD);

		/**
		 * Similar to the LNTS functions but string is NOT null-terminated
		 */
		int addBSTR(const char * s);
		char *getBSTR();

		int addBUIN(const char * s);
		char *getBUIN();

	private:
		/*
		 * Make the buffer bigger by @p inc bytes
		 */
		void expandBuffer(unsigned int inc);

	private:
		QByteArray mBuffer;

		char *mExtDataPointer;
		Q_ULONG mExtDataLen;
		unsigned int mReadPos;

};

#endif
// vim: set noet ts=4 sts=4 sw=4:
