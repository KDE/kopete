/*
   sharingservicebinding.h - Binding to MSN Sharing web service for contact list

   Copyright (c) 2007 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#ifndef SHARINGSERVICEBINDING_H
#define SHARINGSERVICEBINDING_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QDateTime>

class QDomElement;

namespace Papillon
{

class HttpConnection;

namespace Internal
{

class FindMembershipResult;
class Membership;
class Member;
class Service;

class FindMembershipResult
{
public:
	~FindMembershipResult();

	void setServices(const QList<Service*> &services);
	QList<Service*> services() const;

private:
	QList<Service*> m_services;
};

class Service
{
public:
	~Service();
	void setMemberships(const QList<Membership*> &memberships);
	QList<Membership*> memberships() const;

	void setLastChange(const QDateTime &lastChange);
	QDateTime lastChange() const;

private:
	QList<Membership*> m_memberships;
	QDateTime m_lastChange;
};

class Membership
{
public:
	~Membership();
	void setMemberRole(const QString &memberRole);
	QString memberRole() const;

	void setMembers(const QList<Member*> &members);
	QList<Member*> members() const;

private:
	QString m_memberRole;
	QList<Member*> m_members;
};

class Member
{
public:
    void setMembershipId(unsigned int value);
    unsigned int membershipId() const;
    void setType(const QString &value);
    QString type() const;
    void setDisplayName(const QString &value);
    QString displayName() const;
    void setState(const QString &value);
    QString state() const;
    void setDeleted(bool value);
    bool deleted() const;
    void setLastChanged(const QDateTime &value);
    QDateTime lastChanged() const;
    void setChanges(const QString &value);
    QString changes() const;
    void setPassportName(const QString &value);
    QString passportName() const;
    void setIsPassportNameHidden(bool value);
    bool isPassportNameHidden() const;
    void setPassportId(int value);
    int passportId() const;
    void setCID(int value);
    int cID() const;
    void setPassportChanges(const QString &value);
    QString passportChanges() const;

private:
    unsigned int m_membershipId;
    QString m_type;
    QString m_displayName;
    QString m_state;
    bool m_deleted;
    QDateTime m_lastChanged;
    QString m_changes;
    QString m_passportName;
    bool m_isPassportNameHidden;
    int m_passportId;
    int m_cID;
    QString m_passportChanges;
};

/**
 * @brief Binding to MSN Sharing web service for contact list
 * @author Michaël Larouche <larouche@kde.org>
 */
class SharingServiceBinding : public QObject
{
	Q_OBJECT
public:
	explicit SharingServiceBinding(HttpConnection *connection, QObject *parent = 0);
	~SharingServiceBinding();

public slots:
	void findMembership();

signals:
	void findMembershipResult(Papillon::Internal::FindMembershipResult *result);

private slots:
	void connectionReadyRead();

private:
	/**
	 * @internal
	 * @brief Provide a syncronous method to connect to server.
	 * @param server Server
	 */
	void connectToServer(const QString &server);

	void parseFindMembershipResponse(const QDomElement &element);
	Service* parseService(const QDomElement &service);
	Membership* parseMembership(const QDomElement &membership);
	Member* parseMember(const QDomElement &member);

private:
	class Private;
	Private *d;
};

}

}

#endif
