/***************************************************************************
                          oscarfilesendconnection.h  -  description
                             -------------------
    begin                : Thu Jan 9 2003
    copyright            : (C) 2003 by Kopete developers
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OSCARFILESENDCONNECTION_H
#define OSCARFILESENDCONNECTION_H

#include <qfileinfo.h>
#include <oscarconnection.h>

/**Implementation of a direct connection used to send files
  *@author Kopete developers
  */

struct OFT2 { //OFT2 header
	WORD headerlen;
	WORD channel;
	char *cookie; //8 bytes
	WORD encrypt;
	WORD compression;
	WORD totfiles;
	WORD filesleft;
	WORD totparts;
	WORD partsleft;
	DWORD totsize;
	DWORD size;
	DWORD modtime;
	DWORD checksum;
	DWORD rfrcsum;
	DWORD rfsize;
	DWORD cretime;
	DWORD rfcsum;
	DWORD nrecvd;
	DWORD recvcsum;
	char *idstring; //32 bytes
	BYTE flags;
	BYTE lnameoffset;
	BYTE lsizeoffset;
	char *dummy; //69 bytes
	char *macinfo; //16 bytes
	WORD encode;
	WORD language;
	char *filename;
};

class OscarFileSendConnection : public OscarConnection  {
	Q_OBJECT
public: 
	OscarFileSendConnection(const QFileInfo &finfo, const QString &sn, const QString &connName, char cookie[8], QObject *parent=0, const char *name=0);
	~OscarFileSendConnection();
  /** Sends out an OFT2 block to the peer, using the specified header and buffer data */
  void sendOFT2Block(const OFT2 &oft, const Buffer &data, bool nullCookie);
  /** Calls OscarConnection::setSocket
			sends file send request header */
  virtual void setSocket( int socket );
  
private: // Private methods
  /** Gets an OFT2 header from the socket */
  OFT2 getOFT2(void);
  /** Sends request to the client telling he/she that we want to send this file */
  void sendFileSendRequest(void);
  /** Sends an acceptance of the information the peer has sent us and tell the peer we are ready for the file(s) */
  void sendAcceptTransfer(OFT2 &hdr);
  /** Sends the file to the peer, just raw data */
  void sendFile(void);
  /** Tells the peer we have received the file */
  void sendReadConfirm();
	
protected slots:
  /** Called when there is data to be read */
	virtual void slotRead(void);

private: // Private members
	/** Information on the file we are going to transfer */
	QFileInfo mFileInfo;
	/** Tells whether the peer is currently sending data */
	bool mSending;
	/** Tells how many bytes we have transferred */
	unsigned long mBytesTransferred;
	/** The actual file we are writing to or reading from */
	QFile mFile;
	/** The size of the file to be transferred */
	unsigned long mFileSize;

};

#endif
