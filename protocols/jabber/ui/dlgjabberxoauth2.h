/*
    dlgjabberxoauth2.cpp - X-OAuth2 dialog

    Copyright (c) 2016 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef DLGJABBERXOAUTH2_H
#define DLGJABBERXOAUTH2_H

#include <kdialog.h>

class JabberAccount;
namespace Ui {
	class DlgXOAuth2;
}

class DlgJabberXOAuth2 : public KDialog {

Q_OBJECT

public:
	DlgJabberXOAuth2(JabberAccount *account, QWidget *parent = NULL);
	virtual ~DlgJabberXOAuth2();

private slots:
	void slotOk();
	void slotCancel();

private:
	Ui::DlgXOAuth2 *m_mainWidget;
	JabberAccount *m_account;
};

#endif
