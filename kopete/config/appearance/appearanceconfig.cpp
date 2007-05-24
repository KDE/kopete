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
#include "appearanceconfig_emoticons.h"
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
#include <kconfig.h> // for KNewStuff emoticon fetching
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

#ifdef __GNUC__
#warning "Port KNS changes!"
#endif
#if 0
#include <knewstuff/downloaddialog.h> // knewstuff emoticon and chatwindow fetching
#include <knewstuff/engine.h>         // "
#include <knewstuff/entry.h>          // "
#include <knewstuff/knewstuff.h>      // "
#include <knewstuff/provider.h>       // "
#endif


#include "kopeteemoticons.h"
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
	 : mAppearanceTabCtl(0L), mPrfsEmoticons(0L),
	   mPrfsColors(0L), mPrfsContactList(0L)
	{}

	QTabWidget *mAppearanceTabCtl;

	AppearanceConfig_Emoticons *mPrfsEmoticons;
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

	// "Emoticons" TAB ==========================================================
	d->mPrfsEmoticons = new AppearanceConfig_Emoticons(d->mAppearanceTabCtl);
	addConfig( Kopete::AppearanceSettings::self(), d->mPrfsEmoticons );

	connect(d->mPrfsEmoticons->icon_theme_list, SIGNAL(selectionChanged()),
		this, SLOT(slotSelectedEmoticonsThemeChanged()));
	connect(d->mPrfsEmoticons->btnInstallTheme, SIGNAL(clicked()),
		this, SLOT(installEmoticonTheme()));

	connect(d->mPrfsEmoticons->btnGetThemes, SIGNAL(clicked()),
		this, SLOT(slotGetEmoticonThemes()));
	connect(d->mPrfsEmoticons->btnRemoveTheme, SIGNAL(clicked()),
		this, SLOT(removeSelectedEmoticonTheme()));

	d->mAppearanceTabCtl->addTab(d->mPrfsEmoticons, i18n("&Emoticons"));

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

void AppearanceConfig::updateEmoticonsButton(bool _b)
{
    QString themeName = d->mPrfsEmoticons->icon_theme_list->currentText();
    QFileInfo fileInf(KGlobal::dirs()->findResource("emoticons", themeName+'/'));
    d->mPrfsEmoticons->btnRemoveTheme->setEnabled( _b && fileInf.isWritable());
    d->mPrfsEmoticons->btnGetThemes->setEnabled( false );
}

void AppearanceConfig::save()
{
	KCModule::save();
//	kDebug(14000) << k_funcinfo << "called." << endl;

	Kopete::AppearanceSettings *settings = Kopete::AppearanceSettings::self();

	settings->setEmoticonTheme( d->mPrfsEmoticons->icon_theme_list->currentText() );

	settings->writeConfig();

	load();
}

void AppearanceConfig::load()
{
	KCModule::load();


	// Look for available emoticons themes
	updateEmoticonlist();

//	kDebug(14000) << k_funcinfo << "called" << endl;
}


void AppearanceConfig::updateEmoticonlist()
{
	KStandardDirs dir;

	d->mPrfsEmoticons->icon_theme_list->clear(); // Wipe out old list
	// Get a list of directories in our icon theme dir
	QStringList themeDirs = KGlobal::dirs()->findDirs("emoticons", "");
	// loop adding themes from all dirs into theme-list
	for( int x = 0;x < themeDirs.count();x++)
	{
		QDir themeQDir(themeDirs[x]);
		themeQDir.setFilter( QDir::Dirs ); // only scan for subdirs
		themeQDir.setSorting( QDir::Name ); // I guess name is as good as any
		for(unsigned int y = 0; y < themeQDir.count(); y++)
		{
			QStringList themes = themeQDir.entryList(QDir::Dirs, QDir::Name);
			// We don't care for '.' and '..'
			if ( themeQDir[y] != "." && themeQDir[y] != ".." )
			{
				// Add ourselves to the list, using our directory name  FIXME:  use the first emoticon of the theme.
				QPixmap previewPixmap = QPixmap(KStandardDirs::locate("emoticons", themeQDir[y]+"/smile.png"));
				d->mPrfsEmoticons->icon_theme_list->insertItem(previewPixmap,themeQDir[y]);
			}
		}
	}

	// Where is that theme in our big-list-o-themes?

	Q3ListBoxItem *item = d->mPrfsEmoticons->icon_theme_list->findItem( Kopete::AppearanceSettings::self()->emoticonTheme() );

	if (item) // found it... make it the currently selected theme
		d->mPrfsEmoticons->icon_theme_list->setCurrentItem( item );
	else // Er, it's not there... select the current item
		d->mPrfsEmoticons->icon_theme_list->setCurrentItem( 0 );
}

void AppearanceConfig::slotSelectedEmoticonsThemeChanged()
{
	QString themeName = d->mPrfsEmoticons->icon_theme_list->currentText();
	QFileInfo fileInf(KGlobal::dirs()->findResource("emoticons", themeName+'/'));
	d->mPrfsEmoticons->btnRemoveTheme->setEnabled( fileInf.isWritable() );

	Kopete::Emoticons emoticons( themeName );
	QStringList smileys = emoticons.emoticonAndPicList().keys();
	QString newContentText = "<qt>";

	for(QStringList::Iterator it = smileys.begin(); it != smileys.end(); ++it )
		newContentText += QString::fromLatin1("<img src=\"%1\"> ").arg(*it);

	newContentText += QLatin1String("</qt>");
	d->mPrfsEmoticons->icon_theme_preview->setHtml(newContentText);
	emitChanged();
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

void AppearanceConfig::installEmoticonTheme()
{
	KUrl themeURL = KUrlRequesterDialog::getUrl(QString::null, this,
			i18n("Drag or Type Emoticon Theme URL"));
	if ( themeURL.isEmpty() )
		return;

	//TODO: support remote theme files!
	if ( !themeURL.isLocalFile() )
	{
		KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("Sorry, emoticon themes must be installed from local files."),
		                               i18n("Could Not Install Emoticon Theme") );
		return;
	}

	Kopete::Global::installEmoticonTheme( themeURL.path() );
	updateEmoticonlist();
}

void AppearanceConfig::removeSelectedEmoticonTheme()
{
	Q3ListBoxItem *selected = d->mPrfsEmoticons->icon_theme_list->selectedItem();
	if(selected==0)
		return;

	QString themeName = selected->text();

	QString question=i18n("<qt>Are you sure you want to remove the "
			"<strong>%1</strong> emoticon theme?<br>"
			"<br>"
			"This will delete the files installed by this theme.</qt>",
		themeName);

        int res = KMessageBox::warningContinueCancel(this, question, i18n("Confirmation"),KStandardGuiItem::del());
	if (res!=KMessageBox::Continue)
		return;

	KUrl themeUrl(KGlobal::dirs()->findResource("emoticons", themeName+'/'));
	KIO::NetAccess::del(themeUrl, this);

	updateEmoticonlist();
}

void AppearanceConfig::slotGetEmoticonThemes()
{
	KConfigGroup config(KGlobal::config(), "KNewStuff");
	config.writeEntry( "ProvidersUrl",
						"http://download.kde.org/khotnewstuff/emoticons-providers.xml" );
	config.writeEntry( "StandardResource", "emoticons" );
	config.writeEntry( "Uncompress", "application/x-gzip" );
	config.sync();

#ifdef __GNUC__
#warning "Port KNS changes!"
#endif
#if 0
	KNS::DownloadDialog::open( "emoticons", i18n( "Get New Emoticons") );
#endif

	updateEmoticonlist();
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
