/*
 * telepathyaddcontactpage.h - Telepathy Add Contact Page
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef TELEPATHY_ADDCONTACTPAGE_H
#define TELEPATHY_ADDCONTACTPAGE_H

#include <ui/addcontactpage.h>

#include <TelepathyQt4/PendingOperation>

class TelepathyAddContactPage : public AddContactPage
{
    Q_OBJECT
public:
    TelepathyAddContactPage(QWidget *parent = 0);
    ~TelepathyAddContactPage();

    virtual bool validateData();
    virtual bool apply(Kopete::Account *account, Kopete::MetaContact *parentMetaContact);

private:
    class Private;
    Private *d;
};

#endif
