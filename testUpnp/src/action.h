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
		// Action name
		QString m_name;
		// Argument list of the action
		QList<Argument> m_argumentList;
	public:
		// Construtor
		Action();
		Action(QString name);
		// Method which add an argument to the argument list
		void addArgument(QString name, QString direction, QString relatedStateVariable);
		// Getter
		QString name();
		QList<Argument>* listArgument();
		// Setters
		void setName(QString name);
		// Method which show the argument list
		void viewListArgument();
		// Method which test the equality of two action
		bool operator==(const Action &act);
};

#endif

