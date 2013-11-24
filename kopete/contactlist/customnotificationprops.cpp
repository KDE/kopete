/*
    customnotificationprops.cpp

    Kopete Contactlist Custom Notifications GUI for Groups and MetaContacts

    Contains UI controller logic for managing custom notifications

    Copyright (c) 2004 Will Stephenson <wstephenson@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "customnotificationprops.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kurlrequester.h>

#include <knotifyconfigwidget.h>

#include <knotification.h>

#include <QMetaEnum>

CustomNotificationProps::CustomNotificationProps( QWidget *parent, const QPair<QString,QString> &context, const char * name )
	: QObject( parent ) , m_item(context)
{
	setObjectName( name );
	m_notifyWidget = new KNotifyConfigWidget( parent );
	m_notifyWidget->setApplication(QString() , m_item.first , m_item.second);
}



void CustomNotificationProps::dumpData()
{
}

void CustomNotificationProps::resetEventWidgets()
{
}

void CustomNotificationProps::storeCurrentCustoms()
{
	m_notifyWidget->save();
}


QWidget* CustomNotificationProps::widget()
{
	return m_notifyWidget;
}

#include "customnotificationprops.moc"
