/*
    oscarfilesendconnection.h  -  Implementation of an oscar file transfer connection

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARFILESENDCONNECTION_H
#define OSCARFILESENDCONNECTION_H

#include <kio/job.h>
#include <kfileitem.h>
#include "oscarconnection.h"

/**Implementation of a direct connection used to send files
  *@author Kopete developers
  */

struct OFT2
{ //OFT2 header
	WORD headerlen;
	WORD channel;
	QByteArray cookie; //8 bytes
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
	QString idstring; //32 bytes
	BYTE flags;
	BYTE lnameoffset;
	BYTE lsizeoffset;
	QByteArray dummy; //69 bytes
	QByteArray macinfo; //16 bytes
	WORD encode;
	WORD language;
	QString filename;
};

class OscarFileSendConnection : public OscarConnection
{
	Q_OBJECT
	public:
		OscarFileSendConnection(const KFileItem *finfo, const QString &sn, const QString &connName,
			const QByteArray &cookie, QObject *parent=0, const char *name=0);
		~OscarFileSendConnection();

		/** Sends out an OFT2 block to the peer, using the specified header and buffer data */
		void sendOFT2Block(const OFT2 &oft, const Buffer &data, bool nullCookie);

		/** Sends request to the client telling he/she that we want to send this file */
		virtual void sendFileSendRequest(void);

	private: // Private methods
		/** Gets an OFT2 header from the socket */
		OFT2 getOFT2(void);
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
		KFileItem *mFileInfo;
		/** Tells whether the peer is currently sending data */
		bool mSending;
		/** Tells how many bytes we have transferred */
		unsigned long mBytesTransferred;
		/** The actual file we are writing to or reading from */
		KIO::TransferJob *mFile;
		/** The size of the file to be transferred */
		unsigned long mFileSize;
		/** The read buffer (for file data) */
		Buffer mBuffer;
		/** The name (from peer) of the file to be transferred */
		QString mFileName;
		/** The checksum reported by the peer */
		DWORD mCheckSum;
		/** The mod time reported by peer */
		DWORD mModTime;

	private slots: // Private slots
		/** Called when the kio job is done */
		void slotKIOResult(KIO::Job *);
		/** Called when the KIO job sends data */
		void slotKIOData(KIO::Job *job, const QByteArray &data);
		/** Called when the KIO slave wants data */
		void slotKIODataReq(KIO::Job *job, QByteArray &data);
		/** Called when bytes are written */
		void slotBytesWritten( int nBytes );
};

#endif
