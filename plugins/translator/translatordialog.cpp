/*
    translatordialog.cpp  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "translatordialog.h"

#include <klocale.h>
#include <ktextedit.h>


TranslatorDialog::TranslatorDialog(const QString &text, QWidget *parent)
	: KDialog(parent)
{
	setCaption( i18n("Translator Plugin") );
	setButtons( KDialog::Ok );

	m_textEdit=new KTextEdit(this);
	setMainWidget(m_textEdit);
	m_textEdit->setText(text);
}
TranslatorDialog::~TranslatorDialog()
{
}

QString TranslatorDialog::translatedText()
{
	return m_textEdit->toPlainText();
}
/*void TranslatorDialog::slotFinished()
{
	emit finished(m_textEdit->text());
}  */


#include "translatordialog.moc"
