/*
 * client.h - main library class
 * Copyright (C) 2001, 2002  Justin Karneges
 *                           Akito Nozaki
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
  TODO:

  core:
  - Are we deleting task daemons on disconnect????
  - JT_ServInfo needs to return client time
  - JT_ClientTime

  general:
  - Task timeouts
  - Separate QSSL plugin handling from Jabber::Stream so that DTCP can use it too
  - Remove LiveRoster management from Jabber::Client, too limiting for clients
  - SSL client/server support in DTCP
  - JidLink: ensure the class is kosher
  - JidLinkManager: handle objects that are shutting down
  - can't reconnect after 'socket error'
  - QSocket::writeBlock error while trying to reconnect
  - turn rosterAdded into rosterUpdated.  no need for rosterAdded
*/

#ifndef JABBER_CLIENT_H
#define JABBER_CLIENT_H

#include<qobject.h>
#include<qvaluelist.h>
#include"xmpp_types.h"

class QString;
class QDomElement;
class QDomDocument;
class QSSLCert;

namespace Jabber
{
	class Client;
	class Stream;
	class StreamError;
	class StreamProxy;
	class LiveRosterItem;
	class LiveRoster;
	class Message;
	class DTCPManager;
	class DTCPConnection;
	class IBBManager;
	class IBBConnection;
	class JidLink;
	class JidLinkManager;

	class Task : public QObject
	{
		Q_OBJECT
	public:
		enum { ErrDisc };
		Task(Task *parent);
		Task(Client *, bool isRoot);
		virtual ~Task();

		Task *parent() const;
		Client *client() const;
		QDomDocument *doc() const;
		QString id() const;

		bool success() const;
		int statusCode() const;
		const QString & statusString() const;

		void go(bool autoDelete=false);
		virtual bool take(const QDomElement &);
		void safeDelete();

	signals:
		void finished();

	protected:
		virtual void onGo();
		virtual void onDisconnect();
		void send(const QDomElement &);
		void setSuccess(int code=0, const QString &str="");
		void setError(const QDomElement &);
		void setError(int code=0, const QString &str="");
		void debug(const char *, ...);
		void debug(const QString &);
		bool iqVerify(const QDomElement &x, const Jid &to, const QString &id, const QString &xmlns="");

	private slots:
		void clientDisconnected();
		void done();

	private:
		void init();

		class TaskPrivate;
		TaskPrivate *d;
	};

	class Client : public QObject
	{
		Q_OBJECT

	public:
		Client(QObject *parent=0);
		~Client();

		bool isActive() const;
		bool isConnected() const;
		bool isHandshaken() const;
		bool isAuthenticated() const;
		void connectToHost(const QString &host, int port=-1, const QString &virtualHost="");
		void setProxy(const StreamProxy &);
		void close(bool fast=false);

		void setNoopTime(int);

		bool isSSLEnabled() const;
		bool setSSLEnabled(bool);

		//void setQSSL(QSSL *);
		void setSSLTrustedCertStoreDir(const QString &);

		Stream & stream();
		const LiveRoster & roster() const;
		const ResourceList & resourceList() const;

		void send(const QDomElement &);
		void send(const QString &);

		QString host() const;
		QString user() const;
		QString pass() const;
		QString resource() const;
		Jid jid() const;

		void authPlain(const QString &user, const QString &pass, const QString &resource);
		void authDigest(const QString &user, const QString &pass, const QString &resource);
		void rosterRequest();
		void sendMessage(const Message &);
		void sendSubscription(const Jid &, const QString &);
		void setPresence(const Status &);

		void debug(const QString &);
		QString genUniqueId();
		Task *rootTask();
		QDomDocument *doc() const;

		QString OSName() const;
		QString timeZone() const;
		int timeZoneOffset() const;
		QString clientName() const;
		QString clientVersion() const;

		void setOSName(const QString &);
		void setTimeZone(const QString &, int);
		void setClientName(const QString &);
		void setClientVersion(const QString &);

		DTCPManager *dtcpManager() const;
		IBBManager *ibbManager() const;
		JidLinkManager *jidLinkManager() const;

		bool groupChatJoin(const QString &host, const QString &room, const QString &nick);
		void groupChatSetStatus(const QString &host, const QString &room, const Status &);
		void groupChatChangeNick(const QString &host, const QString &room, const QString &nick, const Status &);
		void groupChatLeave(const QString &host, const QString &room);

	signals:
		void connected();
		void handshaken();
		void error(const StreamError &);
		void sslCertReady(const QSSLCert &);
		void closeFinished();
		void disconnected();
		void authFinished(bool, int, const QString &);
		void rosterRequestFinished(bool, int, const QString &);
		void rosterItemAdded(const RosterItem &);
		void rosterItemUpdated(const RosterItem &);
		void rosterItemRemoved(const RosterItem &);
		void resourceAvailable(const Jid &, const Resource &);
		void resourceUnavailable(const Jid &, const Resource &);
		void presenceError(const Jid &, int, const QString &);
		void subscription(const Jid &, const QString &);
		void messageReceived(const Message &);
		void debugText(const QString &);
		void xmlIncoming(const QString &);
		void xmlOutgoing(const QString &);
		void groupChatJoined(const Jid &);
		void groupChatLeft(const Jid &);
		void groupChatPresence(const Jid &, const Status &);
		void groupChatError(const Jid &, int, const QString &);

		void incomingJidLink();

	public slots:
		void continueAfterCert();

	private slots:
		void streamConnected();
		void streamHandshaken();
		void streamError(const StreamError &);
		void streamSSLCertificateReady(const QSSLCert &);
		void streamCloseFinished();
		void streamReceivePacket(const QDomElement &);

		void slotAuthFinished();
		void slotRosterRequestFinished();

		// basic daemons
		void ppSubscription(const Jid &, const QString &);
		void ppPresence(const Jid &, const Status &);
		void pmMessage(const Message &);
		void prRoster(const Roster &);

		void dtcp_incomingReady();
		void ibb_incomingReady();

	public:
		class GroupChat;
	private:
		void cleanup();
		void distribute(const QDomElement &);
		void importRoster(const Roster &);
		void importRosterItem(const RosterItem &);
		void updateSelfPresence(const Jid &, const Status &);
		void updatePresence(LiveRosterItem *, const Jid &, const Status &);

		class ClientPrivate;
		ClientPrivate *d;
	};

	class LiveRosterItem : public RosterItem
	{
	public:
		LiveRosterItem(const Jid &j="");
		LiveRosterItem(const RosterItem &);
		~LiveRosterItem();

		void setRosterItem(const RosterItem &);

		ResourceList & resourceList();
		ResourceList::Iterator priority();

		const ResourceList & resourceList() const;
		ResourceList::ConstIterator priority() const;

		bool isAvailable() const;
		const Status & lastUnavailableStatus() const;
		bool flagForDelete() const;

		void setLastUnavailableStatus(const Status &);
		void setFlagForDelete(bool);

	private:
		ResourceList v_resourceList;
		Status v_lastUnavailableStatus;
		bool v_flagForDelete;

		class LiveRosterItemPrivate;
		LiveRosterItemPrivate *d;
	};

	class LiveRoster : public QValueList<LiveRosterItem>
	{
	public:
		LiveRoster();
		~LiveRoster();

		void flagAllForDelete();
		LiveRoster::Iterator find(const Jid &, bool compareRes=true);
		LiveRoster::ConstIterator find(const Jid &, bool compareRes=true) const;

	private:
		class LiveRosterPrivate;
		LiveRosterPrivate *d;
	};
}

#endif
