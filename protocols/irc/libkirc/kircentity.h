/*
    kircentity.h - IRC Client

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

#ifndef KIRCENTITY_H
#define KIRCENTITY_H

#include "kircconst.h"

#include <QObject>

class QTextCodec;

namespace KIRC
{

class Engine;
class EntityPrivate;

class Entity
	: public QObject,
	  public KShared
{
	Q_OBJECT

public:
	static KIRC::EntityType guessType(const QString &name);
	static bool isChannel( const QString &name );
	static bool isUser( const QString &name );

	Entity(const QString &name = QString::null, const KIRC::EntityType type = Unknown);
	virtual ~Entity();

	bool operator == (const Entity &);

	KIRC::EntityStatus status() const;

	KIRC::EntityType type() const;
	bool isChannel() const;
	bool isUser() const;
	void setType( KIRC::EntityType );

	KIRC::EntityType guessType();

	QString name() const;
	void setName(const QString &);

	QString host() const;

	QString awayMessage() const;
	void setAwayMessage(const QString &);

	QString modes() const;
	QString setModes(const QString &);

	QTextCodec *codec() const;
	void setCodec(QTextCodec *);

signals:
	void destroyed(KIRC::Entity *self);

	void updated();

private:

	static QString userInfo(const QString &s, int num_cap);

	static const QRegExp sm_userRegExp;
	static const QRegExp sm_userStrictRegExp;
	static const QRegExp sm_channelRegExp;

	EntityPrivate *d;
};

}

#endif
