/*
    ksslsocket.cpp - KDE SSL Socket

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <sys/select.h>

#include <qmutex.h>
#include <qthread.h>
#include <qtimer.h>

#include <dcopclient.h>
#include <klocale.h>
#include <kdebug.h>
#include <kssl.h>
#include <ksslinfodlg.h>
#include <ksslpeerinfo.h>
#include <ksslcertchain.h>
#include <ksslcertificatecache.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include "ksslsocket.h"

struct KSSLSocketPrivate
{
	mutable KSSL *kssl;
	QMutex mutex;
	SSLPollThread *thread;
	int pendingBytes;
	KSSLCertificateCache *cc;
	DCOPClient *dcc;
	bool militantSSL;
	QMap<QString,QString> metaData;
};

//Thread to poll the SSL socket
class SSLPollThread : public QThread
{
	public:
		SSLPollThread( KSSLSocket *s ) : QThread(), parent(s), exiting(false) {};

		void quit()
		{
			//Exit the thread
			exiting = true;
		}

	protected:
		void run()
		{
			while( !exiting )
			{
				/*
				 * Pending only returns the data that is in the internal
				 * buffer, that hasn't been picked up by read() yet. It
				 * doesn't check the socket. So, we need to peek the socket.
				 *
				 * peek blocks, which is why this reader thread exists.
				 */
				char buff[64];

				//Lock the mutex, so we will wait until the parent is done
				//reading any previously registered data
				parent->d->mutex.lock();

				//Register any pending bufer data first, then peek if none
				int bytes = parent->d->kssl->pending();
				if( bytes == 0 )
					bytes = parent->d->kssl->peek( buff, 64 );

				if( bytes )
				{
					//Unlock the mutex and register this data with the parent
					parent->d->mutex.unlock();
					parent->readData( bytes );
				}
			}
		}

	private:
		KSSLSocket *parent;
		bool exiting;

};

KSSLSocket::KSSLSocket() : KExtendedSocket()
{
	d = new KSSLSocketPrivate;
	d->kssl = 0L;
	d->pendingBytes = 0;
	d->thread = new SSLPollThread(this);
	d->dcc = KApplication::kApplication()->dcopClient();
	d->cc = new KSSLCertificateCache;
	d->cc->reload();
	d->militantSSL = false;

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

	//Get rid of reader thread
	d->thread->wait();
	delete d->thread;

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
	//Re-implemented because KExtSocket doesn't use this when not in buffered mode
	return KBufferedIO::bytesAvailable();
}

void KSSLSocket::readData( int bytes )
{
	//Lock the mutex, so the reader thread will block until we read this data
	d->mutex.lock();
	d->pendingBytes = bytes;
}

void KSSLSocket::slotPoll()
{
	//Check for registered data
	if( d->pendingBytes )
	{
		//Read in the data sent by the reader thread
		QByteArray buff(d->pendingBytes);
		int bytesRead = d->kssl->read( buff.data(), d->pendingBytes );

		//Fill the read buffer
		feedReadBuffer( bytesRead, buff.data() );
		d->pendingBytes = 0;
		emit readyRead();

		//Unlock the reader thread so it can now check for new data
		d->mutex.unlock();
	}

	if( socketStatus() == connected )
		QTimer::singleShot( 0, this, SLOT( slotPoll() ) );
}

void KSSLSocket::slotConnected()
{
	if( KSSL::doesSSLWork() )
	{
		kdDebug(14120) << k_funcinfo << "Trying SSL connection..." << endl;
		if( !d->kssl )
		{
			d->kssl = new KSSL();
			d->kssl->connect( fd() );
		}
		else
		{
			d->kssl->reInitialize();
		}

		if( verifyCertificate() == 1 )
		{
			//Start polling for data
			d->thread->start();
			slotPoll();
		}
		else
		{
			closeNow();
		}

	}
	else
	{
		kdError(14120) << k_funcinfo << "SSL not functional!" << endl;

		d->kssl = 0L;
		emit sslFailure();
		closeNow();
	}
}

void KSSLSocket::slotDisconnected()
{
	d->thread->quit();
	d->thread->wait();
}

void KSSLSocket::showInfoDialog( QWidget *parent, bool modal )
{
	if( socketStatus() == connected )
	{
		KSSLPeerInfo &peer = d->kssl->peerInfo();
		KSSLConnectionInfo &conn = d->kssl->connectionInfo();
		KSSLInfoDlg *dialog = new KSSLInfoDlg(peer.getPeerCertificate().isValid(), parent,  "", modal );
		dialog->setup( &peer.getPeerCertificate(), host(), QString::null, conn.getCipher(),
			conn.getCipherDescription(), conn.getCipherVersion(), conn.getCipherUsedBits(),
			conn.getCipherBits(), peer.getPeerCertificate().validate() );
		dialog->show();
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
I basically copied the below from tcpslavebase.hpp, with some modificaions and formatting.

 * Copyright (C) 2000 Alex Zepeda <zipzippy@sonic.net
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
 * Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>
*/

//  Returns 0 for failed verification, -1 for rejected cert and 1 for ok
int KSSLSocket::verifyCertificate()
{
	int rc = 0;
	bool permacache = false;
	bool isChild = false;
	bool _IPmatchesCN = false;
	int result;
	bool doAddHost = false;
	QString ourHost = host();
	QString ourIp = peerAddress()->pretty();

	QString theurl = "irc://" + ourHost + ":" + port();

	if (!hasMetaData("ssl_militant") || metaData("ssl_militant") == "FALSE")
		d->militantSSL = false;
	else if (metaData("ssl_militant") == "TRUE")
		d->militantSSL = true;

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

	kdDebug(14120) << "SSL HTTP frame the parent? " << metaData("main_frame_request") << endl;
	if (!hasMetaData("main_frame_request") || metaData("main_frame_request") == "TRUE")
	{
		// Since we're the parent, we need to teach the child.
		setMetaData("ssl_parent_ip", ourIp );
		setMetaData("ssl_parent_cert", pc.toString());

		//  - Read from cache and see if there is a policy for this
		KSSLCertificateCache::KSSLCertificatePolicy cp = d->cc->getPolicyByCertificate(pc);

		//  - validation code
		if (ksv != KSSLCertificate::Ok)
		{
			if (d->militantSSL)
				return -1;

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
					setMetaData("ssl_action", "accept");
					break;

				case KSSLCertificateCache::Reject:
					rc = -1;
					setMetaData("ssl_action", "reject");
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
							result = KMessageBox::messageBox( 0L, KMessageBox::WarningYesNoCancel,
							msg.arg(ourHost),
							i18n("Server Authentication"),
							i18n("&Details"),
							i18n("Co&ntinue") );
						}
						else
						{
							QString msg = i18n("The server certificate failed the "
								"authenticity test (%1).");
							result = KMessageBox::messageBox( 0L, KMessageBox::WarningYesNoCancel,
							msg.arg(ourHost),
							i18n("Server Authentication"),
							i18n("&Details"),
							i18n("Co&ntinue") );
						}

						if (result == KMessageBox::Yes)
						{
							if (!d->dcc->isApplicationRegistered("kio_uiserver"))
							{
								KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
								QStringList() );
							}

							QByteArray data, ignore;
							QCString ignoretype;
							QDataStream arg(data, IO_WriteOnly);
							arg << theurl << d->metaData;
								d->dcc->call("kio_uiserver", "UIServer",
									"showSSLInfoDialog(QString,KIO::MetaData)",
									data, ignoretype, ignore);
						}
					}
					while (result == KMessageBox::Yes);

					if (result == KMessageBox::No)
					{
						setMetaData("ssl_action", "accept");
						rc = 1;
						cp = KSSLCertificateCache::Accept;
						doAddHost = true;
						result = KMessageBox::messageBox( 0L, KMessageBox::WarningYesNo,
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
						setMetaData("ssl_action", "reject");
						rc = -1;
						cp = KSSLCertificateCache::Prompt;
					}

					break;
			}
			default:
			kdDebug(14120) << "TCPSlaveBase/SSL error in cert code."
					<< "Please report this to kfm-devel@kde.org."
					<< endl;
			break;
			}
		}

		//  - cache the results
		d->cc->addCertificate(pc, cp, permacache);
		if (doAddHost)
			d->cc->addHost(pc, ourHost);
	}
	else
	{    // Child frame
		//  - Read from cache and see if there is a policy for this
		KSSLCertificateCache::KSSLCertificatePolicy cp = d->cc->getPolicyByCertificate(pc);
		isChild = true;

		// Check the cert and IP to make sure they're the same
		// as the parent frame
		bool certAndIPTheSame = (ourIp == metaData("ssl_parent_ip") &&
					pc.toString() == metaData("ssl_parent_cert"));

		if (ksv == KSSLCertificate::Ok)
		{
			if (certAndIPTheSame)
			{       // success
				rc = 1;
				setMetaData("ssl_action", "accept");
			}
			else
			{
				setMetaData("ssl_action", "accept");
				rc = 1; // Let's accept this now.  It's bad, but at least the user
					// will see potential attacks in KDE3 with the pseudo-lock
					// icon on the toolbar, and can investigate with the RMB
			}
		}
		else
		{
			if (d->militantSSL)
				return -1;

			if (cp == KSSLCertificateCache::Accept)
			{
				if (certAndIPTheSame)
				{	// success
					rc = 1;
					setMetaData("ssl_action", "accept");
				}
				else
				{	// fail
					result = KMessageBox::messageBox( 0L,KMessageBox::WarningYesNo,
								i18n("You have indicated that you wish to accept this certificate, but it is not issued to the server who is presenting it. Do you wish to continue loading?"),
								i18n("Server Authentication"));
					if (result == KMessageBox::Yes)
					{
						rc = 1;
						setMetaData("ssl_action", "accept");
						d->cc->addHost(pc, ourHost);
					}
					else
					{
						rc = -1;
						setMetaData("ssl_action", "reject");
					}
				}
			}
			else if (cp == KSSLCertificateCache::Reject)
			{      // fail
				KMessageBox::messageBox( 0L, KMessageBox::Information, i18n("SSL certificate is being rejected as requested. You can disable this in the KDE Control Center."),
							i18n("Server Authentication"));
				rc = -1;
				setMetaData("ssl_action", "reject");
			}
			else
			{
				do
				{
					QString msg = i18n("The server certificate failed the "
							"authenticity test (%1).");
					result = KMessageBox::messageBox( 0L,KMessageBox::WarningYesNoCancel,
								msg.arg(ourHost),
								i18n("Server Authentication"),
								i18n("&Details"),
								i18n("Co&ntinue"));
					if (result == KMessageBox::Yes)
					{
						if (!d->dcc->isApplicationRegistered("kio_uiserver"))
						{
							KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
							QStringList() );
						}

						QByteArray data, ignore;
						QCString ignoretype;
						QDataStream arg(data, IO_WriteOnly);
						arg << theurl << d->metaData;
						d->dcc->call("kio_uiserver", "UIServer",
							"showSSLInfoDialog(QString,KIO::MetaData)",
							data, ignoretype, ignore);
					}
				} while (result == KMessageBox::Yes);

				if (result == KMessageBox::No)
				{
					setMetaData("ssl_action", "accept");
					rc = 1;
					cp = KSSLCertificateCache::Accept;
					result = KMessageBox::messageBox( 0L,KMessageBox::WarningYesNo,
								i18n("Would you like to accept this "
								"certificate forever without "
								"being prompted?"),
								i18n("Server Authentication"),
								i18n("&Forever"),
								i18n("&Current Sessions Only"));
					permacache = (result == KMessageBox::Yes);
					d->cc->addCertificate(pc, cp, permacache);
					d->cc->addHost(pc, ourHost);
				}
				else
				{
					setMetaData("ssl_action", "reject");
					rc = -1;
					cp = KSSLCertificateCache::Prompt;
					d->cc->addCertificate(pc, cp, permacache);
				}
			}
		}
	}

	if (rc == -1)
		return rc;

	if (metaData("ssl_activate_warnings") == "TRUE")
	{
		//  - entering SSL
		if (!isChild && metaData("ssl_was_in_use") == "FALSE" && d->kssl->settings()->warnOnEnter())
		{
			int result;
			do
			{
				result = KMessageBox::messageBox( 0L, KMessageBox::WarningYesNo, i18n("You are about to "
									"enter secure mode. "
									"All transmissions "
									"will be encrypted "
									"unless otherwise "
									"noted.\nThis means "
									"that no third party "
									"will be able to "
									"easily observe your "
									"data in transit."),
									i18n("Security Information"),
									i18n("Display SSL "
									"Information"),
									i18n("Continue") );
				if ( result == KMessageBox::Yes )
				{
					if (!d->dcc->isApplicationRegistered("kio_uiserver"))
					{
						KApplication::startServiceByDesktopPath("kio_uiserver.desktop",
						QStringList() );
					}

					QByteArray data, ignore;
					QCString ignoretype;
					QDataStream arg(data, IO_WriteOnly);
					arg << theurl << d->metaData;
					d->dcc->call("kio_uiserver", "UIServer",
						"showSSLInfoDialog(QString,KIO::MetaData)",
						data, ignoretype, ignore);
				}
			}
			while (result != KMessageBox::No);
		}
	}   // if ssl_activate_warnings

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
