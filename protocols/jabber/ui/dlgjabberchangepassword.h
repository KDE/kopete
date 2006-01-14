
/***************************************************************************
                   Change the password of a Jabber account
                             -------------------
    begin                : Tue May 31 2005
    copyright            : (C) 2005 by Till Gerken <till@tantalo.net>

		Kopete (C) 2001-2005 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGJABBERCHANGEPASSWORD_H
#define DLGJABBERCHANGEPASSWORD_H

#include <kdialogbase.h>

class JabberAccount;
class DlgChangePassword;

/**
@author Till Gerken
*/
class DlgJabberChangePassword : public KDialogBase
{

Q_OBJECT

public:
	DlgJabberChangePassword ( JabberAccount *account, QWidget *parent = 0, const char *name = 0);
	~DlgJabberChangePassword();

private slots:
	void slotOk ();
	void slotCancel ();
	void slotChangePassword ();
	void slotChangePasswordDone ();

private:
	DlgChangePassword *m_mainWidget;
	JabberAccount *m_account;
};

#endif
