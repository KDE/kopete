 /*
  * jabberformtranslator.cpp
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

#include <qlabel.h>

#include <kdebug.h>

#include "jabberformlineedit.h"
#include "jabberformtranslator.h"

JabberFormTranslator::JabberFormTranslator (QWidget * parent, const char *name):QWidget (parent, name)
{
}

void JabberFormTranslator::translate (const Jabber::Form & form, QWidget * widget)
{
	QVBoxLayout *innerLayout;

	/* Copy basic form values. */
	privForm.setJid (form.jid ());
	privForm.setInstructions (form.instructions ());
	privForm.setKey (form.key ());

	/* Add instructions to layout. */
	if (widget->layout ())
		innerLayout = new QVBoxLayout (widget->layout (), 4);
	else
		innerLayout = new QVBoxLayout (widget, 0, 4);


	QLabel *label = new QLabel (form.instructions (), parentWidget (), "InstructionLabel");
	label->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed, true);
	innerLayout->addWidget (label, 0, 0);
	label->setAlignment (int (QLabel::WordBreak | QLabel::AlignVCenter));

	label->show ();

	QGridLayout *formLayout = new QGridLayout (innerLayout, form.count (), 2);

	int row = 1;

	for (Jabber::Form::const_iterator it = form.begin (); it != form.end (); it++)
	{
		kdDebug (14130) << "[JabberFormTranslator] Adding field realName()==" <<
			(*it).realName () << ", fieldName()==" << (*it).fieldName () << " to the dialog" << endl;

		label = new QLabel ((*it).fieldName (), parentWidget (), (*it).fieldName ().latin1 ());
		formLayout->addWidget (label, row, 0);
		label->show ();

		JabberFormLineEdit *edit = new JabberFormLineEdit ((*it).type (), (*it).realName (),
														   (*it).value (), parentWidget ());

		formLayout->addWidget (edit, row, 1);
		edit->show ();

		connect (this, SIGNAL (gatherData (Jabber::Form &)), edit, SLOT (slotGatherData (Jabber::Form &)));

		row++;
	}

	innerLayout->addStretch ();
}

Jabber::Form & JabberFormTranslator::resultData ()
{
	/* Let all line edit fields write into our form. */
	emit gatherData (privForm);

	return privForm;
}

JabberFormTranslator::~JabberFormTranslator ()
{
}

#include "jabberformtranslator.moc"
