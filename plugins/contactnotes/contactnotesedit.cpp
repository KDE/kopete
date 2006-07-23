/***************************************************************************
                          contactnotesedit.cpp  -  description
                             -------------------
    begin                : lun sep 16 2002
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

#include <qlabel.h>
#include <q3textedit.h>


#include <klocale.h>
#include <kvbox.h>

#include "kopetemetacontact.h"

#include "contactnotesplugin.h"
#include "contactnotesedit.h"

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
	m_linesEdit= new Q3TextEdit ( w , "m_linesEdit");

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
		emit notesChanged(m_linesEdit->text(),m_metaContact) ;
}

#include "contactnotesedit.moc"
