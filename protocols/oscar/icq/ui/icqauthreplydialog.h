/*
   Kopete Oscar Protocol
   icqauthreplydialog.h - ICQ authorization reply dialog

   Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

   Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef ICQAUTHREPLYDIALOG_H
#define ICQAUTHREPLYDIALOG_H

#include <kdialogbase.h>

class ICQAuthReplyUI;

/**
 * A dialog to ask user what to do when a contact requests authorization
 * @author Gustavo Pichorim Boiko
 */
class ICQAuthReplyDialog : public KDialogBase
{
Q_OBJECT
public:
	ICQAuthReplyDialog(QWidget *parent = 0, const char *name = 0, bool wasRequested = true);
	~ICQAuthReplyDialog();
	
	void setUser( const QString& user );
	void setRequestReason( const QString& reason );
	QString reason();
	bool grantAuth();
private:
	bool m_wasRequested;
	ICQAuthReplyUI *m_ui;
};

#endif
