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
#include "cryptographyplugin.h"  //(for the cached passphrase)

#include <klocale.h>
#include <KPasswordDialog>
#include <KMessageBox>

#include <QProcess>

#include <ktemporaryfile.h>
#include "gpginterface.h"

#include <kopete/kopeteuiglobal.h>

QString GpgInterface::encryptText ( QString text, QString userIDs, bool signAlso, QString privateKey )
{
	int counter = 0;
	QString dests, encResult, gpgcmd, passphrase;

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
		// password challenge loop
		while ( ( counter < 3 ) && ( encResult.isEmpty() ) )
		{
			counter++;
			passphrase = getPassword ( passphrase, privateKey, counter );
			// prepare the gpg command
			gpgcmd = "gpg --no-secmem-warning --no-tty --trust-model always --armor -e --local-user " + privateKey + " " + dests + " -s --passphrase " + passphrase;

			// this is syncronous process IO.
			QProcess fp;
			fp.start ( gpgcmd, QIODevice::ReadWrite );
			fp.waitForStarted();
			fp.write ( text.toAscii() );
			fp.closeWriteChannel();
			fp.waitForFinished();
			encResult = fp.readAll();
			passphrase.clear();
			gpgcmd.clear();;
		}
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
	int counter = 0;
	QString passphrase = CryptographyPlugin::cachedPass();
	// password challenge loop
	while ( ( counter<3 ) && ( encResult.isEmpty() ) )
	{
		counter++;
		passphrase = getPassword ( passphrase, privateKey, counter );

		QString gpgcmd = "gpg --no-secmem-warning --no-tty --trust-model always --armor -s --local-user " + privateKey + "  --passphrase " + passphrase;
		// syncronous IO
		QProcess fp;
		fp.start ( gpgcmd, QIODevice::ReadWrite );
		fp.waitForStarted();
		fp.write ( text.toAscii() );
		fp.closeWriteChannel();
		fp.waitForFinished();
		encResult = fp.readAll();
		passphrase.clear();
		gpgcmd.clear();
	}
	return encResult;
}

QString GpgInterface::decryptText ( QString text, QString userID, int &opState )
{
	QString encResult, gpgcmd, status;

	int counter = 0;
	QString passphrase = CryptographyPlugin::cachedPass();

	// give them three tries on passphrase
	while ( ( counter<3 ) && ( encResult.isEmpty() ) )
	{
		KTemporaryFile tempfile;

		counter++;
		passphrase = getPassword ( passphrase, userID, counter );
		
		tempfile.open();
		gpgcmd += "gpg --no-secmem-warning --no-tty --status-file " + tempfile.fileName() + " -d  --passphrase " + passphrase;

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
		passphrase.clear();
		gpgcmd.clear();
	}
	return encResult;
}

// we have a special class so we know can identify it by classname if it's being displayed
class CryptographyPasswordDialog : public KPasswordDialog
{
		Q_OBJECT
	public:
		CryptographyPasswordDialog ( QWidget *parent=0L, const KPasswordDialogFlags &flags=0, const KDialog::ButtonCodes otherButtons=0 ) : KPasswordDialog ( parent, flags, otherButtons ) {}
};

QString GpgInterface::getPassword ( QString passphrase, QString userID, int counter )
{
	if ( !passphrase.isEmpty() && counter <= 1 )
		return passphrase;
	
	QString passdlg=i18n ( "Enter passphrase for secret key %1:\nYou have %2 tries left.", "0x" + userID.right ( 8 ), 4 - counter );
	CryptographyPasswordDialog dlg ( Kopete::UI::Global::mainWidget(), KPasswordDialog::NoFlags );
	dlg.setPrompt ( passdlg );
	if ( !dlg.exec() )
		return QString(); //the user canceled
	CryptographyPlugin::setCachedPass ( dlg.password() );
	
	// if there is already a passphrase dialog open, get passphrase from user and send it to that
	// identify by classname. I know it's a hack, if you have something better contact Charles Connell
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
