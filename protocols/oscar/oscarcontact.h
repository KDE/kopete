/*
  oscarcontact.h  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
  Copyright (c) 2004 by Matt Rogers <matt.rogers@kdemail.net>
  Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#ifndef OSCARCONTACT_H
#define OSCARCONTACT_H

#include <qwidget.h>
#include <qdatetime.h>
#include <kurl.h>
#include "kopetecontact.h"
#include "kopetemessage.h"
#include "userdetails.h"
#include "client.h"
#include "oscartypeclasses.h"
#include "contact.h"
#include "kopete_export.h"

namespace Kopete
{
class ChatSession;
class OnlineStatus;
}

namespace Oscar
{
class Presence;
}

class OscarAccount;
class QTextCodec;
class OscarEncodingSelectionDialog;

/**
 * Contact for oscar protocol
 * @author Matt Rogers
 * @TODO Reimplement functions to do the following
 * \li get the idle time
 * \li get the real IP for the contact ( DC )
 * \li get the local IP for the contact
 * \li get the port for the DC
 * \li get the sign on time for the contact
 * \li get the status of authorization for the contact
 * \li get the status of authoziation for the contact
 * \li get the user info for the contact
 * \li check if the contact has a certain capability
 * \li request authorization from the contact
 * \li get and set the custom encoding for the contact
 * \li get and set the SSI group id for the contact
 * \li get and set whether the contact is server side or not
 * \li get/set the ignore setting for the contact
 * \li get/set the visibility setting for the contact ( i.e. are we visible to the contact )
 */
class OSCAR_EXPORT OscarContact : public Kopete::Contact
{
Q_OBJECT

public:
	OscarContact( Kopete::Account* account, const QString& name,
	              Kopete::MetaContact* parent, const QString& icon = QString() );
	
	virtual ~OscarContact();
	
	virtual void serialize(QMap<QString, QString>&, QMap<QString, QString>&);
	
	virtual Kopete::ChatSession *manager( Kopete::Contact::CanCreateFlags canCreate = Kopete::Contact::CanCreate );
	
	const QString &contactName() const { return mName; }
	OscarAccount *account() const { return mAccount; }
	
	bool isOnServer() const;

	virtual void setSSIItem( const OContact& item );
	OContact ssiItem() const;
	
	/** we received a typing notification from this contact, tell any message manager */
	void startedTyping();
	void stoppedTyping();

	/**
	 * Returns codec for contact's encoding or default one
	 * if contact has no encoding
	 */
	QTextCodec *contactCodec() const;

	/**
	 * Returns true if contact has this capability
	 */
	bool hasCap( int capNumber ) const;

	/**
	 * Set presence target
	 * Helper function that converts Oscar::Presence to Kopete::OnlineStatus and sets OnlineStatus.
	 */
	void setPresenceTarget( const Oscar::Presence &presence );

	/**
	 * Set encoding for this contact
	 * @param mib the MIBenum
	 * @note If @p mib is 0 then default encoding will be used
	 */
	virtual void setEncoding( int mib );

public slots:	
	/** Remove this contact from the server. Reimplemented from Kopete::Contact */
	virtual void deleteContact();

	/** the metacontact owning this contact changed */
	virtual void sync(unsigned int flags);
	
	/** our user info has been updated */
	virtual void userInfoUpdated( const QString& contact, const UserDetails& details );
	
	/** a user is online */
	virtual void userOnline( const QString& ) = 0;
	
	/** a user is offline */
	virtual void userOffline( const QString& ) = 0;

	/** send a file to this contact */
	virtual void sendFile( const KUrl &sourceURL = KUrl(), const QString &fileName = QString(), uint fileSize = 0L );

	/** set a away message */
	virtual void setAwayMessage( const QString &message );

	/** change contact encoding */
	void changeContactEncoding();

	void requestAuthorization();

protected slots:
	void slotTyping( bool typing );
	void messageAck( const QString& contact, uint messageId );
	void messageError( const QString& contact, uint messageId );

protected:
	OscarAccount *mAccount;
	QString mName;
	Kopete::ChatSession *mMsgManager;
	UserDetails m_details;
	OContact m_ssiItem;
	int m_warningLevel;
	QString m_clientFeatures;

protected slots:
	virtual void slotSendMsg( Kopete::Message& msg, Kopete::ChatSession* session);
	
private slots:
	void changeEncodingDialogClosed( int );
	void chatSessionDestroyed();
	void requestBuddyIcon();
	void haveIcon( const QString&, QByteArray );
	void receivedStatusMessage( const QString& userId, const QString& message );
	
private:
	QString filterAwayMessage( const QString &message ) const;
	int oscarFontSize( int size ) const;
	QString brMargin( int margin, int fontPointSize, bool forceBr = false ) const;

	bool m_buddyIconDirty;

	OscarEncodingSelectionDialog* m_oesd;
};

#endif
//kate: tab-width 4; indent-mode csands;
