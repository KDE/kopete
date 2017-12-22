/*
    Kopete Yahoo Protocol
    Notifies about new mails

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

#include "mailnotifiertask.h"

#include <qstring.h>

#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include "yahoo_protocol_debug.h"

MailNotifierTask::MailNotifierTask(Task *parent) : Task(parent)
{
    qCDebug(YAHOO_PROTOCOL_LOG);
}

MailNotifierTask::~MailNotifierTask()
{
}

bool MailNotifierTask::take(Transfer *transfer)
{
    if (!forMe(transfer)) {
        return false;
    }

    YMSGTransfer *t = static_cast<YMSGTransfer *>(transfer);

    parseMail(t);

    return true;
}

bool MailNotifierTask::forMe(const Transfer *transfer) const
{
    const YMSGTransfer *t = nullptr;
    t = dynamic_cast<const YMSGTransfer *>(transfer);
    if (!t) {
        return false;
    }

    if (t->service() == Yahoo::ServiceNewMail) {
        return true;
    } else {
        return false;
    }
}

void MailNotifierTask::parseMail(YMSGTransfer *t)
{
    qCDebug(YAHOO_PROTOCOL_LOG);

    QString count = t->firstParam(9);
    QString mail = t->firstParam(42);
    QString from = t->firstParam(43);
    QString subject = t->firstParam(18);

    if (!mail.isEmpty() && !from.isEmpty() && !subject.isEmpty()) {
        emit mailNotify(QStringLiteral("%1 <%2>").arg(from, mail), subject, count.toInt());
    } else {
        emit mailNotify(QString(), QString(), count.toInt());
    }
}
