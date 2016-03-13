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

#include "jabberformtranslator.h"
#include <qlabel.h>
#include <QVBoxLayout>
#include <QGridLayout>

#include <kdebug.h>

#include "jabberformlineedit.h"

JabberFormTranslator::JabberFormTranslator (const XMPP::Form & form, QWidget * parent) : QWidget (parent)
{
	/* Copy basic form values. */
	privForm.setJid (form.jid ());
	privForm.setInstructions (form.instructions ());
	privForm.setKey (form.key ());

	emptyForm = privForm;

	/* Add instructions to layout. */
	QVBoxLayout *innerLayout = new QVBoxLayout (this);
	setLayout (innerLayout);
	innerLayout->setSpacing (4);

	QLabel *label = new QLabel (form.instructions (), this);
	label->setWordWrap (true);
	label->setAlignment (Qt::AlignVCenter);
	label->setSizePolicy (QSizePolicy::Minimum,QSizePolicy::Fixed);
	label->show ();

	innerLayout->addWidget (label);

	QGridLayout *formLayout = new QGridLayout (this);
	innerLayout->addLayout (formLayout);

	int row = 1;
	XMPP::Form::const_iterator formEnd = form.end ();
	for (XMPP::Form::const_iterator it = form.begin (); it != formEnd; ++it, ++row)
	{
		kDebug (14130) << "[JabberFormTranslator] Adding field realName()==" <<
			(*it).realName () << ", fieldName()==" << (*it).fieldName () << " to the dialog" << endl;

		label = new QLabel ((*it).fieldName (), this);
		formLayout->addWidget (label, row, 0);
		label->show ();

		KLineEdit *edit;
        edit = new JabberFormLineEdit ((*it).type (), (*it).realName (),
                                         (*it).value (), this);
		if ((*it).type() == XMPP::FormField::password)
            edit->setPasswordMode(true);

		formLayout->addWidget (edit, row, 1);
		edit->show ();

		connect (this, SIGNAL (gatherData(XMPP::Form&)), edit, SLOT (slotGatherData(XMPP::Form&)));
	}

	innerLayout->addStretch ();
}

XMPP::Form & JabberFormTranslator::resultData ()
{
	// clear form data
	privForm = emptyForm;

	// let all line edit fields write into our form
	emit gatherData (privForm);

	return privForm;
}

JabberFormTranslator::~JabberFormTranslator ()
{
}

#include "jabberformtranslator.moc"
