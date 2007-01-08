 /*
  * jabberformlineedit.cpp
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

#include "jabberformlineedit.h"

JabberFormLineEdit::JabberFormLineEdit (const int type, const QString & realName, const QString & value, QWidget * parent) : KLineEdit (value, parent)
{

	fieldType = type;
	fieldName = realName;

}

void JabberFormLineEdit::slotGatherData (XMPP::Form & form)
{

	form += XMPP::FormField (fieldName, text ());

}

JabberFormLineEdit::~JabberFormLineEdit ()
{
}

#include "jabberformlineedit.moc"
