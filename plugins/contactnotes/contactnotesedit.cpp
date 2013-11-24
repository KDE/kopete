/*
    contactnotesedit.cpp  -  description

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

#include "contactnotesedit.h"

#include <qlabel.h>
#include <QTextEdit>


#include <klocale.h>
#include <kvbox.h>

#include "kopetemetacontact.h"

#include "contactnotesplugin.h"

ContactNotesEdit::ContactNotesEdit(Kopete::MetaContact *m,ContactNotesPlugin *p) 
 : KDialog()
{
	setCaption( i18n("Contact Notes") );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );

	m_plugin=p;
	m_metaContact=m;

	KVBox *w=new KVBox(this);
	w->setSpacing(KDialog::spacingHint());
	m_label = new QLabel(i18n("Notes about %1:", m->displayName()) , w );
	m_label->setObjectName( QLatin1String("m_label") );
	m_linesEdit= new QTextEdit ( w );

	m_linesEdit->setText(p->notes(m));

	showButtonSeparator(true);
	setMainWidget(w);
}

ContactNotesEdit::~ContactNotesEdit()
{
}

void ContactNotesEdit::slotButtonClicked(int buttonCode)
{
	KDialog::slotButtonClicked(buttonCode);
	if( buttonCode == KDialog::Ok )
		emit notesChanged(m_linesEdit->toPlainText(),m_metaContact) ;
}

#include "contactnotesedit.moc"
