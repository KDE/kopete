/*
    YahooInviteListImpl - conference invitation dialog
    
    Copyright (c) 2004 by Duncan Mac-Vicar P.    <duncan@kde.org>
    
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

#ifndef YAHOO_INVITE_LIST_IMPL
#define YAHOO_INVITE_LIST_IMPL

#include <qwidget.h>
#include <kdialog.h>
#include "ui_yahooinvitelistbase.h"


class YahooInviteListImpl : public KDialog
{
	Q_OBJECT
public: 
	YahooInviteListImpl(QWidget *parent=0);
	~YahooInviteListImpl();
	
	void fillFriendList( const QStringList &buddies );
	void addInvitees( const QStringList &buddies );
	void removeInvitees( const QStringList &buddies );
	void setRoom( const QString &room );
	void addParticipant( const QString &participant );
private:
	
signals:
	void readyToInvite( const QString &room, const QStringList &buddies, const QStringList &participants, const QString &msg );
protected slots:

public slots:
	virtual void slotInvite();
	virtual void slotCancel();
	virtual void slotAddCustom();
	virtual void slotRemove();
	virtual void slotAdd();
private:
	void updateListBoxes();

	QStringList m_buddyList;
	QStringList m_inviteeList;
	QStringList m_participants;
	QString m_room;

	Ui::YahooInviteListBase* m_inviteWidget;
};

#endif

