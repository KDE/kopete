 /*
    icquserinfo.h  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Nick Betcher <nbetcher@kde.org>
    
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICQUSERINFO_H
#define ICQUSERINFO_H

#include <kdebug.h>
#include <qhbox.h>
#include <kdialogbase.h>


class QComboBox;
class OscarAccount;
class OscarContact;
class ICQUserInfoWidget;

class ICQUserInfo : public KDialogBase
{
	Q_OBJECT

public:
	ICQUserInfo( OscarContact *c, const QString name, OscarAccount *account, bool editable = false, QWidget *parent = 0, const char* name = "ICQUserInfo" );

private:
	QString mName;
	OscarAccount *mAccount;
	bool mEditable;
	ICQUserInfoWidget *mMainWidget;

	void sendInfo(void);
	void setEditable ( bool e );
	void setCombo ( QComboBox *combo, int type, int value );

private slots:
	void slotSaveClicked();
	void slotCloseClicked();
	void slotHomePageClicked(const QString &);
	void slotEmailClicked(const QString &);
	void slotFetchInfo(void); // initiate fetching info from server
	void slotReadInfo(void); // read in results from fetch

signals:
	void updateNickname(const QString);
	void closing( void );
};
#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

