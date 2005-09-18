/*
    kircentity.cpp - IRC Client

    Copyright (c) 2004      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircentity.moc"

#include <kdebug.h>

using namespace KIRC;

/**
 * Match a possible user definition:
 * nick!user@host
 * where user and host are optionnal.
 * NOTE: If changes are done to the regexp string, update also the sm_userStrictRegExp regexp string.
 */
const QRegExp Entity::sm_userRegExp(QString::fromLatin1("^([^\\s,:!@]+)(?:(?:!([^\\s,:!@]+))?(?:@([^\\s,!@]+)))?$"));

/**
 * Regexp to match strictly the complete user definition:
 * nick!user@host
 * NOTE: If changes are done to the regexp string, update also the sm_userRegExp regexp string.
 */
const QRegExp Entity::sm_userStrictRegExp(QString::fromLatin1("^([^\\s,:!@]+)!([^\\s,:!@]+)@([^\\s,:!@]+)$"));

const QRegExp Entity::sm_channelRegExp( QString::fromLatin1("^[#!+&][^\\s,]+$") );

// FIXME: Implement me
EntityType Entity::guessType(const QString &)
{
	return Unknown;
}

bool Entity::isUser( const QString &name )
{
	return sm_userRegExp.exactMatch(name);
}

bool Entity::isChannel( const QString &name )
{
	return sm_channelRegExp.exactMatch(name);
}

class KIRC::Entity::Private
{
public:
	Private()
		: codec(0)
	{ }

	EntityType type;

	QString name;
	QString host;

	QString awayMessage;
	QString modes;

	QTextCodec *codec;
};

Entity::Entity(const QString &name, const EntityType type)
	: d( new Private )
{
	setName(name);
	setType(type);

	if (d->type == Unknown)
		guessType();
}

Entity::~Entity()
{
	emit destroyed(this);

	delete d;
}

bool Entity::operator == (const Entity &) const
{
	#warning Implement Me
	return false;
}

EntityType Entity::type() const
{
	return d->type;
}

bool Entity::isChannel() const
{
	return type() == Channel;
}

bool Entity::isUser() const
{
	return type() == User;
}

void Entity::setType( EntityType type )
{
	if ( d->type != type )
	{
//		d->status.type = type;
		emit updated();
	}
}

EntityType Entity::guessType()
{
	setType( guessType(d->name) );
	return type();
}

QString Entity::name() const
{
	return d->name;
}

void Entity::setName(const QString &name)
{
	if ( d->name != name )
	{
		d->name = name;
		emit updated();
	}
}

QString Entity::host() const
{
	return d->host;
}

void Entity::setAwayMessage(const QString &awayMessage)
{
	if ( d->awayMessage != awayMessage )
	{
		d->awayMessage = awayMessage;
		emit updated();
	}
}

QString Entity::modes() const
{
	return d->modes;
}

QString Entity::setModes(const QString &modes)
{
	#warning this needs more logic to handle the +/- modes.
	if ( d->modes != modes )
	{
		d->modes = modes;
		emit updated();
	}
	return d->modes;
}

QTextCodec *Entity::codec() const
{
	return d->codec;
}

void Entity::setCodec(QTextCodec *codec)
{
	if ( d->codec != codec )
	{
		d->codec = codec;
		emit updated();
	}
}

