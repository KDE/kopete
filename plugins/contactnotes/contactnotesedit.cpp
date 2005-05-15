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
#include <qtextedit.h>
#include <qvbox.h>

#include <klocale.h>

#include "kopetemetacontact.h"

#include "contactnotesplugin.h"
#include "contactnotesedit.h"

ContactNotesEdit::ContactNotesEdit(Kopete::MetaContact *m,ContactNotesPlugin *p,const char *name) : KDialogBase(0L, name , false, i18n("Contact Notes") , KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok)
{
	m_plugin=p;
	m_metaContact=m;

	QVBox *w=new QVBox(this);
	w->setSpacing(KDialog::spacingHint());
	m_label = new QLabel(i18n("Notes about %1:").arg(m->displayName()) , w , "m_label");
	m_linesEdit= new QTextEdit ( w , "m_linesEdit");

	m_linesEdit->setText(p->notes(m));

	enableButtonSeparator(true);
	setMainWidget(w);
}

ContactNotesEdit::~ContactNotesEdit()
{
}

void ContactNotesEdit::slotOk()
{
	emit notesChanged(m_linesEdit->text(),m_metaContact) ;
	KDialogBase::slotOk();
}

#include "contactnotesedit.moc"
