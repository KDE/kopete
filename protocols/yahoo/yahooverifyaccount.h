/*
    yahooverifyaccount.h - UI Page for Verifying a locked account

    Copyright (c) 2005 by André Duffeck          <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __YAHOOVERIFYACCOUNT_H
#define __YAHOOVERIFYACCOUNT_H

// Local Includes

// Kopete Includes
// QT Includes

// KDE Includes
#include <kdialogbase.h>

namespace Kopete { class Account; }
class YahooVerifyAccountBase;
class KTempFile;

class YahooVerifyAccount : public KDialogBase
{
	Q_OBJECT
private:
	Kopete::Account *mTheAccount;
	KTempFile *mFile;
	YahooVerifyAccountBase *mTheDialog;
public:
	YahooVerifyAccount(Kopete::Account *account, QWidget *parent = 0, const char *name = 0);
	~YahooVerifyAccount();

	virtual bool validateData();

	void setUrl( KURL url );

protected slots:
	virtual void slotClose();
	virtual void slotApply();
public slots:
	void slotData( KIO::Job *job, const QByteArray& data );
	void slotComplete( KIO::Job *job );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

