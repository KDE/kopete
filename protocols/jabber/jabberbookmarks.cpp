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

#include "jabberbookmarks.h"
#include "jabberaccount.h"

#include <kopetecontact.h>


#include <QPointer>
#include <kdebug.h>
#include <kaction.h>
#include <kselectaction.h>
#include <klocale.h>
#include <kicon.h>

#include "tasks/jt_privatestorage.h"
#include "ui/dlgjabberbookmarkeditor.h"

JabberBookmark::JabberBookmark() : m_autoJoin( false )
{
}

void JabberBookmark::setJId( const QString &jid )
{
	m_jId = jid;
}

QString JabberBookmark::jId() const
{
	return m_jId;
}

QString JabberBookmark::fullJId() const
{
	if ( !m_nickName.isEmpty() )
		return m_jId + '/' + m_nickName;
	else
		return m_jId;
}

void JabberBookmark::setName( const QString &name )
{
	m_name = name;
}

QString JabberBookmark::name() const
{
	return m_name;
}

void JabberBookmark::setNickName( const QString &name )
{
	m_nickName = name;
}

QString JabberBookmark::nickName() const
{
	return m_nickName;
}

void JabberBookmark::setPassword( const QString &password )
{
	m_password = password;
}

QString JabberBookmark::password() const
{
	return m_password;
}

void JabberBookmark::setAutoJoin( bool autoJoin )
{
	m_autoJoin = autoJoin;
}

bool JabberBookmark::autoJoin() const
{
	return m_autoJoin;
}

JabberBookmarks::JabberBookmarks(JabberAccount *parent) : QObject(parent) , m_account(parent) 
{
	connect( m_account , SIGNAL(isConnectedChanged()) , this , SLOT(accountConnected()) );
}

void JabberBookmarks::accountConnected()
{
	if(!m_account->isConnected())
		return;
	
	JT_PrivateStorage * task = new JT_PrivateStorage ( m_account->client()->rootTask ());
	task->get( "storage" , "storage:bookmarks" );
	QObject::connect ( task, SIGNAL (finished()), this, SLOT (slotReceivedBookmarks()) );
	task->go ( true );
}

JabberBookmark::List JabberBookmarks::bookmarksFromStorage( const QDomElement &storageElement )
{
	JabberBookmark::List bookmarks;
	if ( !storageElement.isNull() && storageElement.tagName() == "storage" ) {
		for ( QDomElement element = storageElement.firstChildElement(); !element.isNull(); element = element.nextSiblingElement() ) {

			if ( element.tagName() == "conference" ) {
				JabberBookmark bookmark;

				bookmark.setJId( element.attribute( "jid" ) );
				bookmark.setName( element.attribute( "name" ) );
				bookmark.setAutoJoin( element.attribute( "autojoin", "false" ) == "true" );

				for ( QDomElement childElement = element.firstChildElement(); !childElement.isNull(); childElement = childElement.nextSiblingElement() ) {
					if ( childElement.tagName() == "nick" ) {
						bookmark.setNickName( childElement.text() );
					} else if ( childElement.tagName() == "password" ) {
						bookmark.setPassword( childElement.text() );
					}
				}

				bookmarks += bookmark;
			}
		}
	}

	return bookmarks;
}

QDomElement JabberBookmarks::bookmarksToStorage( const JabberBookmark::List &bookmarks, QDomDocument &document )
{
	QDomElement storageElement = document.createElement( "storage" );
	storageElement.setAttribute( "xmlns", "storage:bookmarks" );

	foreach ( const JabberBookmark &bookmark, bookmarks ) {
		QDomElement conferenceElement = document.createElement( "conference" );
		conferenceElement.setAttribute( "jid", bookmark.jId() );

		if ( !bookmark.name().isEmpty() )
			conferenceElement.setAttribute( "name", bookmark.name() );

		if ( bookmark.autoJoin() )
			conferenceElement.setAttribute( "autojoin", "true" );

		if ( !bookmark.nickName().isEmpty() ) {
			QDomElement element = document.createElement( "nick" );
			element.appendChild( document.createTextNode( bookmark.nickName() ) );
			conferenceElement.appendChild( element );
		}

		if ( !bookmark.password().isEmpty() ) {
			QDomElement element = document.createElement( "password" );
			element.appendChild( document.createTextNode( bookmark.password() ) );
			conferenceElement.appendChild( element );
		}

		storageElement.appendChild( conferenceElement );
	}

	return storageElement;
}

void JabberBookmarks::slotReceivedBookmarks( )
{
	JT_PrivateStorage *task = (JT_PrivateStorage*)( sender() );
	m_bookmarks.clear();

	if ( task->success() ) {
		m_bookmarks = bookmarksFromStorage( task->element() );

		foreach ( const JabberBookmark &bookmark, m_bookmarks ) {
			if ( bookmark.autoJoin() ) {
				XMPP::Jid x_jid( bookmark.fullJId() );

				QString nickName = x_jid.resource();
				if ( nickName.isEmpty() )
					nickName = m_account->myself()->displayName();

				if ( bookmark.password().isEmpty() )
					m_account->client()->joinGroupChat( x_jid.domain(), x_jid.node(), nickName );
				else
					m_account->client()->joinGroupChat( x_jid.domain(), x_jid.node(), nickName, bookmark.password() );
			}
		}
	}
}


void JabberBookmarks::insertGroupChat(const XMPP::Jid &jid)
{
	bool containsConference = false;
	foreach ( const JabberBookmark &bookmark, m_bookmarks ) {
		if ( bookmark.fullJId() == jid.full() ) {
			containsConference = true;
			break;
		}
	}

	if ( containsConference || !m_account->isConnected() )
		return;

	JabberBookmark bookmark;
	bookmark.setJId( jid.bare() );
	bookmark.setNickName( jid.resource() );
	bookmark.setName( jid.full() );

	m_bookmarks.append( bookmark );

	QDomDocument document( "storage" );
	const QDomElement element = bookmarksToStorage( m_bookmarks, document );

	JT_PrivateStorage *task = new JT_PrivateStorage( m_account->client()->rootTask() );
	task->set( element );
	task->go( true );
}

KAction * JabberBookmarks::bookmarksAction(QObject *parent)
{
	Q_UNUSED( parent )

	QStringList menuEntries;
	foreach ( const JabberBookmark &bookmark, m_bookmarks ) {
		menuEntries << bookmark.fullJId();
	}

	if ( !menuEntries.isEmpty() ) {
		menuEntries << QString(); // separator
		menuEntries << i18n( "Edit Bookmarks..." );
	}

	KSelectAction *action = new KSelectAction( this );
	action->setIcon( KIcon( "jabber_group" ) );
	action->setText( i18n( "Groupchat Bookmark" ) );
	action->setItems( menuEntries );

	connect( action, SIGNAL(triggered(QString)), this, SLOT(slotJoinChatBookmark(QString)) );
	return action;
}

void JabberBookmarks::slotJoinChatBookmark( const QString & _jid )
{
	if ( !m_account->isConnected() )
		return;

	if ( _jid != i18n( "Edit Bookmarks..." ) ) {
		XMPP::Jid jid( _jid );
		m_account->client()->joinGroupChat( jid.domain(), jid.node(), jid.resource() );
	} else {
		QPointer <DlgJabberBookmarkEditor> editor = new DlgJabberBookmarkEditor( m_bookmarks );
		if ( editor->exec() && editor ) {
			m_bookmarks = editor->bookmarks();

			QDomDocument document( "storage" );
			const QDomElement element = bookmarksToStorage( m_bookmarks, document );

			JT_PrivateStorage *task = new JT_PrivateStorage( m_account->client()->rootTask() );
			task->set( element );
			task->go( true );
		}
		delete editor;
	}
}

#include "jabberbookmarks.moc"
