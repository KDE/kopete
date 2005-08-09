/*
    ksslsocket.cpp - KDE SSL Socket

    Copyright (c) 2005      by Tommi Rantala <tommi.rantala@cs.helsinki.fi>
    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qsocketnotifier.h>

#include <dcopclient.h>
#include <klocale.h>
#include <kdebug.h>
#include <kssl.h>
#include <ksslinfodlg.h>
#include <ksslpeerinfo.h>
#include <ksslcertchain.h>
#include <ksslcertificatecache.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include "ksslsocket.h"

struct KSSLSocketPrivate
{
	mutable KSSL *kssl;
	KSSLCertificateCache *cc;
	DCOPClient *dcc;
	QMap<QString,QString> metaData;
	QSocketNotifier *socketNotifier;
};

KSSLSocket::KSSLSocket() : KExtendedSocket()
{
	d = new KSSLSocketPrivate;
	d->kssl = 0;
	d->dcc = KApplication::kApplication()->dcopClient();
	d->cc = new KSSLCertificateCache;
	d->cc->reload();

	//No blocking
	setBlockingMode(false);

	//Connect internal slots
	QObject::connect( this, SIGNAL(connectionSuccess()), this, SLOT(slotConnected()) );
	QObject::connect( this, SIGNAL(closed(int)), this, SLOT(slotDisconnected()) );
	QObject::connect( this, SIGNAL(connectionFailed(int)), this, SLOT(slotDisconnected()));
}

KSSLSocket::~KSSLSocket()
{
	//Close connection
	closeNow();

	if( d->kssl )
	{
		d->kssl->close();
		delete d->kssl;
	}

	delete d->cc;

	delete d;
}

Q_LONG KSSLSocket::readBlock( char* data, Q_ULONG maxLen )
{
	//Re-implemented because KExtSocket doesn't use this when not in buffered mode
	Q_LONG retval = consumeReadBuffer(maxLen, data);

	if( retval == 0 )
	{
		if (sockfd == -1)
			return 0;

		retval = -1;
	}

	return retval;
}

int KSSLSocket::peekBlock( char* data, uint maxLen )
{
	//Re-implemented because KExtSocket doesn't use this when not in buffered mode
	if( socketStatus() < connected )
		return -2;

	if( sockfd == -1 )
		return -2;

	return consumeReadBuffer(maxLen, data, false);
}

Q_LONG KSSLSocket::writeBlock( const char* data, Q_ULONG len )
{
	return d->kssl->write( data, len );
}

int KSSLSocket::bytesAvailable() const
{
	if( socketStatus() < connected )
		return -2;

	//Re-implemented because KExtSocket doesn't use this when not in buffered mode
	return KBufferedIO::bytesAvailable();
}

void KSSLSocket::slotReadData()
{
	kdDebug(14120) << k_funcinfo << d->kssl->pending() << endl;
	QByteArray buff(512);
	int bytesRead = d->kssl->read( buff.data(), 512 );

	//Fill the read buffer
	feedReadBuffer( bytesRead, buff.data() );
	emit readyRead();
}

void KSSLSocket::slotConnected()
{
	if (!KSSL::doesSSLWork()) {
		kdError(14120) << k_funcinfo << "SSL not functional!" << endl;

		closeNow();
		emit sslFailure();
		return;
	}

	delete d->kssl;
	d->kssl = new KSSL();

	if (d->kssl->connect( sockfd ) != 1) {
		kdError(14120) << k_funcinfo << "SSL connect() failed." << endl;

		closeNow();
		emit sslFailure();
		return;
	}

	//Disconnect the KExtSocket notifier slot, we use our own
	QObject::disconnect( readNotifier(), SIGNAL(activated( int )),
			this, SLOT(socketActivityRead()) );

	QObject::connect( readNotifier(), SIGNAL(activated( int )),
			this, SLOT(slotReadData()) );

	readNotifier()->setEnabled(true);

	if (verifyCertificate() != 1) {
		closeNow();
		emit certificateRejected();
		return;
	}

	emit certificateAccepted();
}

void KSSLSocket::slotDisconnected()
{
	kdDebug(14120) << k_funcinfo << "Disconnected" << endl;

	if( readNotifier() )
		readNotifier()->setEnabled(false);

	delete d->kssl;
	d->kssl = 0L;
}

void KSSLSocket::showInfoDialog()
{
	if( socketStatus() == connected )
	{
		if (!d->dcc->isApplicationRegistered("kio_uiserver"))
		{
			KApplication::startServiceByDesktopPath("kio_uiserver.desktop",QStringList());
		}

		QByteArray data, ignore;
		QCString ignoretype;
		QDataStream arg(data, IO_WriteOnly);
		arg << "irc://" + peerAddress()->pretty() + ":" + port() << d->metaData;
		d->dcc->call("kio_uiserver", "UIServer",
			"showSSLInfoDialog(QString,KIO::MetaData)", data, ignoretype, ignore);
	}
}

void KSSLSocket::setMetaData( const QString &key, const QVariant &data )
{
	QVariant v = data;
	d->metaData[key] = v.asString();
}

bool KSSLSocket::hasMetaData( const QString &key )
{
	return d->metaData.contains(key);
}

QString KSSLSocket::metaData( const QString &key )
{
	if( d->metaData.contains(key) )
		return d->metaData[key];
	return QString::null;
}

/*
I basically copied the below from tcpKIO::SlaveBase.hpp, with some modificaions and formatting.

 * Copyright (C) 2000 Alex Zepeda <zipzippy@sonic.net
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
 * Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>
*/

int KSSLSocket::messageBox( KIO::SlaveBase::MessageBoxType type, const QString &text, const QString &caption,
	const QString &buttonYes, const QString &buttonNo )
{
	kdDebug(14120) << "messageBox " << type << " " << text << " - " << caption << buttonYes << buttonNo << endl;
	QByteArray data, result;
	QCString returnType;
	QDataStream arg(data, IO_WriteOnly);
	arg << (int)1 << (int)type << text << caption << buttonYes << buttonNo;

	if (!d->dcc->isApplicationRegistered("kio_uiserver"))
	{
		KApplication::startServiceByDesktopPath("kio_uiserver.desktop",QStringList());
	}

	d->dcc->call("kio_uiserver", "UIServer",
			"messageBox(int,int,QString,QString,QString,QString)", data, returnType, result);

	if( returnType == "int" )
	{
		int res;
		QDataStream r(result, IO_ReadOnly);
		r >> res;
		return res;
	}
	else
		return 0; // communication failure
}


//  Returns 0 for failed verification, -1 for rejected cert and 1 for ok
int KSSLSocket::verifyCertificate()
{
	int rc = 0;
	bool permacache = false;
	bool _IPmatchesCN = false;
	int result;
	bool doAddHost = false;
	QString ourHost = host();
	QString ourIp = peerAddress()->pretty();

	QString theurl = "irc://" + ourHost + ":" + port();

	if (!d->cc)
		d->cc = new KSSLCertificateCache;

	KSSLCertificate& pc = d->kssl->peerInfo().getPeerCertificate();

	KSSLCertificate::KSSLValidationList ksvl = pc.validateVerbose(KSSLCertificate::SSLServer);

	_IPmatchesCN = d->kssl->peerInfo().certMatchesAddress();

	if (!_IPmatchesCN)
	{
		ksvl << KSSLCertificate::InvalidHost;
	}

	KSSLCertificate::KSSLValidation ksv = KSSLCertificate::Ok;
	if (!ksvl.isEmpty())
		ksv = ksvl.first();

	/* Setting the various bits of meta-info that will be needed. */
	setMetaData("ssl_cipher", d->kssl->connectionInfo().getCipher());
	setMetaData("ssl_cipher_desc", d->kssl->connectionInfo().getCipherDescription());
	setMetaData("ssl_cipher_version", d->kssl->connectionInfo().getCipherVersion());
	setMetaData("ssl_cipher_used_bits", QString::number(d->kssl->connectionInfo().getCipherUsedBits()));
	setMetaData("ssl_cipher_bits", QString::number(d->kssl->connectionInfo().getCipherBits()));
	setMetaData("ssl_peer_ip", ourIp );

	QString errorStr;
	for(KSSLCertificate::KSSLValidationList::ConstIterator it = ksvl.begin();
		it != ksvl.end(); ++it)
	{
		errorStr += QString::number(*it)+":";
	}

	setMetaData("ssl_cert_errors", errorStr);
	setMetaData("ssl_peer_certificate", pc.toString());

	if (pc.chain().isValid() && pc.chain().depth() > 1)
	{
		QString theChain;
		QPtrList<KSSLCertificate> chain = pc.chain().getChain();
		for (KSSLCertificate *c = chain.first(); c; c = chain.next())
		{
			theChain += c->toString();
			theChain += "\n";
		}
		setMetaData("ssl_peer_chain", theChain);
	}
	else
	{
		setMetaData("ssl_peer_chain", "");
	}

	setMetaData("ssl_cert_state", QString::number(ksv));

	if (ksv == KSSLCertificate::Ok)
	{
		rc = 1;
		setMetaData("ssl_action", "accept");
	}

	// Since we're the parent, we need to teach the child.
	setMetaData("ssl_parent_ip", ourIp );
	setMetaData("ssl_parent_cert", pc.toString());

	//  - Read from cache and see if there is a policy for this
	KSSLCertificateCache::KSSLCertificatePolicy cp = d->cc->getPolicyByCertificate(pc);

	//  - validation code
	if (ksv != KSSLCertificate::Ok)
	{
		if( cp == KSSLCertificateCache::Unknown || cp == KSSLCertificateCache::Ambiguous)
		{
			cp = KSSLCertificateCache::Prompt;
		}
		else
		{
			// A policy was already set so let's honor that.
			permacache = d->cc->isPermanent(pc);
		}

		if (!_IPmatchesCN && cp == KSSLCertificateCache::Accept)
		{
			cp = KSSLCertificateCache::Prompt;
		}

		// Precondition: cp is one of Reject, Accept or Prompt
		switch (cp)
		{
			case KSSLCertificateCache::Accept:
				rc = 1;
				break;

			case KSSLCertificateCache::Reject:
				rc = -1;
				break;

			case KSSLCertificateCache::Prompt:
			{
				do
				{
					if (ksv == KSSLCertificate::InvalidHost)
					{
						QString msg = i18n("The IP address of the host %1 "
								"does not match the one the "
								"certificate was issued to.");
						result = messageBox( KIO::SlaveBase::WarningYesNoCancel,
						msg.arg(ourHost),
						i18n("Server Authentication"),
						i18n("&Details"),
						i18n("Co&ntinue") );
					}
					else
					{
						QString msg = i18n("The server certificate failed the "
							"authenticity test (%1).");
						result = messageBox( KIO::SlaveBase::WarningYesNoCancel,
						msg.arg(ourHost),
						i18n("Server Authentication"),
						i18n("&Details"),
						i18n("Co&ntinue") );
					}

					if (result == KMessageBox::Yes)
					{
						showInfoDialog();
					}
				}
				while (result == KMessageBox::Yes);

				if (result == KMessageBox::No)
				{
					rc = 1;
					cp = KSSLCertificateCache::Accept;
					doAddHost = true;
					result = messageBox( KIO::SlaveBase::WarningYesNo,
							i18n("Would you like to accept this "
							"certificate forever without "
							"being prompted?"),
							i18n("Server Authentication"),
								i18n("&Forever"),
								i18n("&Current Sessions Only"));
					if (result == KMessageBox::Yes)
						permacache = true;
					else
						permacache = false;
				}
				else
				{
					rc = -1;
					cp = KSSLCertificateCache::Prompt;
				}

				break;
		}
		default:
		kdDebug(14120) << "SSL error in cert code."
				<< "Please report this to kopete-devel@kde.org."
				<< endl;
		break;
		}
	}

	//  - cache the results
	d->cc->addCertificate(pc, cp, permacache);
	if (doAddHost)
		d->cc->addHost(pc, ourHost);


	if (rc == -1)
		return rc;


	kdDebug(14120) << "SSL connection information follows:" << endl
		<< "+-----------------------------------------------" << endl
		<< "| Cipher: " << d->kssl->connectionInfo().getCipher() << endl
		<< "| Description: " << d->kssl->connectionInfo().getCipherDescription() << endl
		<< "| Version: " << d->kssl->connectionInfo().getCipherVersion() << endl
		<< "| Strength: " << d->kssl->connectionInfo().getCipherUsedBits()
		<< " of " << d->kssl->connectionInfo().getCipherBits()
		<< " bits used." << endl
		<< "| PEER:" << endl
		<< "| Subject: " << d->kssl->peerInfo().getPeerCertificate().getSubject() << endl
		<< "| Issuer: " << d->kssl->peerInfo().getPeerCertificate().getIssuer() << endl
		<< "| Validation: " << (int)ksv << endl
		<< "| Certificate matches IP: " << _IPmatchesCN << endl
		<< "+-----------------------------------------------"
		<< endl;

	// sendMetaData();  Do not call this function!!
	return rc;
}


#include "ksslsocket.moc"
