/*
    msneditaccountwidget.h - MSN Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/



#ifndef MSNEDITACCOUNTWIDEGET_H
#define MSNEDITACCOUNTWIDEGET_H

#include "editaccountwidget.h"

#include "msneditaccountui.h"


/**
  *@author Olivier Goffart <ogoffart@tiscalinet.be>
  */

class MSNProtocol;
class KAutoConfig;

class MSNEditAccountWidget : public MSNEditAccountUI, public EditAccountWidget
{
	Q_OBJECT

	public:
		MSNEditAccountWidget(MSNProtocol *proto, KopeteAccount *, QWidget *parent=0, const char *name=0);
		~MSNEditAccountWidget();
		virtual bool validateData();
		virtual KopeteAccount *apply();

	private:
		MSNProtocol *m_protocol;
		KAutoConfig *kautoconfig;

	private slots:
		void slotAllow();
		void slotBlock();
		void slotShowReverseList();
		void slotSelectImage();
};


#endif
