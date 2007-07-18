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
#include <KPasswordDialog>

#include <QProcess>

#include <ktemporaryfile.h>
#include "gpginterface.h"

#include "kopeteuiglobal.h"

QString GpgInterface::encryptText ( QString text, QString userIDs, QString options, bool signAlso, QString privateKey )
{
	int counter = 0;
	QString dests, encResult, gpgcmd, password;

	userIDs=userIDs.trimmed();
	userIDs=userIDs.simplified();
	options=options.trimmed();

	int ct=userIDs.indexOf ( " " );
	while ( ct!=-1 )  // if multiple keys...
	{
		dests += " --recipient " + userIDs.section ( ' ',0,0 );
		userIDs.remove ( 0, ct+1 );
		ct = userIDs.indexOf ( " " );
	}
	dests += " --recipient " + userIDs;

	if ( signAlso )
	{
		while ( ( counter < 3 ) && ( encResult.isEmpty() ) )
		{
			counter++;
			password = getPassword ( password, privateKey, counter );
			gpgcmd = "gpg --no-secmem-warning --no-tty " + options + " -e " + dests;
			gpgcmd += " --passphrase " + password + " -s ";

			QProcess fp;
			fp.start ( gpgcmd, QIODevice::ReadWrite );
			fp.waitForStarted();
			fp.write ( text.toAscii() );
			fp.closeWriteChannel();
			fp.waitForFinished();
			encResult = fp.readAll();
			password.clear();
			gpgcmd.clear();;
		}
	}
	else
	{
		gpgcmd = "gpg --no-secmem-warning --no-tty " + options + " -e " + dests;
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
	int counter = 0;
	QString password = CryptographyPlugin::cachedPass();
	while ( ( counter<3 ) && ( encResult.isEmpty() ) )
	{
		counter++;
		password = getPassword ( password, privateKey, counter );

		QString gpgcmd = "gpg --no-secmem-warning --no-tty --armor --passphrase " + password + " -s ";
		QProcess fp;
		fp.start ( gpgcmd, QIODevice::ReadWrite );
		fp.waitForStarted();
		fp.write ( text.toAscii() );
		fp.closeWriteChannel();
		fp.waitForFinished();
		encResult = fp.readAll();
		password.clear();
		gpgcmd.clear();
	}
	return encResult;
}

QString GpgInterface::decryptText ( QString text, QString userID, int &opState )
{
	QString encResult, gpgcmd, status;

	int counter = 0;
	QString password = CryptographyPlugin::cachedPass();

	// give them three tries on passphrase
	while ( ( counter<3 ) && ( encResult.isEmpty() ) )
	{
		KTemporaryFile tempfile;

		counter++;
		password = getPassword ( password, userID, counter );
		tempfile.open();
		gpgcmd += "gpg --no-secmem-warning --no-tty ";
		gpgcmd += " --status-file "  + tempfile.fileName();
		gpgcmd += " --passphrase " + password;
		gpgcmd += " -d ";

		QProcess fp;
		fp.start ( gpgcmd, QIODevice::ReadWrite );
		fp.waitForStarted();
		fp.write ( text.toAscii() );
		fp.closeWriteChannel();
		fp.waitForFinished();
		encResult = fp.readAll();

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
		password.clear();
		gpgcmd.clear();
	}
	return encResult;
}

QString GpgInterface::checkForUtf8 ( QString txt )
{

	//    code borrowed from gpa
	const char *s;

	/* Make sure the encoding is UTF-8.
	* Test structure suggested by Werner Koch */
	if ( txt.isEmpty() )
		return QString::null;

	for ( s = txt.ascii(); *s && ! ( *s & 0x80 ); s++ )
		;
	if ( *s && !strchr ( txt.ascii(), 0xc3 ) && ( txt.find ( "\\x" ) ==-1 ) )
		return txt;

	/* The string is not in UTF-8 */
	//if (strchr (txt.ascii(), 0xc3)) return (txt+" +++");
	if ( txt.find ( "\\x" ) ==-1 )
		return QString::fromUtf8 ( txt.ascii() );
	//        if (!strchr (txt.ascii(), 0xc3) || (txt.find("\\x")!=-1)) {
	for ( int idx = 0 ; ( idx = txt.find ( "\\x", idx ) ) >= 0 ; ++idx )
	{
		char str[2] = "x";
		str[0] = ( char ) QString ( txt.mid ( idx + 2, 2 ) ).toShort ( 0, 16 );
		txt.replace ( idx, 4, str );
	}
	if ( !strchr ( txt.ascii(), 0xc3 ) )
		return QString::fromUtf8 ( txt.ascii() );
	else
		return QString::fromUtf8 ( QString::fromUtf8 ( txt.ascii() ).ascii() );  // perform Utf8 twice, or some keys display badly
}

class CryptographyPasswordDialog : public KPasswordDialog
{
		Q_OBJECT
	public:
		CryptographyPasswordDialog ( QWidget *parent=0L, const KPasswordDialogFlags &flags=0, const KDialog::ButtonCodes otherButtons=0 ) : KPasswordDialog ( parent, flags, otherButtons ) {}
};

QString GpgInterface::getPassword ( QString password, QString userID, int counter )
{
	if ( !password.isEmpty() && counter <= 1 )
		return password;
	
	QString passdlg=i18n ( "Enter passphrase for secret key %1:\nYou have %2 tries left.", "0x" + userID.right ( 8 ), 4 - counter );
	CryptographyPasswordDialog dlg ( Kopete::UI::Global::mainWidget(), KPasswordDialog::NoFlags );
	dlg.setPrompt ( passdlg );
	if ( !dlg.exec() )
		return QString(); //the user canceled
	CryptographyPlugin::setCachedPass ( dlg.password() );
	
	// if there is already a password dialog open, get password and send it to that
	QList<CryptographyPasswordDialog*> otherDialogs = Kopete::UI::Global::mainWidget()->findChildren <CryptographyPasswordDialog *> ();
	if ( otherDialogs.size() > 1){
		foreach ( CryptographyPasswordDialog *otherDialog, otherDialogs ){
			otherDialog->setPassword ( dlg.password());
			otherDialog->accept();
		}
	}
	return dlg.password();
}

#include "gpginterface.moc"
