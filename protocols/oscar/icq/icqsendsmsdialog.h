/*
    icqreadaway.h  -  ICQ Protocol Plugin

    Copyright (c) 2003 by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICQSENDSMSDIALOG_H
#define ICQSENDSMSDIALOG_H

#include <kdebug.h>
#include <kdialogbase.h>

class ICQProtocol;
class ICQAccount;
class ICQContact;
class KLineEdit;
class KTextEdit;
class QVBox;

class ICQSendSMSDialog : public KDialogBase
{
	Q_OBJECT

	public:
		ICQSendSMSDialog(ICQAccount *, ICQContact *, QWidget *parent = 0, const char* name = "ICQSendSMSDialog");

	private slots:
		void slotSendShortMessage();
		void slotCloseClicked();

	signals:
		void closing();

	private:
		ICQProtocol *p;
		ICQAccount *mAccount;
		ICQContact *mContact;

		QVBox *mMainWidget;
		QLabel *lblNumber;
		KLineEdit *edtNumber;
		QLabel *lblMessage;
		KTextEdit *edtMessage;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
