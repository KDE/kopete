 /*

    Copyright (c) 2006      by Olivier Goffart  <ogoffart at kde.org>

    Kopete    (c) 2006 by the Kopete developers <kopete-devel@kde.org>


    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#ifndef JABBERBOOKMARKS_H
#define JABBERBOOKMARKS_H

#include <qobject.h>
#include <qdom.h>
#include <qstringlist.h>

namespace XMPP { class Jid; }
class JabberAccount;

class KAction;

class JabberBookmark
{
  public:
    typedef QList<JabberBookmark> List;

    JabberBookmark();

    void setJId( const QString &jid );
    QString jId() const;
    QString fullJId() const;

    void setName( const QString &name );
    QString name() const;

    void setNickName( const QString &name );
    QString nickName() const;

    void setPassword( const QString &password );
    QString password() const;

    void setAutoJoin( bool autoJoin );
    bool autoJoin() const;

  private:
    QString m_jId;
    QString m_name;
    QString m_nickName;
    QString m_password;
    bool m_autoJoin;
};

/**
 * This is a class that hanlde the bookmark collection  (JEP-0048)
 * There is one instance of that class by accounts.
 * @author Olivier Goffart 
 */
class JabberBookmarks : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Constructor
		 */
		JabberBookmarks(JabberAccount *parent);
		~JabberBookmarks(){}
		
		/**
		 * update or create en entry with the given jid.
		 * the jid resource is the nickname
		 */
		void insertGroupChat(const XMPP::Jid &jid);
		
		/**
		 * return an action that will be added in the jabber popup menu
		 */
		KAction *bookmarksAction(QObject * parent);
	private slots:
		void accountConnected();
		void slotReceivedBookmarks();
		void slotJoinChatBookmark(const QString&);
		

	private:
		JabberAccount *m_account;
		QDomDocument m_storage;

    static JabberBookmark::List bookmarksFromStorage( const QDomElement& );
    static QDomElement bookmarksToStorage( const JabberBookmark::List&, QDomDocument& );

		JabberBookmark::List m_bookmarks;
};

#endif
