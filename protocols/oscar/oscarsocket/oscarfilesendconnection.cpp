/*
    oscarfilesendconnection.cpp  -  Implementation of an oscar file send connection

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

#include <kdebug.h>
#include <unistd.h>
#include "oscardebugdialog.h"
#include "oscarfilesendconnection.h"

OscarFileSendConnection::OscarFileSendConnection(const KFileItem *finfo, const QString &sn,
	const QString &connName, char cookie[8], QObject *parent, const char *name)
	: OscarConnection(sn, connName, SendFile, cookie, parent, name)
{
	if (finfo)
		mFileInfo = new KFileItem(*finfo);
	else
		mFileInfo = 0L;
	mBytesTransferred = 0;
	mFileSize = 0;
	mSending = false;
}

OscarFileSendConnection::~OscarFileSendConnection()
{
}

/** Called when there is data to be read */
void OscarFileSendConnection::slotRead(void)
{
	if ( !mSending)
	{
		OFT2 hdr = getOFT2();

		if ( hdr.channel == 0x0101 ) //the peer is sending, we need to accept
		{
			sendAcceptTransfer(hdr);
		}
		else if ( hdr.channel == 0x0202 ) //the peer has accepted, we must send the file
		{
			sendFile();
		}
		else if ( hdr.channel == 0x0204 ) //the peer has confirmed that the file is sent
		{
			emit transferComplete(connectionName());
			close();
			emit connectionClosed(connectionName());
		}
		if (hdr.filename)
			delete [] hdr.filename;
		if (hdr.dummy)
			delete [] hdr.dummy;
		if (hdr.cookie)
			delete [] hdr.cookie;
		if (hdr.idstring)
			delete [] hdr.idstring;
		if (hdr.macinfo)
			delete [] hdr.macinfo;
	}
	else //this is part of the file!
	{
		int bytesToRead;
		if ( bytesAvailable() < mFileSize )
			bytesToRead = bytesAvailable();
		else
			bytesToRead = mFileSize - mBytesTransferred;

		char *data = new char[bytesToRead];
		mBytesTransferred += readBlock(data,bytesToRead);
    mBuffer.addString(data,bytesToRead);
    mFile->resume(); //resume processing of KIO transfer
		delete [] data;
		if ( mBytesTransferred >= mFileSize ) //we are done
		{
			mSending = false;
			mFile->resume();
			kdDebug() << "[OscarFileSendConnection] Sending read confirm.  filesize: " << mFileSize << ", bytes transferred: " << mBytesTransferred << endl;
			sendReadConfirm();
		}
		if ( bytesAvailable() )
			emit readyRead();
	}
}	

/** Gets an OFT2 header from the socket */
OFT2 OscarFileSendConnection::getOFT2(void)
{
	OFT2 oft;
	int theword, theword2;
  int start;
	//the ODC2 start byte
  if (((start = getch()) == 0x4f) &&
    		((start = getch()) == 0x46) &&
      	((start = getch()) == 0x54) &&
        ((start = getch()) == 0x32))
	{
		//get the header length
		if ((theword = getch()) == -1) {
			kdDebug() << "[OSCAR] Error reading length, byte 1: nothing to be read" << endl;
			oft.headerlen = 0x00;
		} else if((theword2 = getch()) == -1){
			kdDebug() << "[OSCAR] Error reading data field length, byte 2: nothing to be read" << endl;
			oft.headerlen = 0x00;
		} else {
			oft.headerlen = (theword << 8) | theword2;
		}
 	
		//convert header to a buffer
		char *buf = new char[oft.headerlen-6];  // the -6 is there because we have already read 6 bytes
		readBlock(buf,oft.headerlen-6);
		Buffer inbuf;
		inbuf.setBuf(buf,oft.headerlen-6);
 	
		//inbuf.print();
		if(hasDebugDialog()){
			debugDialog()->addMessageFromServer(inbuf.toString(),connectionName());
		}
		oft.channel = inbuf.getWord();
		oft.cookie = inbuf.getBlock(8);
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
		oft.dummy = inbuf.getBlock(69);
		oft.macinfo = inbuf.getBlock(16);
		oft.encode = inbuf.getWord();
		oft.language = inbuf.getWord();
		oft.filename = inbuf.getBlock(64);
	}
	else
	{
		kdDebug() << "[OSCAR] Error reading OFT2 header... start byte is " << start << endl;
		oft.size = 0;
	}

	kdDebug() << "[OSCAR] Read an OFT2 header!  header length: " << oft.headerlen
		<< ", channel: " << oft.channel << ", size: " << oft.size
		<< ", nrecvd: " << oft.nrecvd << ", file name: " << oft.filename << endl;

	return oft;		
}

/** Sends out an OFT2 block to the peer, using the specified header and buffer data */
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
		outbuf.addString(cookie(),8);
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
	outbuf.addString(oft.filename,64);

	if(hasDebugDialog()){
		debugDialog()->addMessageFromClient(outbuf.toString(),connectionName());
	}
	writeBlock(outbuf.getBuf(),outbuf.getLength());
}

/** Calls OscarConnection::setSocket
		sends file send request header */
void OscarFileSendConnection::setSocket( int socket )
{
	OscarConnection::setSocket(socket);
	sendFileSendRequest();
}

/** Sends request to the client telling he/she that we want to send this file */
void OscarFileSendConnection::sendFileSendRequest(void)
{
	OFT2 oft;
	oft.channel = 0x0101;
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
	oft.dummy = new char[69];
	for (int i=0;i<69;i++)
		oft.dummy[i] = 0;
	oft.macinfo = new char[16];
	for (int i=0;i<16;i++)
		oft.macinfo[i] = 0;
	oft.encode = 0x0000;
	oft.language = 0x0000;
	oft.filename = new char[64];
	for (unsigned int i=0;i<mFileInfo->url().fileName().length();i++)
	{
		oft.filename[i] = mFileInfo->url().fileName()[i].latin1();	
	}
	for (unsigned int i=mFileInfo->url().fileName().length();i<64;i++)
		oft.filename[i] = 0;

	Buffer thebuf;
	sendOFT2Block(oft, thebuf, true);
	delete [] oft.dummy;
	delete [] oft.macinfo;
	delete [] oft.filename;
}

/** Sends an acceptance of the information the peer has sent us and tell the peer we are ready for the file(s) */
void OscarFileSendConnection::sendAcceptTransfer(OFT2 &hdr)
{
	hdr.channel = 0x0202; //this means accept the transfer

	mFileSize = hdr.size;
	kdDebug(14150) << "[OscarFileSendConnection] Accepting transfer of " << hdr.filename << ", size: " << mFileSize << endl;

	Buffer outbuf;
	sendOFT2Block(hdr, outbuf, false);

	if ( !mFileInfo )
	{
		kdDebug(14150) << k_funcinfo << "mfileinfo is null :-(  can't accept transfer" << endl;
		return;
	}
	
 	mFile = KIO::put(mFileInfo->url(), -1, true, false, true);
	mFile->suspend();
	connect( mFile, SIGNAL(result(KIO::Job*)),
		this, SLOT(slotKIOResult(KIO::Job*)) );
  //allow the KIO to write to file
  connect( mFile, SIGNAL(dataReq(KIO::Job*, QByteArray &)),
		this, SLOT(slotKIODataReq(KIO::Job*, QByteArray &)) );

	mSending = true;
}

/** Sends the file to the peer, just raw data */
void OscarFileSendConnection::sendFile(void)
{
	mSending = true;
	mFile = KIO::get(mFileInfo->url(), true, true);
	connect( mFile, SIGNAL(result(KIO::Job*)),
  	this, SLOT(slotKIOResult(KIO::Job*)) );
  connect( mFile, SIGNAL(data(KIO::Job*, const QByteArray &)),
  	this, SLOT(slotKIOData(KIO::Job*, const QByteArray &)) );
}

/** Tells the peer we have received the file */
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
	oft.totsize = 0x00000000;

	oft.size = 0x00000000;
	oft.modtime = 0x00000000;
	//oft.modtime = mFileInfo.lastModified().toTime_t();

	oft.checksum = 0x00000000; //we might get crap about this
	oft.rfrcsum = 0x00000000;
	oft.rfsize = 0x00000000;
	oft.cretime = 0x00000000;
	//oft.cretime = mFileInfo.created().toTime_t();
	oft.rfcsum = 0x00000000;
	oft.nrecvd = mBytesTransferred;
//	mFile.open(IO_ReadOnly);
//	QByteArray data = mFile.readAll();
//	oft.recvcsum = qChecksum(data.data(),data.size());
	oft.recvcsum = 0x00000000;
	oft.flags = 0x02;
	oft.lnameoffset = 0x00;
	oft.lsizeoffset = 0x00;
	oft.dummy = new char[69];
	for (int i=0;i<69;i++)
		oft.dummy[i] = 0;
	oft.macinfo = new char[16];
	for (int i=0;i<16;i++)
		oft.macinfo[i] = 0;
	oft.encode = 0x0000;
	oft.language = 0x0000;
	oft.filename = new char[64];
//	for (unsigned int i=0;i<mFile.name().length();i++)
//	{
//		oft.filename[i] = mFile.name()[i].latin1();
//	}
//	for (unsigned int i=mFile.name().length();i<64;i++)
//		oft.filename[i] = 0;

	Buffer thebuf;
	sendOFT2Block(oft, thebuf, false);
	delete [] oft.dummy;
	delete [] oft.macinfo;
	delete [] oft.filename;
}

/** Called when the kio job is done */
void OscarFileSendConnection::slotKIOResult(KIO::Job *job)
{
	if (job->error())
  	job->showErrorDialog();
  else
  {
		mSending = false;
		kdDebug(14150) << "[OscarFileSendConnection] Finished transferring file" << endl;
  }
}

/** Called when the KIO job sends data */
void OscarFileSendConnection::slotKIOData(KIO::Job *job, const QByteArray &data)
{
	kdDebug(14150) << "[OscarFileSendConnection] got data " << ((KIO::SimpleJob *)(job))->url().fileName() << ", size " << data.size() << endl;
	mBytesTransferred += writeBlock(data.data(),data.size());
}

/** Called when the KIO slave wants data */
void OscarFileSendConnection::slotKIODataReq(KIO::Job *job, QByteArray &data)
{
  int len = mBuffer.getLength();
	data.assign(mBuffer.getBlock(len), len);
	kdDebug(14150) << "[OscarFileSendConnection] writing data " << ((KIO::SimpleJob *)(job))->url().fileName() << ", size " << data.size() << endl;
	if ( mSending )
		((KIO::TransferJob *)job)->suspend();
}

#include "oscarfilesendconnection.moc"
