//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SETSTATUSTASK_H
#define SETSTATUSTASK_H

#include "gwerror.h"
#include "requesttask.h"

/**
@author Kopete Developers
*/
class SetStatusTask : public RequestTask
{
Q_OBJECT
public:
	SetStatusTask(Task* parent);
	~SetStatusTask();
	void status( GroupWise::Status newStatus, const QString &awayMessage, const QString &autoReply );
	GroupWise::Status requestedStatus() const;
	QString awayMessage() const;
	QString autoReply() const; 
private:
	GroupWise::Status m_status;
	QString m_awayMessage;
	QString m_autoReply;
};

#endif
