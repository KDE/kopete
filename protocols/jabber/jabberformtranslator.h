 /*
  * jabberformtranslator.h
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

#ifndef JABBERFORMTRANSLATOR_H
#define JABBERFORMTRANSLATOR_H

#include <qwidget.h>
#include <qlayout.h>

#include "xmpp_tasks.h"

/**
  *@author Till Gerken <till@tantalo.net>
  */

class JabberFormTranslator:public QWidget
{

Q_OBJECT

public:
	explicit JabberFormTranslator (const XMPP::Form & form, QWidget * parent = 0);
	~JabberFormTranslator ();

	XMPP::Form & resultData ();

signals:
	void gatherData (XMPP::Form & form);

private:
	XMPP::Form emptyForm, privForm;

};

#endif
