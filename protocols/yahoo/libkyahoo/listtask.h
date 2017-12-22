/*
    Kopete Yahoo Protocol
    Handles several lists such as buddylist, ignorelist and so on

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LISTTASK_H
#define LISTTASK_H

#include "task.h"
#include "yahootypes.h"

class QString;
class YMSGTransfer;
/**
@author André Duffeck
*/
class ListTask : public Task
{
    Q_OBJECT
public:
    ListTask(Task *parent);
    ~ListTask();

    bool take(Transfer *transfer) Q_DECL_OVERRIDE;

protected:
    bool forMe(const Transfer *transfer) const Q_DECL_OVERRIDE;
    void parseBuddyList(YMSGTransfer *transfer);

Q_SIGNALS:
    void gotBuddy(const QString &, const QString &, const QString &);
    void stealthStatusChanged(const QString &, Yahoo::StealthStatus);
};

#endif
