
/***************************************************************************
                   jabberregister.h  -  Register dialog for Jabber
                             -------------------
    begin                : Sun Jul 11 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

		Kopete (C) 2001-2004 Kopete developers <kopete-devel@kde.org>
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

#include <kdialogbase.h>
#include <qregexp.h>
#include <qpixmap.h>

class DlgJabberRegisterAccount;
class JabberProtocol;
class JabberClient;
class JabberEditAccountWidget;

/**
@author Till Gerken
*/
class JabberRegisterAccount : public KDialogBase
{

Q_OBJECT

public:
	JabberRegisterAccount ( JabberEditAccountWidget *parent = 0, const char *name = 0 );

	~JabberRegisterAccount ();

	void setServer ( const QString &server );

private slots:
	void slotChooseServer ();
	void slotJIDInformation ();
	void slotSSLToggled ();
	void slotOk ();

	void slotHandleTLSWarning ( int validityResult );
	void slotCSError ( int error );
	void slotConnected ();

	void slotRegisterUserDone ();
	void slotDeleteDialog ();
	void validateData ();

	void disconnect ();

private:
	DlgJabberRegisterAccount *mMainWidget;
	JabberEditAccountWidget *mParentWidget;

	QRegExp jidRegExp;
	QPixmap hintPixmap;

	JabberClient *jabberClient;

	bool mSuccess;
};

#endif
