/*
    oscarfilesendconnection.cpp  -  Implementation of an oscar file send connection

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
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

#include "oscarfilesendconnection.h"

#include "oscardebug.h"
#include <unistd.h>

#include <kdebug.h>

OscarFileSendConnection::OscarFileSendConnection(const KFileItem *finfo, const QString &sn,
	const QString &connName, const QByteArray &cookie, QObject *parent, const char *name)
	: OscarConnection(sn, connName, SendFile, cookie, parent, name)
{
	kdDebug(14151) << k_funcinfo <<  "called, sn='" << sn << "' connName='" <<
		connName << "'" << endl;

	if (finfo)
		mFileInfo = new KFileItem(*finfo);
	else
		mFileInfo = 0L;
	mBytesTransferred = 0;
	mFileSize = 0;
	mSending = false;
	mModTime = 0;
	mCheckSum = 0;
}

OscarFileSendConnection::~OscarFileSendConnection()
{
	kdDebug(14151) << k_funcinfo << "Called." << endl;
}

void OscarFileSendConnection::slotRead()
{
	if (!mSending)
	{
		OFT2 hdr = getOFT2();

		if ( hdr.channel == 0x0101 ) //the peer is sending, we need to accept
		{
			mModTime = hdr.modtime;
			mCheckSum = hdr.checksum;
			sendAcceptTransfer(hdr);
			//emit transferBegun(this, hdr.filename, hdr.totsize, connectionName());
		}
		else if ( hdr.channel == 0x0202 ) //the peer has accepted, we must send the file
		{
			sendFile();
		}
		else if ( hdr.channel == 0x0204 ) //the peer has confirmed that the file is sent
		{
			emit transferComplete(connectionName());
			socket()->close();
			emit connectionClosed(connectionName());
		}
	}
	else //this is part of the file!
	{
		int bytesToRead;
		if(socket()->bytesAvailable() < (int)mFileSize)
			bytesToRead = socket()->bytesAvailable();
		else
			bytesToRead = mFileSize - mBytesTransferred;

		char *data = new char[bytesToRead];
		mBytesTransferred += socket()->readBlock(data, bytesToRead);
		mBuffer.addString(data, bytesToRead);

		emit percentComplete(100 * mBytesTransferred / mFileSize);

		mFile->resume(); //resume processing of KIO transfer
		delete [] data;
		if ( mBytesTransferred >= mFileSize ) //we are done
		{
			mSending = false;
			mFile->resume();
			kdDebug(14151) << "[OscarFileSendConnection] Sending read confirm.  filesize: " << mFileSize << ", bytes transferred: " << mBytesTransferred << endl;
			sendReadConfirm();
		}
	}
}

OFT2 OscarFileSendConnection::getOFT2()
{
	OFT2 oft;
	int theword, theword2;
	int start;
	//the ODC2 start byte
	if (((start = socket()->getch()) == 0x4f) &&
		((start = socket()->getch()) == 0x46) &&
		((start = socket()->getch()) == 0x54) &&
		((start = socket()->getch()) == 0x32))
	{
		//get the header length
		if ((theword = socket()->getch()) == -1)
		{
			kdDebug(14151) << "[OSCAR] Error reading length, byte 1: nothing to be read" << endl;
			oft.headerlen = 0x00;
		}
		else if((theword2 = socket()->getch()) == -1)
		{
			kdDebug(14151) << "[OSCAR] Error reading data field length, byte 2: nothing to be read" << endl;
			oft.headerlen = 0x00;
		}
		else
		{
			oft.headerlen = (theword << 8) | theword2;
		}

		//convert header to a buffer
		char *buf = new char[oft.headerlen-6];  // the -6 is there because we have already read 6 bytes
		socket()->readBlock(buf,oft.headerlen-6);
		Buffer inbuf;
		inbuf.setBuf(buf,oft.headerlen-6);

#ifdef OSCAR_PACKETLOG
		kdDebug(14151) << "=== INPUT ===" << inbuf.toString();
#endif

		oft.channel = inbuf.getWord();
		oft.cookie.assign( inbuf.getBlock(8), 8 );
		oft.encrypt = inbuf.getWord();
		oft.compression = inbuf.getWord();
		oft.totfiles = inbuf.getWord();
		oft.filesleft = inbuf.getWord();
		oft.totparts = inbuf.getWord();
		oft.partsleft = inbuf.getWord();
		oft.totsize = inbuf.getDWord();
		oft.size = inbuf.getDWord();
		oft.modtime = inbuf.getDWord();
		oft.checksum = inbuf.getDWord();
		oft.rfrcsum = inbuf.getDWord();
		oft.rfsize = inbuf.getDWord();
		oft.cretime = inbuf.getDWord();
		oft.rfcsum = inbuf.getDWord();
		oft.nrecvd = inbuf.getDWord();
		oft.recvcsum = inbuf.getDWord();
		oft.idstring = inbuf.getBlock(32);
		oft.flags = inbuf.getByte();
		oft.lnameoffset = inbuf.getByte();
		oft.lsizeoffset = inbuf.getByte();
		oft.dummy.assign( inbuf.getBlock(69), 69 );
		oft.macinfo.assign( inbuf.getBlock(16), 16 );
		oft.encode = inbuf.getWord();
		oft.language = inbuf.getWord();
		oft.filename = inbuf.getBlock(64);

		//convert "\" characters in filename to "/" characters
		for (int i=0;i<64;i++)
			if ( oft.filename[i] == '\\' )
				oft.filename[i] = '/';
	}
	else
	{
		kdDebug(14151) << "[OSCAR] Error reading OFT2 header... start byte is " << start << endl;
		oft.size = 0;
	}

	kdDebug(14151) << "[OSCAR] Read an OFT2 header!  header length: " << oft.headerlen
		<< ", channel: " << oft.channel << ", size: " << oft.size
		<< ", nrecvd: " << oft.nrecvd << ", file name: " << oft.filename << endl;

	return oft;
}

void OscarFileSendConnection::sendOFT2Block(const OFT2 &oft, const Buffer &/*data*/, bool nullCookie)
{
	Buffer outbuf;
	outbuf.addString("OFT2",4);
	outbuf.addWord(0x0100); //header length
	outbuf.addWord(oft.channel);
	//often a peer will send a cookie of all 0's, and we have to fill it in
	if ( nullCookie )
	{
		char tmp[8];
		for (int i=0;i<8;i++)
			tmp[i] = 0;
		outbuf.addString(tmp,8);
	}
	else
	{
		outbuf.addString(cookie().data(),8);
	}
	outbuf.addWord(oft.encrypt);
	outbuf.addWord(oft.compression);
	outbuf.addWord(oft.totfiles);
	outbuf.addWord(oft.filesleft);
	outbuf.addWord(oft.totparts);
	outbuf.addWord(oft.partsleft);
	outbuf.addDWord(oft.totsize);
	outbuf.addDWord(oft.size);
	outbuf.addDWord(oft.modtime);
	outbuf.addDWord(oft.checksum);
	outbuf.addDWord(oft.rfrcsum);
	outbuf.addDWord(oft.rfsize);
	outbuf.addDWord(oft.cretime);
	outbuf.addDWord(oft.rfcsum);
	outbuf.addDWord(oft.nrecvd);
	outbuf.addDWord(oft.recvcsum);
	char idstr[32] = "OFT_Windows ICBMFT V1.1 32";
	for (int i=26;i<32;i++)
		idstr[i] = 0;
	outbuf.addString(idstr,32);
	outbuf.addByte(oft.flags);
	outbuf.addByte(oft.lnameoffset);
	outbuf.addByte(oft.lsizeoffset);
	outbuf.addString(oft.dummy,69);
	outbuf.addString(oft.macinfo,16);
	outbuf.addWord(oft.encode);
	outbuf.addWord(oft.language);
	outbuf.addString(oft.filename.latin1(),oft.filename.length());
	for (int i=oft.filename.length();i<64;i++)
		outbuf.addByte(0x00);

#ifdef OSCAR_PACKETLOG
		kdDebug(14151) << "=== OUTPUT ===" << outbuf.toString();
#endif
	socket()->writeBlock(outbuf.buffer(), outbuf.length());
}

void OscarFileSendConnection::sendFileSendRequest(void)
{
	OFT2 oft;
	oft.channel = 0x0101;
	oft.encrypt = 0x0000;
	oft.compression = 0x0000;

	// TODO: these will need to be changed when we go for multiple file support
	oft.totfiles = 0x0001;
	oft.filesleft = 0x0001;

	oft.totparts = 0x0001;
	oft.partsleft = 0x0001;

	// TODO: this will need to be changed when we go for multiple file support
	oft.totsize = mFileInfo->size();

	oft.size = mFileInfo->size();
	oft.modtime = mFileInfo->time(KIO::UDS_MODIFICATION_TIME);

	oft.checksum = 0x00000000; //we might get crap about this
	oft.rfrcsum = 0x00000000;
	oft.rfsize = 0x00000000;
	oft.cretime = mFileInfo->time(KIO::UDS_CREATION_TIME);
	oft.rfcsum = 0x00000000;
	oft.nrecvd = 0x00000000;
	oft.recvcsum = 0x00000000;
	oft.flags = 0x02;
	oft.lnameoffset = 0x00;
	oft.lsizeoffset = 0x00;
	oft.dummy.resize(69);
	for (int i=0;i<69;i++)
		oft.dummy[i] = 0;
	oft.macinfo.resize(16);
	for (int i=0;i<16;i++)
		oft.macinfo[i] = 0;
	oft.encode = 0x0000;
	oft.language = 0x0000;
	oft.filename = mFileInfo->url().fileName();

	mFileSize = mFileInfo->size();
	mFileName = mFileInfo->url().fileName();
	Buffer thebuf;
	sendOFT2Block(oft, thebuf, true);
}

void OscarFileSendConnection::sendAcceptTransfer(OFT2 &hdr)
{
	hdr.channel = 0x0202; //this means accept the transfer

	mFileSize = hdr.size;
	mFileName = hdr.filename;

	Buffer outbuf;
	sendOFT2Block(hdr, outbuf, false);

	if(!mFileInfo)
	{
		kdDebug(14151) << k_funcinfo <<
			"mfileinfo is null :-(  can't accept transfer" << endl;
		return;
	}

	KURL ku = mFileInfo->url();
	ku.setFileName(hdr.filename);
	mFileInfo->setURL(ku);

	kdDebug(14151) << "[OscarFileSendConnection] Accepting transfer of " <<
		mFileInfo->url().path() << ", size: " << mFileSize << endl;

	mFile = KIO::put(mFileInfo->url(), -1, true, false, false);
	mFile->suspend();

	connect( mFile, SIGNAL(result(KIO::Job*)),
		this, SLOT(slotKIOResult(KIO::Job*)) );

	//allow the KIO to write to file
	connect( mFile, SIGNAL(dataReq(KIO::Job*, QByteArray &)),
		this, SLOT(slotKIODataReq(KIO::Job*, QByteArray &)) );

	mSending = true;
}

void OscarFileSendConnection::sendFile()
{
	mSending = true;
	kdDebug(14151) << k_funcinfo <<  "The transfer of " <<
		mFileInfo->url().path() << " has begun." << endl;

	emit transferBegun(this, mFileName, mFileSize, connectionName());
	mFile = KIO::get(mFileInfo->url(), true, true);

	connect( this, SIGNAL(bytesWritten( int )),
		this, SLOT(slotBytesWritten( int )) );
	connect( mFile, SIGNAL(result(KIO::Job*)),
		this, SLOT(slotKIOResult(KIO::Job*)) );
	connect( mFile, SIGNAL(data(KIO::Job*, const QByteArray &)),
		this, SLOT(slotKIOData(KIO::Job*, const QByteArray &)) );
}

void OscarFileSendConnection::sendReadConfirm()
{
	OFT2 oft;
	oft.channel = 0x0204;
	oft.encrypt = 0x0000;
	oft.compression = 0x0000;

	//these will need to be changed when we go for multiple file support
	oft.totfiles = 0x0001;
	oft.filesleft = 0x0001;

	oft.totparts = 0x0001;
	oft.partsleft = 0x0001;

	//this will need to be changed when we go for multiple file support
	oft.totsize = mFileInfo->size();

	oft.size = mFileInfo->size();
	oft.modtime = mModTime;
	//oft.modtime = mFileInfo->time( KIO::UDS_MODIFICATION_TIME);

	oft.checksum = mCheckSum; //0x00000000; //we might get crap about this
	oft.rfrcsum = 0x00000000;
	oft.rfsize = 0x00000000;
	oft.cretime = mFileInfo->time( KIO::UDS_CREATION_TIME );
	oft.rfcsum = 0x00000000;
	oft.nrecvd = mBytesTransferred;
	oft.recvcsum = mCheckSum;
	oft.flags = 0x20;
	oft.lnameoffset = 0x00;
	oft.lsizeoffset = 0x00;
	oft.dummy.resize(69);
	for (int i=0;i<69;i++)
		oft.dummy[i] = 0;
	oft.macinfo.resize(16);
	for (int i=0;i<16;i++)
		oft.macinfo[i] = 0;
	oft.encode = 0x0000;
	oft.language = 0x0000;
	oft.filename = mFileName;

	Buffer thebuf;
	sendOFT2Block(oft, thebuf, false);
}

void OscarFileSendConnection::slotKIOResult(KIO::Job *job)
{
	if (job->error())
	{
		kdDebug(14151) << k_funcinfo << "Error on transferring file" << endl;
		job->showErrorDialog();
	}
	else
	{
		mSending = false;
		kdDebug(14151) << k_funcinfo << "Finished transferring file" << endl;
	}
}

void OscarFileSendConnection::slotKIOData(KIO::Job * /* job */, const QByteArray &data)
{
	//kdDebug(14151) << "[OscarFileSendConnection] got data " << ((KIO::SimpleJob *)(job))->url().fileName() << ", size " << data.size() << endl;
	socket()->writeBlock(data.data(),data.size());
}

void OscarFileSendConnection::slotKIODataReq(KIO::Job *job, QByteArray &data)
{
	int len = mBuffer.length();
	data.assign(mBuffer.getBlock(len), len);
/*
	kdDebug(14151) << k_funcinfo << "Writing data " <<
		((KIO::SimpleJob *)(job))->url().fileName() <<
		", size " << data.size() << endl;
*/
	if(mSending)
		((KIO::TransferJob *)job)->suspend();
}

void OscarFileSendConnection::slotBytesWritten(int nBytes)
{
	mBytesTransferred += nBytes;

	emit percentComplete(100 * mBytesTransferred / mFileSize);

	if ( mBytesTransferred >= mFileSize )
	{
		disconnect(this, SIGNAL(bytesWritten(int)),
			this, SLOT(slotBytesWritten(int)));
	}
}

#include "oscarfilesendconnection.moc"
// vim: set noet ts=4 sts=4 sw=4:
