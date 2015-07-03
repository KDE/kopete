/*
    Kopete Groupwise Protocol
    gwcontactproperties.h - dialog showing a contact's server side properties

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GWCONTACTPROPERTIES_H
#define GWCONTACTPROPERTIES_H

#include <QHash>
#include <QObject>
#include "gwcontact.h"
#include "ui_gwcontactprops.h"

namespace Ui { class GroupWiseContactPropsWidget; }
class KDialog;
class KAction;

/**
Logic, wrapping UI, for displaying contact properties

@author SUSE AG
*/
class GroupWiseContactProperties : public QObject
{
Q_OBJECT
public:
	/**
	 * Display properties given a GroupWiseContact
	 */ 
	GroupWiseContactProperties( GroupWiseContact * contact, QWidget *parent );
	/**
	 * Display properties given a GroupWise::ContactDetails
	 */
	explicit GroupWiseContactProperties( GroupWise::ContactDetails contactDetails, QWidget *parent = 0 );
	~GroupWiseContactProperties();
protected:
	void setupProperties( QMap< QString, QVariant > serverProps );
	void init();
protected slots:
	void copy();
private:
	Ui::GroupWiseContactProps m_ui;
	KAction * m_copyAction;
	KDialog * m_dialog;
};

#endif // GWCONTACTPROPERTIES_H
