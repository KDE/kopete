/*
    action.h -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

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
		// Method which show the description of an action
		void viewAction();

		Argument* getArgumentAt(int i);
};

#endif

