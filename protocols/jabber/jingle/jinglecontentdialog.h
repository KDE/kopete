 /*
  * jinglecontentdialog.h - A dialog which asks the user to accept contents.
  *
  * This class is a dialog asking what contents the user accepts for an
  * incoming session.
  * This class could also be used to ask the user what contents he wants
  * to propose in the session-initiate jingle action
  *
  * Copyright (c) 2008 by Detlev Casanova <detlev.casanova@gmail.com>
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
#ifndef CONTENT_DIALOG_H
#define CONTENT_DIALOG_H
#include "ui_jinglecontentdialog.h"

#include "jingletasks.h"

#include <QCheckBox>

class JingleContentDialog : public QDialog
{
	Q_OBJECT
public:
	JingleContentDialog(QWidget* parent = 0);
	~JingleContentDialog();
	void setContents(QList<XMPP::JingleContent*> c);
	void setSession(XMPP::JingleSession *s);
	XMPP::JingleSession *session();
	QStringList checked();
	QStringList unChecked();

private:
	Ui::jingleContentDialog ui;
	XMPP::JingleSession *m_session;
	QList<QCheckBox*> m_checkBoxes;
	QStringList m_contentNames;
};

#endif
