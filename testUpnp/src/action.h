#ifndef _ACTION_H_
#define _ACTION_H_

#include "util_Xml.h"
#include <upnp/ixml.h>
#include <upnp/FreeList.h>

#include <upnp/ithread.h>
#include <upnp/LinkedList.h>
#include <upnp/ThreadPool.h>
#include <upnp/TimerThread.h>
#include <upnp/upnpconfig.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include <QList>
#include <QString>
#include "argument.h"

class Action
{
	private:
		QString m_name;
		QList<Argument> m_argumentList;
	public:
		Action();
		Action(QString name);
		void addArgument(QString name, QString direction, QString relatedStateVariable);
		QString name();
		QList<Argument> listArgument();
		void viewListArgument();
};

#endif

