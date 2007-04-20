/*
   Kopete Oscar Protocol
   icqchangepassworddialog.h - ICQ change password dialog

   Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

   Kopete (c) 2006 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef ICQCHANGEPASSWORDDIALOG_H
#define ICQCHANGEPASSWORDDIALOG_H

#include <kdialog.h>

namespace Ui { class ICQChangePassword; }
class ICQAccount;

/**
 * A dialog for changing own password
 * @author Roman Jarosz
 */
class ICQChangePasswordDialog : public KDialog
{
	Q_OBJECT
public:
	explicit ICQChangePasswordDialog( ICQAccount* account, QWidget* parent = 0 );
	~ICQChangePasswordDialog();

protected slots:
	virtual void slotButtonClicked( int button );

private slots:
	void slotPasswordChanged( bool error );

private:
	Ui::ICQChangePassword* m_ui;
	ICQAccount* m_account;
};

#endif
