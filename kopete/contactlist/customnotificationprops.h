/*
    customnotificationprops.h

    Kopete Contactlist Custom Notifications GUI for Groups and MetaContacts

    Copyright (c) 2004 Will Stephenson <lists@stevello.free-online.co.uk>
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

#ifndef KOPETE_CUSTOM_NOTIFICATION_PROPS_H
#define KOPETE_CUSTOM_NOTIFICATION_PROPS_H

class CustomNotificationWidget;
class QBoxLayout;

namespace Kopete
{
class NotifyDataObject;
}

class CustomNotificationProps : public QObject
{
	Q_OBJECT
public:
	CustomNotificationProps( QWidget *parent, Kopete::NotifyDataObject* item, const char * name = 0 );
	~CustomNotificationProps() {}
	void dumpData();
	void resetEventWidgets();
	void storeCurrentCustoms();
	CustomNotificationWidget* widget();

protected slots:
	void slotEventsComboChanged( int itemNo );

private:
	CustomNotificationWidget* m_notifyWidget;
	Kopete::NotifyDataObject * m_item;
	QStringList m_eventList;
	QString m_event;
};

#endif
