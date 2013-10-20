/*
    statusconfig.h - Kopete Status Config

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef STATUSCONFIG_H
#define STATUSCONFIG_H

#include <kcmodule.h>

class QTabWidget;

class StatusConfig_Manager;
class StatusConfig_General;

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/
class StatusConfig : public KCModule
{
	Q_OBJECT

	public:
		StatusConfig( QWidget *parent, const QVariantList &args );

	public slots:
		virtual void save();
		virtual void load();

	private:
		QTabWidget* mStatusTabCtl;

		StatusConfig_Manager *mPrfsManager;
		StatusConfig_General *mPrfsGeneral;
};

#endif
