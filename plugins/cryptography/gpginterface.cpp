#include "cryptographyplugin.h"  //(for the cached passphrase)
//Code from KGPG

/***************************************************************************
                          gpginterface.cpp  -  description
                             -------------------
    begin                : Mon Jul 8 2002
    copyright            : (C) 2002 by y0k0 <bj@altern.org>
                           (C) 2007 by Charles Connell <charles@connells.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <klocale.h>
#include <KDE/KPasswordDialog>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <qfile.h>
#include <QProcess>
//Added by qt3to4:
#include <QByteArray>

#include <k3procio.h>

//#include "kdetailedconsole.h"

#include "gpginterface.h"

QString GpgInterface::encryptText(QString text, QString userIDs, QString options)
{
	QString dests, encResult, gpgcmd;
	
	userIDs=userIDs.trimmed();
	userIDs=userIDs.simplified();
	options=options.trimmed();
	
	int ct=userIDs.indexOf(" ");
	while (ct!=-1)  // if multiple keys...
	{
		dests += " --recipient " + userIDs.section(' ',0,0);
		userIDs.remove ( 0, ct+1 );
		ct = userIDs.indexOf(" ");
	}
	dests += " --recipient " + userIDs;
	
	gpgcmd = "gpg --no-secmem-warning --no-tty ";
	gpgcmd += options.toLocal8Bit();
	gpgcmd += " -e ";
	gpgcmd += dests.toLocal8Bit();
		
	QProcess fp;
	fp.start(gpgcmd, QIODevice::ReadWrite);
	fp.waitForStarted();
	fp.write (text.toAscii());
	fp.closeWriteChannel();
	fp.waitForFinished();
	encResult = fp.readAll();
	
	return encResult;
}

QString GpgInterface::decryptText(QString text,QString userID)
{
	QString encResult, gpgcmd;
	
	int counter=0;
	QString password = CryptographyPlugin::cachedPass();
	bool passphraseHandling=CryptographyPlugin::passphraseHandling();
	
	// give them three tries on passphrase
	while ((counter<3) && (encResult.isEmpty()))
	{
		counter++;
		if(password.isNull())
		{
			userID.replace('<',"&lt;");
			QString passdlg=i18n("Enter passphrase for secret key %1:", userID);
			if (counter>1)
				passdlg.prepend(i18n("<b>Bad passphrase</b><br> You have %1 tries left.<br>", 4-counter));
	
			/// pipe for passphrase
			KPasswordDialog dlg( 0L, KPasswordDialog::NoFlags );
			dlg.setPrompt( passdlg );
			if( !dlg.exec() )
				return QString(); //the user canceled
			if (passphraseHandling)
				CryptographyPlugin::setCachedPass(dlg.password().toLocal8Bit());
			password = dlg.password();
		}
		gpgcmd += "gpg --no-secmem-warning --no-tty ";
		if(passphraseHandling)
			gpgcmd += "--passphrase " + password;
		gpgcmd += " -d ";
		
//		kDebug (14303) << k_funcinfo << "text is " << text << endl;
//		kDebug (14303) << k_funcinfo << "gmgcmd is " << gpgcmd << endl;
		QProcess fp;
		fp.start(gpgcmd, QIODevice::ReadWrite);
		fp.waitForStarted();
		fp.write (text.toAscii());
		fp.closeWriteChannel();
		fp.waitForFinished();
		encResult = fp.readAll();
		password.clear();
		gpgcmd.clear();
	}
	
	return encResult;
}

QString GpgInterface::checkForUtf8(QString txt)
{

        //    code borrowed from gpa
	const char *s;

        /* Make sure the encoding is UTF-8.
	* Test structure suggested by Werner Koch */
	if (txt.isEmpty())
		return QString::null;

	for (s = txt.ascii(); *s && !(*s & 0x80); s++)
		;
	if (*s && !strchr (txt.ascii(), 0xc3) && (txt.find("\\x")==-1))
		return txt;

	/* The string is not in UTF-8 */
        //if (strchr (txt.ascii(), 0xc3)) return (txt+" +++");
	if (txt.find("\\x")==-1)
		return QString::fromUtf8(txt.ascii());
        //        if (!strchr (txt.ascii(), 0xc3) || (txt.find("\\x")!=-1)) {
	for ( int idx = 0 ; (idx = txt.find( "\\x", idx )) >= 0 ; ++idx ) {
		char str[2] = "x";
		str[0] = (char) QString( txt.mid( idx + 2, 2 ) ).toShort( 0, 16 );
		txt.replace( idx, 4, str );
	}
	if (!strchr (txt.ascii(), 0xc3))
		return QString::fromUtf8(txt.ascii());
	else
		return QString::fromUtf8(QString::fromUtf8(txt.ascii()).ascii());  // perform Utf8 twice, or some keys display badly
}
#include "kgpginterface.moc"
