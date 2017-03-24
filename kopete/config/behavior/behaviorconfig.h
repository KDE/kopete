/*
    behaviorconfig.h  -  Kopete Look Feel Config

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef BEHAVIORCONFIG_H
#define BEHAVIORCONFIG_H

#include <kcmodule.h>
#include <kplugininfo.h>

class QTabWidget;

class BehaviorConfig_General;
class BehaviorConfig_Events;
class BehaviorConfig_Chat;
class BehaviorConfig_Away;

class BehaviorConfig : public KCModule
{
	Q_OBJECT

	public:
		BehaviorConfig(QWidget *parent, const QVariantList &args) ;

		void save() Q_DECL_OVERRIDE;
		void load() Q_DECL_OVERRIDE;

	private slots:
		void slotSettingsChanged(bool);
		void slotValueChanged(int);
		void slotTextChanged();

	private:
		QTabWidget* mBehaviorTabCtl;

		BehaviorConfig_General *mPrfsGeneral;
		BehaviorConfig_Events *mPrfsEvents;
		BehaviorConfig_Chat *mPrfsChat;
		BehaviorConfig_Away *mPrfsAway;

		QList<KPluginInfo> viewPlugins;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
