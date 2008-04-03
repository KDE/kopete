/*
    kirceventhandler.h - IRC event handler.

    Copyright (c) 2008      by Michel Hermier <michel.hermier@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCEVENTHANDLER_H
#define KIRCEVENTHANDLER_H

#include "kircglobal.h"

#include <QtCore/QObject>

namespace KIrc
{

class CommandEvent;
class MessageEvent;
class EventHandlerPrivate;

class KIRC_EXPORT EventHandler
	: public QObject
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::EventHandler)

public:
	explicit EventHandler(QObject *parent = 0);
	virtual ~EventHandler();

	bool isEnabled() const;
	void setEnabled(bool);	

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event);

	virtual bool commandEvent(KIrc::CommandEvent *event);
	virtual bool messageEvent(KIrc::MessageEvent *event);

private:
	Q_DISABLE_COPY(EventHandler)

	EventHandlerPrivate * const d_ptr;
};

}

#endif
