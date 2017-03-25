/*
    kircentity.h - IRC Client

    Copyright (c) 2004-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2004-2007 by the Kopete developers <kopete-devel@kde.org>

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

#include "kirc_export.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QSharedData>

class QTextCodec;

namespace KIrc {
class Context;
class EntityPrivate;

class KIRC_EXPORT Entity : public QObject, public QSharedData
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Entity)

//	Q_PROPERTY(QTextCodec *codec READ codec WRITE setCodec)

//	Q_PROPERTY(QByteArray awayMessage READ awayMessage WRITE setAwayMessage)
//	Q_PROPERTY(QByteArray name READ name WRITE setName)
//	Q_PROPERTY(QByteArray modes READ modes WRITE setModes)
//	Q_PROPERTY(QByteArray topic READ topic WRITE setTopic)

//	Q_ENUMS(Type)

public:
    friend class KIrc::Context;

    enum Type
    {
        // From lower to higher importance
        Unknown,
        User,
        Service,
        Channel,
        Server
    };
/*
    static EntityType guessType(const QByteArray &name);
    static bool isChannel( const QByteArray &name );
    static bool isUser( const QByteArray &name );
*/
    Entity(KIrc::Context *context);
    virtual ~Entity();

public: // Read attributes accessors
    QByteArray awayMessage() const;
    QByteArray host() const;
    QByteArray modes() const;
    QByteArray name() const;
//	QByteArray nick() const;
    QByteArray topic() const;
//	QByteArray user() const;

public slots: // Write attributes accessors
    void setAwayMessage(const QByteArray &);
    QByteArray setModes(const QByteArray &);
    void setName(const QByteArray &);
//	void setNick(const QByteArray &);
//	void setTopic(const QByteArray &);
//	void setUser(const QByteArray &);

public:
    Entity::Type type() const;
    bool isChannel() const;
    bool isUser() const;
    void setType(Entity::Type type);

//	EntityType guessType();

    QTextCodec *codec() const;
    void setCodec(QTextCodec *);

signals:
    void destroyed(Entity *self);

    void updated();

private:
    static QByteArray userInfo(const QByteArray &s, int num_cap);

    Q_DISABLE_COPY(Entity)

    EntityPrivate * const d_ptr;
};
}

#endif
