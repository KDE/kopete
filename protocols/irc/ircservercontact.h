/*
    ircservercontact.h - IRC User Contact

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCSERVERCONTACT_H
#define IRCSERVERCONTACT_H

#include <qvaluelist.h>
#include <qstringlist.h>

#include "kopetemessagemanagerfactory.h"
#include "irccontact.h"

class KActionCollection;
class KAction;
class KActionMenu;
class KopeteView;

class IRCContactManager;
class IRCChannelContact;

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 *
 * This class is the @ref KopeteContact object representing IRC Servers.
 * It is derrived from @ref IRCContact where much of its functionality is shared with @ref IRCChannelContact and @ref IRCUserContact.
 */
class IRCServerContact : public IRCContact
{
	Q_OBJECT

	public:
		// This class provides a KopeteContact for each server of a given IRC connection.
		IRCServerContact(IRCContactManager *, const QString &servername, KopeteMetaContact *mc);

		virtual const QString caption() const;

		virtual void appendMessage(KopeteMessage &);
                void appendMessage( const QString &message );

	protected slots:
		void engineInternalError(KIRC::EngineError error, const KIRCMessage &ircmsg);
		virtual void slotSendMsg(KopeteMessage &message, KopeteMessageManager *);

	private slots:
		virtual void updateStatus();
		void slotViewCreated( KopeteView* );
		void slotDumpMessages();

		void slotIncomingUnknown( const QString &message );
		void slotIncomingConnect( const QString &message );
		void slotIncomingMotd( const QStringList &motd );
		void slotIncomingNotice( const QString &orig, const QString &notice );
		void slotCannotSendToChannel( const QString &channel, const QString &msg );

	private:
		QValueList<KopeteMessage> mMsgBuffer;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:

