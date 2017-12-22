/*
  * jabberformlineedit.h
  *
  * Copyright (c) 2002-2003 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#ifndef JABBERFORMLINEEDIT_H
#define JABBERFORMLINEEDIT_H

#include <qwidget.h>
#include <QLineEdit>

#include "xmpp_tasks.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class JabberFormLineEdit : public QLineEdit
{
    Q_OBJECT public:
    JabberFormLineEdit (const int type, const QString &realName, const QString &value, QWidget *parent = nullptr);
    ~JabberFormLineEdit ();

public Q_SLOTS: void slotGatherData(XMPP::Form &form);

private:
    int fieldType;
    QString fieldName;
};

#endif
