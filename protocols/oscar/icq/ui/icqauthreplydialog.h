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

#include <kdialog.h>

namespace Ui { class ICQAuthReplyUI; }

/**
 * A dialog to ask user what to do when a contact requests authorization
 * @author Gustavo Pichorim Boiko
 */
class ICQAuthReplyDialog : public KDialog
{
Q_OBJECT
public:
	explicit ICQAuthReplyDialog(QWidget *parent = 0, bool wasRequested = true);
	~ICQAuthReplyDialog();
	
	void setUser( const QString& user );
	void setRequestReason( const QString& reason );
	void setContact( const QString& contact );

	QString reason() const;
	QString contact() const;
	bool grantAuth() const;
private:
	bool m_wasRequested;
	QString m_contact;

	Ui::ICQAuthReplyUI *m_ui;
};

#endif
