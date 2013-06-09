
/***************************************************************************
                   jabberregister.h  -  Register dialog for Jabber
                             -------------------
    begin                : Sun Jul 11 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

    Copyright 2006 by Tommi Rantala <tommi.rantala@cs.helsinki.fi>

		Kopete (C) 2001-2006 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERREGISTER_H
#define JABBERREGISTER_H

#include <kdialog.h>
#include <QRegExp>
#include <QPixmap>
#include <QtCrypto>

namespace Ui { class DlgJabberRegisterAccount; }
class JabberClient;
class JabberEditAccountWidget;

/**
@author Till Gerken
*/
class JabberRegisterAccount : public KDialog
{

Q_OBJECT

public:
	JabberRegisterAccount ( JabberEditAccountWidget *parent = 0 );

	~JabberRegisterAccount ();

	void setServer ( const QString &server );

public slots:
	virtual void accept ();

private slots:
	void slotChooseServer ();
	void slotJIDInformation ();
	void slotSSLToggled ();
	void slotOverrideHostToggled();
	void slotOk ();

	void slotHandleTLSWarning ( QCA::TLS::IdentityResult, QCA::Validity );
	void slotCSError ( int error );
	void slotConnected ();

	void slotRegisterUserDone ();
	void slotDeleteDialog ();
	void validateData ();

	void disconnect ();

	void slotDebugMessage ( const QString &msg );

private:
	Ui::DlgJabberRegisterAccount *mMainWidget;
	JabberEditAccountWidget *mParentWidget;

	QRegExp jidRegExp;
	QPixmap hintPixmap;

	JabberClient *jabberClient;

	bool mSuccess;
};

#endif
