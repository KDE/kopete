//
// C++ Interface: gwreceiveinvitationdialog
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GWRECEIVEINVITATIONDIALOG_H
#define GWRECEIVEINVITATIONDIALOG_H

#include <kdialogbase.h>

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
	void invitationAccepted( bool, const QString & guid );
protected slots:
	void slotYesClicked();
	void slotNoClicked();
private:
	GroupWiseAccount * m_account;
	QString m_guid; // the conference we were invited to join.
};

#endif
