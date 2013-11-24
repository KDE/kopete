/*
    yahooverifyaccount.h - UI Page for Verifying a locked account

    Copyright (c) 2005 by André Duffeck          <duffeck@kde.org>

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
#include <kdialog.h>
#include <kio/job.h>

namespace Kopete { class Account; }
namespace Ui { class YahooVerifyAccountBase; }
class KTemporaryFile;

class YahooVerifyAccount : public KDialog
{
	Q_OBJECT
private:
	Kopete::Account *mTheAccount;
	KTemporaryFile *mFile;
	Ui::YahooVerifyAccountBase *mTheDialog;
public:
	explicit YahooVerifyAccount(Kopete::Account *account, QWidget *parent = 0);
	~YahooVerifyAccount();

	virtual bool validateData();

	void setUrl( const KUrl &url );

protected slots:
	virtual void slotClose();
	virtual void slotApply();
public slots:
	void slotData( KIO::Job *job, const QByteArray& data );
	void slotComplete( KJob *job );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

