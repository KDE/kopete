/*
    cryptographyplugin.h  -  description

    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

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

#ifndef CRYPTOGRAPHYPLUGIN_H
#define CRYPTOGRAPHYPLUGIN_H

#include "kopeteplugin.h"

#include <QVariantList>
#include <QHash>

class QString;

namespace GpgME
{
	class DecryptionResult;
	class VerificationResult;
}

namespace Kleo { class Job; }


namespace Kopete
{
	class Message;
	class MessageEvent;
	class ChatSession;
}

class CryptographyMessageHandlerFactory;

/**
  * @author Olivier Goffart
  * @author Charles Connell
  * Main plugin class, handles messages. Also has static functions used by rest of plugin
  *
  * Basic architecture:
  *
  * Outgoing messages are routed through slotOutgoingMessage().
  * That uses Kleo::SomethingJob->exec() to do the actual crypto work
  *
  * Incoming messages go through slotIncomingMessage(). This starts
  * a crypto job to deal with the message, remembers which job goes with
  * which message (using mCurrentJobs), and then discards it.
  * When the job is done, it activates slotIncomingMessageContinued().
  * Since slotIncomingMessageContinued was called to decrypt and verify the PGP block.
  * it will only give back good data if the block was actually encrypted
  * and signed. If the data is good, it will lookup the message that it was processing,
  * and it will set the body to the plaintext the crypto job returned. If the job's
  * returned data is not good, its plaintext result will be right, but the crypto
  * meta-data will be invalid. So, if slotIncomingMessageContinued sees an
  * empty list of signers, it will assume that the PGP block was not encrypted
  * *and* signed, but may be one or the other. To check for each of those
  * possibilies,slotIncomingMessageContinued then launches two new jobs,
  * one to check if the message was only signed, and the other to check if
  * the message was only encrypted. If either of these find their case to be true
  * by looking at heuristics in the info that the job returns, they will take the
  * message from mCurrentJobs and put it into circulation. Whenever we detect that
  * a message has had crypto applied to it, we add icons to the message to
  * convey what we know to the user through the look of the message itself.
  *
  */

class CryptographyPlugin : public Kopete::Plugin
{
		Q_OBJECT

	public:
		static CryptographyPlugin  *plugin();

		static QStringList supportedProtocols() {
			return QStringList() << "MSNProtocol"
			       << "MessengerProtocol"
			       << "WLMProtocol"
			       << "JabberProtocol"
			       << "SkypeProtocol"
			       << "BonjourProtocol"
			       << "WPProtocol"
			       << "IRCProtocol"
			       << "YahooProtocol";
		}
		
		static QStringList getKabcKeys ( QString uid );
		
		static QString kabcKeySelector ( QString displayName, QString addresseeName, QStringList keys, QWidget *parent );

		CryptographyPlugin ( QObject *parent, const QVariantList &args );
		~CryptographyPlugin();

	private slots:
		void slotIncomingMessage ( Kopete::MessageEvent *msg );
		
		void slotIncomingMessageContinued ( const GpgME::DecryptionResult &decryptionResult, const GpgME::VerificationResult &verificationResult, const QByteArray &plainText );
		
		void slotIncomingEncryptedMessageContinued ( const GpgME::DecryptionResult &decryptionResult, const QByteArray &plainText );
		
		void slotIncomingSignedMessageContinued ( const GpgME::VerificationResult &verificationResult, const QByteArray &plainText );
		
		void finalizeMessage ( Kopete::Message & msg, const QString &intendedBody, const GpgME::VerificationResult & validity, bool encrypted );

		void slotOutgoingMessage ( Kopete::Message& msg );
		
		void slotExportSelectedMetaContactKeys ();
		
		void slotSelectContactKey();
		
		void slotNewKMM ( Kopete::ChatSession * );

	private:
		static CryptographyPlugin* mPluginStatic;
		CryptographyMessageHandlerFactory *mInboundHandler;
		QHash<Kleo::Job*, Kopete::Message> mCurrentJobs;
};

#endif

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

