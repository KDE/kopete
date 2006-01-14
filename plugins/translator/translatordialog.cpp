/***************************************************************************
                          translatordialog.cpp  -  description
                             -------------------
    begin                : sam oct 19 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <ktextedit.h>
 
#include "translatordialog.h"


TranslatorDialog::TranslatorDialog(const QString &text,QWidget *parent, const char *name ) : KDialogBase(parent,name,true,i18n("Translator Plugin"), Ok)
{
	m_textEdit=new KTextEdit(this);
	setMainWidget(m_textEdit);
	m_textEdit->setText(text);
}
TranslatorDialog::~TranslatorDialog()
{
}

QString TranslatorDialog::translatedText()
{
	return m_textEdit->text();
}
/*void TranslatorDialog::slotFinished()
{
	emit finished(m_textEdit->text());
}  */


#include "translatordialog.moc"
