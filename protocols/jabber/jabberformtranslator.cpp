/***************************************************************************
                          jabberformtranslator.cpp  -  description
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002 by Till Gerken
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qlabel.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <qwidget.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kdebug.h>
 
#include <psi/types.h>
#include <psi/tasks.h>

#include "jabberformtranslator.h"

JabberFormTranslator::JabberFormTranslator()
{
}

void JabberFormTranslator::translate(const Jabber::Form &form, QLayout *layout, QWidget *parent)
{

	// add instructions
	QVBoxLayout *innerLayout = new QVBoxLayout(layout);
	
	QLabel *label = new QLabel(form.instructions(), parent, "InstructionLabel");
	innerLayout->addWidget(label, 0, 0);
	label->show();

	QGridLayout *formLayout = new QGridLayout(innerLayout);
	
	int row = 1;
	for(Jabber::Form::const_iterator it = form.begin(); it != form.end(); it++)
	{
		kdDebug() << "[JabberFormTranslator] Adding field realName()==" << (*it).realName() << ", fieldName()==" << (*it).fieldName() << " to the dialog" << endl;

		label = new QLabel((*it).fieldName(), parent, (*it).fieldName());
		formLayout->addWidget(label, row, 0);
		label->show();

		QLineEdit *edit = new QLineEdit((*it).value(), parent);
		formLayout->addWidget(edit, row, 1);
		edit->show();

		row++;
	}

}

JabberFormTranslator::~JabberFormTranslator()
{
}
