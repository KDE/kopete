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
#ifndef KEEPALIVETASK_H
#define KEEPALIVETASK_H

#include <requesttask.h>

class QTimer;

/**
@author Kopete Developers
*/
class KeepAliveTask : public RequestTask
{
Q_OBJECT
public:
	KeepAliveTask(Task* parent);
	~KeepAliveTask();
protected slots:
	void slotSendKeepAlive();
private:
	QTimer * m_keepAliveTimer;
};

#endif
