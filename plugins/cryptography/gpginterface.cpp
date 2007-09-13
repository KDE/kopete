/*
    cryptographyselectuserkey.h  -  description

    Copyright (c) 2002      by y0k0            <bj@altern.org> (as part of KGPG)
    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/
#include <klocale.h>
#include <KMessageBox>

#include <QProcess>

#include <ktemporaryfile.h>
#include "gpginterface.h"

#include <kopete/kopeteuiglobal.h>

QString GpgInterface::encryptText ( QString text, QString userIDs, bool signAlso, QString privateKey )
{
	QString dests, encResult, gpgcmd;

	userIDs = userIDs.trimmed();
	userIDs = userIDs.simplified();

	int ct = userIDs.indexOf ( " " );
	while ( ct!=-1 )  // if multiple keys...
	{
		dests += " --recipient " + userIDs.section ( ' ',0,0 );
		userIDs.remove ( 0, ct+1 );
		ct = userIDs.indexOf ( " " );
	}
	dests += " --recipient " + userIDs;

	// sign and encrypt
	if ( signAlso )
	{
		// prepare the gpg command
		gpgcmd = "gpg --no-secmem-warning --no-tty --trust-model always --armor -e --local-user " + privateKey + " " + dests + " -s ";

		// this is syncronous process IO.
		QProcess fp;
		fp.start ( gpgcmd, QIODevice::ReadWrite );
		fp.waitForStarted();
		fp.write ( text.toAscii() );
		fp.closeWriteChannel();
		fp.waitForFinished();
		encResult = fp.readAll();
		gpgcmd.clear();;
	}
	// just encrypt
	else
	{
		gpgcmd = "gpg --no-secmem-warning --no-tty --trust-model always --armor -e " + dests;
		QProcess fp;
		fp.start ( gpgcmd, QIODevice::ReadWrite );
		fp.waitForStarted();
		fp.write ( text.toAscii() );
		fp.closeWriteChannel();
		fp.waitForFinished();
		encResult = fp.readAll();
	}
	return encResult;
}

QString GpgInterface::signText ( QString text, QString privateKey )
{
	QString gpgcmd, encResult;

	gpgcmd = "gpg --no-secmem-warning --no-tty --trust-model always --armor -s --local-user " + privateKey;
	// syncronous IO
	QProcess fp;
	fp.start ( gpgcmd, QIODevice::ReadWrite );
	fp.waitForStarted();
	fp.write ( text.toAscii() );
	fp.closeWriteChannel();
	fp.waitForFinished();
	encResult = fp.readAll();
	gpgcmd.clear();

	return encResult;
}

QString GpgInterface::decryptText ( QString text, int &opState )
{
	QString encResult, gpgcmd, status;

	KTemporaryFile tempfile;

	tempfile.open();
	gpgcmd += "gpg --no-secmem-warning --no-tty --status-file " + tempfile.fileName() + " -d";

	// syncronous IO
	QProcess fp;
	fp.start ( gpgcmd, QIODevice::ReadWrite );
	fp.waitForStarted();
	fp.write ( text.toAscii() );
	fp.closeWriteChannel();
	fp.waitForFinished();
	encResult = fp.readAll();

	// set signature sttate
	status = tempfile.readAll();
	if ( status.contains ( "GOODSIG" ) )
		opState = GoodSig;
	else if ( status.contains ( "BADSIG" ) )
		opState = BadSig;
	else if ( status.contains ( "ERRSIG" ) )
		opState = ErrorSig;
	else
		opState = NoSig;

	if ( status.contains ( "DECRYPTION_OKAY" ) )
		opState = ( opState | Decrypted );

	status.clear();
	gpgcmd.clear();

	return encResult;
}

#include "gpginterface.moc"
