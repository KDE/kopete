/*
    kirchandler.h - IRC handler.

    Copyright (c) 2008      by Michel Hermier <michel.hermier@gmail.com>
    Copyright (c) 2008-2009 by Alexander Rieder <alexanderrieder@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCHANDLER_H
#define KIRCHANDLER_H

#include "kircmessage.h"

#include <QtCore/QObject>

namespace KIrc
{

class Context;
class Socket;

class HandlerPrivate;

class KIRC_EXPORT Handler
	: public QObject
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::Handler)
	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
	Q_ENUMS(Handled)

private:
	Q_DISABLE_COPY(Handler)

public:
	enum Handled
	{
		NotHandled	= (0<<0),
		CoreHandled	= (1<<0),
		PluginHandled	= (1<<1),
		FullyHandled	= CoreHandled | PluginHandled
	};

	explicit Handler(QObject *parent = 0);
	Handler(Handler *parent);
	virtual ~Handler();

	void addEventHandler(KIrc::Handler *handler);
	void removeEventHandler(KIrc::Handler *handler);

	bool isEnabled() const;
	void setEnabled(bool);	

#if 0
	bool registerCommand();
	bool unregisterCommand();
#endif
	void registerCommandAlias(const QByteArray &alias, const QByteArray &command);

#if 0
	bool registerMessage();
	bool unregisterMessage();
#endif

	void registerMessageAlias(const QByteArray &message, const QByteArray &alias);

	/** returns a list of commands, that are handled by this handler and
	 *  his subhandlers. Used to give an overview of what commands can 
	 *  be invoked by the user
	 */
	virtual KIrc::Command handledCommands();
public Q_SLOTS:
	virtual KIrc::Handler::Handled onCommand(KIrc::Context *context, const KIrc::Command &command/*, KIrc::Entity::Ptr from*/, KIrc::Socket* socket);
	virtual KIrc::Handler::Handled onMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

Q_SIGNALS:
/*
	(QString eventId, KIrc::Entity::Ptr from, KIrc::Entity::List to, QString text);
*/
protected:
	Handler(HandlerPrivate *d, QObject *parent = 0);
	Handler(HandlerPrivate *d, Handler *parent = 0);

	HandlerPrivate * const d_ptr;
};

}

#endif
