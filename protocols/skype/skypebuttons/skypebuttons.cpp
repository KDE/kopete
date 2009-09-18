/*
    skypebuttons.cpp - Skype Action Handler for Kopete

    Copyright (c) 2009 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <QtGui>
#include <qtbrowserplugin.h>

/**
 * @author Pali Rohár
 * This NS plugin only register application/x-skype mime type to web browser for using Skype Action Handler
 */
class SkypeButtons : public QWidget
{
	Q_OBJECT
	Q_CLASSINFO("MIME", "application/x-skype::Skype Buttons")
};

#include "skypebuttons.moc"

QTNPFACTORY_BEGIN("Skype Buttons for Kopete", "Mime Type x-skype for Skype Buttons")
	QTNPCLASS(SkypeButtons)
QTNPFACTORY_END()
