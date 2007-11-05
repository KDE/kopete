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
#include "appearanceconfig_colors.h"
#include "appearanceconfig_contactlist.h"

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
#include <kgenericfactory.h>
#include <kio/netaccess.h>
#include <khtmlview.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kurl.h> // KNewStuff
#include <kurlrequesterdialog.h>
#include <krun.h>
#include <kfiledialog.h>

#include "kopeteglobal.h"

#include <qtabwidget.h>

#include "kopeteappearancesettings.h"

typedef KGenericFactory<AppearanceConfig, QWidget> KopeteAppearanceConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_appearanceconfig, KopeteAppearanceConfigFactory( "kcm_kopete_appearanceconfig" ) )

class FakeProtocol;
class FakeAccount;
class FakeContact;

class AppearanceConfig::Private
{
public:
	Private()
	 : mAppearanceTabCtl(0L), mPrfsColors(0L), mPrfsContactList(0L)
	{}

	QTabWidget *mAppearanceTabCtl;

	AppearanceConfig_Colors *mPrfsColors;
	AppearanceConfig_ContactList *mPrfsContactList;
};


AppearanceConfig::AppearanceConfig(QWidget *parent, const QStringList &args )
: KCModule( KopeteAppearanceConfigFactory::componentData(), parent, args )
{
	d = new Private;

	QVBoxLayout *layout = new QVBoxLayout(this);

	d->mAppearanceTabCtl = new QTabWidget(this);
	d->mAppearanceTabCtl->setObjectName("mAppearanceTabCtl");
	layout->addWidget( d->mAppearanceTabCtl );

	KConfigGroup config(KGlobal::config(), "ChatWindowSettings");

	// "Contact List" TAB =======================================================
	d->mPrfsContactList = new AppearanceConfig_ContactList(d->mAppearanceTabCtl);
	addConfig( Kopete::AppearanceSettings::self(), d->mPrfsContactList );

	connect(d->mPrfsContactList->mEditTooltips, SIGNAL(clicked()),
		this, SLOT(slotEditTooltips()));

	d->mAppearanceTabCtl->addTab(d->mPrfsContactList, i18n("Contact List"));

	// "Colors and Fonts" TAB ===================================================
	d->mPrfsColors = new AppearanceConfig_Colors(d->mAppearanceTabCtl);
	addConfig( Kopete::AppearanceSettings::self(), d->mPrfsColors );

	d->mAppearanceTabCtl->addTab(d->mPrfsColors, i18n("Colors && Fonts"));

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

	settings->writeConfig();

	load();
}

void AppearanceConfig::load()
{
	KCModule::load();

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
	TooltipEditDialog *dlg = new TooltipEditDialog(this);
	connect(dlg, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
	dlg->exec();
	delete dlg;
}

#include "appearanceconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
