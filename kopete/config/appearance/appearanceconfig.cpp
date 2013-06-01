/*
    appearanceconfig.cpp  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2005-2006 by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "appearanceconfig.h"
#include "ui_appearanceconfig_colors.h"
#include "ui_appearanceconfig_contactlist.h"
#include "ui_appearanceconfig_advanced.h"

#include "tooltipeditdialog.h"

#include <QCheckBox>
#include <QDir>
#include <QLayout>
#include <QSpinBox>
#include <QSlider>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

#include <kdeversion.h>
#include <kinputdialog.h>

#include <kcolorcombo.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kfontrequester.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kio/netaccess.h>
#include <khtmlview.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kurlrequesterdialog.h>
#include <krun.h>
#include <kfiledialog.h>

#include "kopeteglobal.h"

#include <qtabwidget.h>

#include "kopeteappearancesettings.h"
#include "contactlistlayoutwidget.h"

//class AppearanceConfig;

K_PLUGIN_FACTORY( KopeteAppearanceConfigFactory,
		registerPlugin<AppearanceConfig>(); )
K_EXPORT_PLUGIN( KopeteAppearanceConfigFactory("kcm_kopete_appearanceconfig") )

class FakeProtocol;
class FakeAccount;
class FakeContact;

class AppearanceConfig::Private
{
public:
	Private()
	 : mAppearanceTabCtl(0L)
	{}

	QTabWidget *mAppearanceTabCtl;

	Ui::AppearanceConfig_Colors mPrfsColors;
	Ui::AppearanceConfig_ContactList mPrfsContactList;
	Ui::AppearanceConfig_Advanced mPrfsAdvanced;
	ContactListLayoutWidget *contactListLayoutWidget;
};


AppearanceConfig::AppearanceConfig(QWidget *parent, const QVariantList &args )
: KCModule( KopeteAppearanceConfigFactory::componentData(), parent, args ), d(new Private())
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	// since the tab widget is already within a layout with margins in the KSettings::Dialog
	// it needs no margins of its own.
	layout->setContentsMargins( 0, 0, 0, 0 );
	d->mAppearanceTabCtl = new QTabWidget(this);
	d->mAppearanceTabCtl->setObjectName("mAppearanceTabCtl");
	layout->addWidget( d->mAppearanceTabCtl );

	KConfigGroup config(KGlobal::config(), "ChatWindowSettings");

	// "Contact List" TAB =======================================================
	QWidget *contactListWidget = new QWidget(d->mAppearanceTabCtl);
	d->mPrfsContactList.setupUi(contactListWidget);
	addConfig( Kopete::AppearanceSettings::self(), contactListWidget );

	connect(d->mPrfsContactList.mEditTooltips, SIGNAL(clicked()),
		this, SLOT(slotEditTooltips()));

	d->mAppearanceTabCtl->addTab(contactListWidget, i18n("Contact List"));

	// "Colors and Fonts" TAB ===================================================
	QWidget *colorsWidget = new QWidget(d->mAppearanceTabCtl);
	d->mPrfsColors.setupUi(colorsWidget);
	addConfig( Kopete::AppearanceSettings::self(), colorsWidget );

	d->mAppearanceTabCtl->addTab(colorsWidget, i18n("Colors && Fonts"));

	// "Advanced" TAB ===========================================================
	QWidget *advancedWidget = new QWidget(d->mAppearanceTabCtl);
	d->mPrfsAdvanced.setupUi(advancedWidget);
	addConfig( Kopete::AppearanceSettings::self(), advancedWidget );
	connect ( d->mPrfsAdvanced.kcfg_contactListResizeAnchor, SIGNAL (toggled(bool)), this, SLOT (emitChanged()));

	d->mAppearanceTabCtl->addTab(advancedWidget, i18n("Advanced"));

	
	d->contactListLayoutWidget = new ContactListLayoutWidget( d->mAppearanceTabCtl );
	connect( d->contactListLayoutWidget, SIGNAL(changed()), this, SLOT (emitChanged()) );
	d->mAppearanceTabCtl->addTab( d->contactListLayoutWidget, i18n("Layout") );

	// ==========================================================================

	load();
}

AppearanceConfig::~AppearanceConfig()
{
	delete d;
}

void AppearanceConfig::save()
{
	KCModule::save();
//	kDebug(14000) << "called.";

	Kopete::AppearanceSettings *settings = Kopete::AppearanceSettings::self();
	settings->setContactListAutoResize (d->mPrfsAdvanced.kcfg_contactListResizeAnchor->isChecked());
	settings->writeConfig();

	if ( d->contactListLayoutWidget->save() )
		load();
	else
		QTimer::singleShot( 0, this, SLOT(emitChanged()) );
}

void AppearanceConfig::load()
{
	KCModule::load();
	d->mPrfsAdvanced.kcfg_contactListResizeAnchor->setChecked(Kopete::AppearanceSettings::contactListAutoResize ());

	d->contactListLayoutWidget->load();
//	kDebug(14000) << "called";
}

void AppearanceConfig::slotHighlightChanged()
{
//	bool value = mPrfsChatWindow->highlightEnabled->isChecked();
//	mPrfsChatWindow->foregroundColor->setEnabled ( value );
//	mPrfsChatWindow->backgroundColor->setEnabled ( value );
//	slotUpdateChatPreview();
}

void AppearanceConfig::slotChangeFont()
{
	emitChanged();
}

void AppearanceConfig::emitChanged()
{
	emit changed( true );
}

void AppearanceConfig::slotEditTooltips()
{
	QPointer <TooltipEditDialog> dlg = new TooltipEditDialog(this);
	connect(dlg, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
	dlg->exec();
	delete dlg;
}

#include "appearanceconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
