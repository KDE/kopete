/***************************************************************************
                          buffer.h  -  description
                             -------------------
    begin                : Thu Jun 6 2002
    copyright            : (C) 2002 by twl6
    email                : twl6@paranoia.STUDENT.CWRU.Edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

#include <qobject.h>
#include <qlist.h>

/**The buffer that is sent to the oscar server
  *@author twl6
  */

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

struct SNAC { //snac header
	WORD family;
	WORD subtype;
	WORD flags;
	DWORD id;
};

struct TLV { //TLV
	WORD type;
	WORD length;
	char *data;
};

class Buffer : public QObject {
	Q_OBJECT
public: 
	Buffer(QObject *parent=0, const char *name=0);
	Buffer(char *b, Q_ULONG len, QObject *parent=0, const char *name=0);
	~Buffer();
	/** returns the actual buffer */
	inline char *getBuf(void) const { return buf; };
  /** adds the given string to the buffer (make sure it's NULL-terminated) */
  int addString(const char *, const WORD);
  /** adds the given DWord to the buffer */
  int addDWord(const DWORD);
  /** adds the given word to the buffer */
  int addWord(const WORD);
	/** returns the length of the buffer */
	inline int getLength(void) const { return length; };
  /** adds the given byte to the buffer */
  int addByte(const BYTE);
  /** deletes the current buffer */
  void clear();
  /** Adds a TLV with the given type and data */
  int addTLV(WORD, WORD, const char *);
  /** adds the given flap header to the beginning of the buffer, returns new buffer length */
  int addFlap(const BYTE channel);
  /** Prints out the buffer */
  void print() const;
	/** Returns a QString representation of the buffer */
	QString toString() const;
  /** Adds a SNAC to the end of the buffer with given family, subtype, flags, and request ID */
  int addSnac(const WORD, const WORD, const WORD, const DWORD);
  /** gets a Dword out of the buffer */
  DWORD getDWord();
  /** Gets a word out of the buffer */
  WORD getWord();
  /** Gets a byte out of the buffer */
  BYTE getByte();
	/** Gets a SNAC header from the head of the buffer */
  SNAC getSnacHeader();
  /** sets the buffer and length to the given values */
  void setBuf(char *, const WORD);
  /** Allocates memory for and gets a block of buffer bytes */
  char * getBlock(WORD len);
  /** adds a 16-bit long TLV */
  int addTLV16(const WORD type, const WORD data);
  /** adds the given byte to a TLV */
  int addTLV8(const WORD type, const BYTE data);
  /** Gets a TLV, storing it in a struct and returning it */
  TLV getTLV(void);
  /** Gets a list of TLV's */
  QList<TLV> getTLVList(void);
  /** appends a flap header to the end of the buffer w/ given length and channel */
  int appendFlap(const BYTE chan, const WORD len);
  /** Creates a chat data segment for a tlv and calls addTLV with that data */
  int addChatTLV(const WORD, const WORD, const QString &, const WORD);
  /** Gets a snac header out of the buffer */
signals: // Signals
  /** Emitted when an error occurs */
  void bufError(QString);
private: // Private attributes
  /** The length of the buffer */
  WORD length;
  /** The actual buffer */
  char * buf;
  /** The sequence number, incremented after every command sent to the oscar server */
  static WORD sequenceNum;
public slots: // Public slots
  /** Called when a buffer error occurs */
  void OnBufError(QString);
};

#endif
