/*
    Kopete Groupwise Protocol
    gwreceiveinvitationdialog.h - dialog shown when the user receives an invitation to chat

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
   Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GWRECEIVEINVITATIONDIALOG_H
#define GWRECEIVEINVITATIONDIALOG_H

#include <kdialogbase.h>

class ShowInvitationWidget;

/**
This is the dialog that is shown when you receive an invitation to chat.

@author SUSE AG
*/
class ReceiveInvitationDialog : public KDialogBase
{
Q_OBJECT
public:
	ReceiveInvitationDialog( GroupWiseAccount * account, const ConferenceEvent & event, QWidget *parent, const char *name );
	~ReceiveInvitationDialog();
signals:
	void invitationAccepted( bool, const GroupWise::ConferenceGuid & guid );
protected slots:
	void slotYesClicked();
	void slotNoClicked();
private:
	GroupWiseAccount * m_account;
	ConferenceGuid m_guid; // the conference we were invited to join.
	ShowInvitationWidget * m_wid;
};

#endif
