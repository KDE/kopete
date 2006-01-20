/*
    yahooauthreply.h - UI Page for Accepting/Rejecting an authorization request

    Copyright (c) 2006 by André Duffeck          <andre.duffeck@kdemail.net>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __YAHOOAUTHREPLY_H
#define __YAHOOAUTHREPLY_H

// Local Includes

// Kopete Includes
// QT Includes

// KDE Includes
#include <kdialogbase.h>

class YahooAuthReplyBase;
class KTempFile;

class YahooAuthReply : public KDialogBase
{
	Q_OBJECT
public:
	YahooAuthReply(QWidget *parent = 0, const char *name = 0);
	~YahooAuthReply();

	void setUser( const QString& user );
	void setName( const QString& name );
	void setRequestReason( const QString& reason );

	bool acceptAuth();
	QString user();
	QString reason();
private:
	void updateReqLabel();
	YahooAuthReplyBase *mTheDialog;
	QString m_User;
	QString m_Name;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

