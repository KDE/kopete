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

//	Q_PROPERTY(QString modes READ modes write setModes)

public:
	static KIRC::EntityType guessType(const QString &name);
	static bool isChannel( const QString &name );
	static bool isUser( const QString &name );

	Entity(const QString &name = QString::null, const KIRC::EntityType type = Unknown);
	virtual ~Entity();

public: // Read attributes accessors
	QString awayMessage() const;
	QString host() const;
	QString modes() const;
	QString name() const;
//	QString nick() const;
//	QString topic() const;
//	QString user() const;

public slots: // Write attributes accessors
	void setAwayMessage(const QString &);
	QString setModes(const QString &);
	void setName(const QString &);
//	void setNick(const QString &);
//	void setTopic(const QString &);
//	void setUser(const QString &);

public:
	bool operator == (const Entity &) const;

	KIRC::EntityStatus status() const;

	KIRC::EntityType type() const;
	bool isChannel() const;
	bool isUser() const;
	void setType( KIRC::EntityType );

	KIRC::EntityType guessType();

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
