 /*
    jabberformlineedit.h

    Copyright (c) 2002 by the Kopete Developers <kopete-devel@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef JABBERFORMLINEEDIT_H
#define JABBERFORMLINEEDIT_H

#include <qwidget.h>
#include <qlineedit.h>

#include <psi/types.h>
#include <psi/tasks.h>

/**
  *@author Kopete developers
  */

class JabberFormLineEdit : public QLineEdit
{

   Q_OBJECT

public: 
	JabberFormLineEdit(const int type, const QString &realName, const QString &value, QWidget *parent=0, const char *name=0);
	~JabberFormLineEdit();

public slots:
	void slotGatherData(Jabber::Form &form);

private:
	int fieldType;
	QString fieldName;

};

#endif
