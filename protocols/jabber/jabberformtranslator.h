 /*
  * jabberformtranslator.h
  * 
  * Copyright (c) 2002 by the Kopete Developers <kopete-devel@kde.org>
  * 
  * Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>
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

#include "types.h"
#include "tasks.h"

/**
  *@author Kopete developers
  */

class JabberFormTranslator:public QWidget
{

  Q_OBJECT public:
	  JabberFormTranslator (QWidget * parent = 0, const char *name = 0);
	 ~JabberFormTranslator ();

	void translate (const Jabber::Form & form, QWidget * widget);
	  Jabber::Form & resultData ();

	  signals:void gatherData (Jabber::Form & form);

  private:
	  Jabber::Form privForm;

};

#endif
