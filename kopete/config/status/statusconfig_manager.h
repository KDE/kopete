/*
    statusconfig_manager.h

    Copyright (c) 2008       by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2008       by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef STATUSCONFIG_MANAGER_H
#define STATUSCONFIG_MANAGER_H

#include "ui_statusconfig_manager.h"

class StatusConfig_Manager : public QWidget, private Ui::StatusConfig_Manager
{
	Q_OBJECT

public:
	StatusConfig_Manager( QWidget *parent = 0 );
	~StatusConfig_Manager();

public slots:
	void load();
	void save();

	void addStatus();
	void addGroup();
	void removeStatus();

signals:
	void changed();

private slots:
	void currentRowChanged( const QModelIndex &current, const QModelIndex &previous );
	
	void editTitleEdited( const QString &text );
	void editMessageChanged();
	void editTypeChanged( int index );

private:
	class Private;
	Private * const d;
};

#endif // STATUSCONFIG_MANAGER_H
