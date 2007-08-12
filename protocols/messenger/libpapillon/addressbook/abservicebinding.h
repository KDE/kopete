/*
	abservicebinding.h: Header file for Address Book Service Binding

    Copyright (c) 2007		by Zhang Panyong	        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers	<kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef ABSERVICEBINDING_H
#define ABSERVICEBINDING_H
namespace Papillon
{

namespace Internal
{

class FindABResult
{
	public:
	   ~FindABResult();

	private:
}

class AddressBook
{
public:
void setabid(const QString &);
const QString abid();
void setABInfo(QHash<QString, QString> abinfo);
QHash<QString, QString> abInfo();
void setLastChange(const QString &);
const QString lastChange();
void setDynamicItemLastChanged(const QString &);
const QString dynamicItemLastChanged();
void setCreateDate(const QString &);
const QString createDate();

private:
	QString m_abid;
	QHash<QString, QString> m_abInfo;
	QString m_LastChange;
	QString m_dynamicItemLastChanged;
	QString m_createDate;
}

}

}
#endif/* ABSERVICEBINDING_H*/
