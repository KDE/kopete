/***************************************************************************
                          jabberformtranslator.h  -  description
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

#ifndef JABBERFORMTRANSLATOR_H
#define JABBERFORMTRANSLATOR_H

#include <qlayout.h>

#include <psi/types.h>
#include <psi/tasks.h>

/**
  *@author Kopete developers
  */

class JabberFormTranslator
{

	public: 
		JabberFormTranslator();
		~JabberFormTranslator();

		static void translate(const Jabber::Form &form, QLayout *layout, QWidget *parent);

};

#endif
