
/*
    msneditaccountwidget.h - Jabber Account Widget

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



#ifndef JabberEDITACCOUNTWIDEGET_H
#define JabberEDITACCOUNTWIDEGET_H

#include <qwidget.h>
#include "editaccountwidget.h"
#include "jabberaccount.h"
#include "dlgjabbereditaccountwidget.h"
#include "jabberprotocol.h"

/**
  *@author Olivier Goffart <ogoffart@tiscalinet.be>
  */

class JabberProtocol;
class QCheckBox;
class QLineEdit;

class JabberEditAccountWidget:public DlgJabberEditAccountWidget, public EditAccountWidget
{
  Q_OBJECT public:
	  JabberEditAccountWidget (JabberProtocol * proto, JabberAccount *, QWidget * parent = 0, const char *name = 0);
	 ~JabberEditAccountWidget ();
	virtual bool validateData ();
	virtual KopeteAccount *apply ();
	bool settings_changed;

  private:
	void reopen ();
	void writeConfig ();
	JabberProtocol *m_protocol;

	public slots:virtual void registerClicked ();
	virtual void sslToggled (bool);

	private slots:void configChanged ();
};

#endif
