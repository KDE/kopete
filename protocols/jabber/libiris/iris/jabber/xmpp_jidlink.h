/*
 * jidlink.h - establish a link between Jabber IDs
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
   NOTE: this is not to be confused with JEP-0041
*/

#ifndef JABBER_JIDLINK_H
#define JABBER_JIDLINK_H

#include<qobject.h>
#include<qstring.h>
#include"xmpp.h"

class ByteStream;

namespace XMPP
{
	class Client;

	class JidLink : public QObject
	{
		Q_OBJECT
	public:
		enum { None, DTCP, IBB };
		enum { Idle, Connecting, WaitingForAccept, Active };
		enum { ErrConnect, ErrStream };
		enum { StatDTCPRequesting, StatDTCPAccepted, StatDTCPConnected, StatIBBRequesting, StatIBBConnected };
		JidLink(Client *client);
		~JidLink();

		void connectToJid(const Jid &jid, int type, const QDomElement &comment);
		void accept();

		int type() const;
		Jid peer() const;
		int state() const;

		bool isOpen() const;
		void close();
		void write(const QByteArray &);
		QByteArray read(int bytes=0);
		int bytesAvailable() const;
		int bytesToWrite() const;

	signals:
		void connected();
		void connectionClosed();
		void readyRead();
		void bytesWritten(int);
		void error(int);
		void status(int);

	private slots:
		void dtcp_connected();
		void dtcp_accepted();
		void ibb_connected();

		void bs_connectionClosed();
		void bs_error(int);
		void bs_readyRead();
		void bs_bytesWritten(int);

		void doRealAccept();

	private:
		class Private;
		Private *d;

		void reset(bool clear=false);

		void link();
		void unlink();

		friend class JidLinkManager;
		bool setStream(ByteStream *);
	};

	// the job of JidLinkManager is to keep track of streams and properly shut them down
	class JidLinkManager : public QObject
	{
		Q_OBJECT
	public:
		JidLinkManager(Client *);
		~JidLinkManager();

		JidLink *takeIncoming();

		void insertStream(ByteStream *);

	private:
		class Private;
		Private *d;
	};
}

#endif
