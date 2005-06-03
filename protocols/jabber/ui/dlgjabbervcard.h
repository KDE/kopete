
/***************************************************************************
                          dlgjabbervcard.h  -  vCard dialog
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGJABBERVCARD_H
#define DLGJABBERVCARD_H

#include <kdialogbase.h>
#include "xmpp_vcard.h"

class JabberAccount;
class JabberContact;
class QString;
class dlgVCard;

class dlgJabberVCard : public KDialogBase
{
	Q_OBJECT

public:
	dlgJabberVCard (JabberAccount *account, JabberContact *contact, QWidget * parent = 0, const char *name = 0);
	~dlgJabberVCard ();

signals:
	void informationChanged();

private slots:
	void slotSaveNickname();
	void slotSaveVCard();
	void slotClose();
	void slotOpenURL(const QString &url);

private:
	JabberAccount *m_account;
	JabberContact *m_contact;
	dlgVCard *m_mainWidget;

	void assignContactProperties();
	void setReadOnly(bool state);

};

#endif // DLGJABBERVCARD_H

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
