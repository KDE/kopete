/*
    appearanceconfig.cpp  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2005-2006 by Michaël Larouche       <michael.larouche@kdemail.net>

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "appearanceconfig.h"
#include "appearanceconfig_emoticons.h"
#include "appearanceconfig_chatwindow.h"
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

#include <kapplication.h>
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
#include <kurlrequesterdlg.h>
#include <krun.h>
#include <kfiledialog.h>

#include <knewstuff/downloaddialog.h> // knewstuff emoticon and chatwindow fetching
#include <knewstuff/engine.h>         // "
#include <knewstuff/entry.h>          // "
#include <knewstuff/knewstuff.h>      // "
#include <knewstuff/provider.h>       // "

// For Kopete Chat Window Style configuration and preview.
#include <kopetechatwindowstylemanager.h>
#include <kopetechatwindowstyle.h>
#include <chatmessagepart.h>


// Things we fake to get the message preview to work
#include <kopeteprotocol.h>
#include <kopetemetacontact.h>
#include <kopeteaccount.h>
#include <kopetecontact.h>
#include <kopetemessage.h>
#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>
#include <kopetestatusmessage.h>

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
	 : mAppearanceTabCtl(0L), preview(0L), mPrfsEmoticons(0L),mPrfsChatWindow(0L),
	   mPrfsColors(0L), mPrfsContactList(0L), currentStyle(0L), loading(false),
	   styleChanged(false)
	{}

	QTabWidget *mAppearanceTabCtl;
	
	ChatMessagePart *preview;
	AppearanceConfig_Emoticons *mPrfsEmoticons;
	AppearanceConfig_ChatWindow *mPrfsChatWindow;
	AppearanceConfig_Colors *mPrfsColors;
	AppearanceConfig_ContactList *mPrfsContactList;
	
	// value is the style path
	QMap<Q3ListBoxItem*,QString> styleItemMap;
	ChatWindowStyle::StyleVariants currentVariantMap;
	ChatWindowStyle *currentStyle;
	bool loading;
	bool styleChanged;
	bool m_allowDownloadTheme;
	// For style preview
	FakeProtocol *previewProtocol;
	FakeAccount *previewAccount;
	Kopete::MetaContact *myselfMetaContact;
	Kopete::MetaContact *jackMetaContact;
	FakeContact *myself;
	FakeContact *jack;
	Kopete::ChatSession *previewChatSession;
};

class KopeteStyleNewStuff : public KNewStuff
{
public:
	KopeteStyleNewStuff(const QString &type, QWidget *parentWidget = 0)
	 : KNewStuff( type, parentWidget)
	{}

	bool createUploadFile(const QString &)
	{
		return false;
	}

	bool install(const QString &styleFilename)
	{
		int styleInstallReturn = 0;
		styleInstallReturn = ChatWindowStyleManager::self()->installStyle( styleFilename );

		switch(styleInstallReturn)
		{
			case ChatWindowStyleManager::StyleInstallOk:
			{
				KMessageBox::queuedMessageBox( this->parentWidget(), KMessageBox::Information, i18n("The Chat Window style was successfully installed."), i18n("Install successful") );
				return true;
			}
			case ChatWindowStyleManager::StyleCannotOpen:
			{
				KMessageBox::queuedMessageBox( this->parentWidget(), KMessageBox::Error, i18n("The specified archive cannot be openned.\nMake sure that the archive is valid ZIP or TAR archive."), i18n("Can't open archive") );
				break;
			}
			case ChatWindowStyleManager::StyleNoDirectoryValid:
			{
				KMessageBox::queuedMessageBox( this->parentWidget(), KMessageBox::Error, i18n("Could not find a suitable place to install the Chat Window style in user directory."), i18n("Can't find styles directory") );
				break;
			}
			case ChatWindowStyleManager::StyleNotValid:
			{
				KMessageBox::queuedMessageBox( this->parentWidget(), KMessageBox::Error, i18n("The specified archive does not contain a valid Chat Window style."), i18n("Invalid Style") );
				break;
			}
				
			case ChatWindowStyleManager::StyleUnknow:
			default:
			{
				KMessageBox::queuedMessageBox( this->parentWidget(), KMessageBox::Error, i18n("An unknow error occurred while trying to install the Chat Window style."), i18n("Unknow error") );
				break;
			}
		}
		return false;
	}
};

AppearanceConfig::AppearanceConfig(QWidget *parent, const QStringList &args )
: KCModule( KopeteAppearanceConfigFactory::instance(), parent, args )
{
	d = new Private;

	QVBoxLayout *layout = new QVBoxLayout(this);

	d->mAppearanceTabCtl = new QTabWidget(this);
	d->mAppearanceTabCtl->setObjectName("mAppearanceTabCtl");
	layout->addWidget( d->mAppearanceTabCtl );

	KConfig *config = KGlobal::config();
	config->setGroup( "ChatWindowSettings" );

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

	// "Chat Window" TAB ========================================================
	d->mPrfsChatWindow = new AppearanceConfig_ChatWindow(d->mAppearanceTabCtl);
	addConfig( Kopete::AppearanceSettings::self(), d->mPrfsChatWindow );

	connect(d->mPrfsChatWindow->styleList, SIGNAL(selectionChanged(Q3ListBoxItem *)),
		this, SLOT(slotChatStyleSelected()));
	connect(d->mPrfsChatWindow->variantList, SIGNAL(activated(const QString&)),
		this, SLOT(slotChatStyleVariantSelected(const QString &)));
	connect(d->mPrfsChatWindow->deleteButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteChatStyle()));
	connect(d->mPrfsChatWindow->installButton, SIGNAL(clicked()),
		this, SLOT(slotInstallChatStyle()));
	connect(d->mPrfsChatWindow->btnGetStyles, SIGNAL(clicked()),
		this, SLOT(slotGetChatStyles()));

	// Show the available styles when the Manager has finish to load the styles.
	connect(ChatWindowStyleManager::self(), SIGNAL(loadStylesFinished()), this, SLOT(slotLoadChatStyles()));

	d->mPrfsChatWindow->htmlFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	// Create the fake Chat Session
	createPreviewChatSession();
	QVBoxLayout *l = new QVBoxLayout(d->mPrfsChatWindow->htmlFrame);
	d->preview = new ChatMessagePart(d->previewChatSession, d->mPrfsChatWindow->htmlFrame);
	d->preview->setJScriptEnabled(false);
	d->preview->setJavaEnabled(false);
	d->preview->setPluginsEnabled(false);
	d->preview->setMetaRefreshEnabled(false);
	KHTMLView *htmlWidget = d->preview->view();
	htmlWidget->setMarginWidth(4);
	htmlWidget->setMarginHeight(4);
	htmlWidget->setFocusPolicy(Qt::NoFocus);
	htmlWidget->setSizePolicy(
		QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	l->addWidget(htmlWidget);
	// Add the preview message to the ChatMessagePart
	createPreviewMessages();

	d->mAppearanceTabCtl->addTab( d->mPrfsChatWindow, i18n("Chat Window") );

	// "Contact List" TAB =======================================================
	d->mPrfsContactList = new AppearanceConfig_ContactList(d->mAppearanceTabCtl);
	addConfig( Kopete::AppearanceSettings::self(), d->mPrfsContactList );

	connect(d->mPrfsContactList->mEditTooltips, SIGNAL(clicked()),
		this, SLOT(slotEditTooltips()));

	// don't enable the checkbox if XRender is not available
	#ifdef HAVE_XRENDER
	d->mPrfsContactList->kcfg_contactListFading->setEnabled(true);
	#else
	d->mPrfsContactList->kcfg_contactListFading->setEnabled(false);
	#endif

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

	// Get the stylePath
	if(d->currentStyle)
	{
		kDebug(14000) << k_funcinfo << d->currentStyle->getStylePath() << endl;
		settings->setStylePath( d->currentStyle->getStylePath() );
	}
	// Get and save the styleVariant
	if( !d->currentVariantMap.empty() )
	{
		kDebug(14000) << k_funcinfo << d->currentVariantMap[ d->mPrfsChatWindow->variantList->currentText()] << endl;
		settings->setStyleVariant( d->currentVariantMap[d->mPrfsChatWindow->variantList->currentText()] );
	}

	settings->writeConfig();
	d->styleChanged = false;

	load();
}

void AppearanceConfig::load()
{
	KCModule::load();

	//we will change the state of somme controls, which will call some signals.
	//so to don't refresh everything several times, we memorize we are loading.
	d->loading=true;

	// Look for available emoticons themes
	updateEmoticonlist();

	// Look for available chat window styles.
	slotLoadChatStyles();

#ifndef HAVE_XRENDER
	d->mPrfsContactList->kcfg_contactListFading->setChecked( false );
#endif

//	kDebug(14000) << k_funcinfo << "called" << endl;

	d->loading=false;
	slotUpdateChatPreview();
}

void AppearanceConfig::slotLoadChatStyles()
{
	d->mPrfsChatWindow->styleList->clear();
	d->styleItemMap.clear();

	ChatWindowStyleManager::StyleList availableStyles;
	availableStyles = ChatWindowStyleManager::self()->getAvailableStyles();
	if( availableStyles.empty() )
		kDebug(14000) << k_funcinfo << "Warning, available styles is empty !" << endl;

	ChatWindowStyleManager::StyleList::ConstIterator it, itEnd = availableStyles.constEnd();
	for(it = availableStyles.constBegin(); it != itEnd; ++it)
	{
		// Insert style name into the listbox
		d->mPrfsChatWindow->styleList->insertItem( it.key(), 0 );
		// Insert the style class into the internal map for futher access.
		d->styleItemMap.insert( d->mPrfsChatWindow->styleList->firstItem(), it.value() );

		if( it.value() == Kopete::AppearanceSettings::self()->stylePath() )
		{
			kDebug(14000) << k_funcinfo << "Restoring saved style: " << it.key() << endl;

			d->mPrfsChatWindow->styleList->setSelected( d->mPrfsChatWindow->styleList->firstItem(), true );
		}
	}

	d->mPrfsChatWindow->styleList->sort();
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
	QStringList smileys = emoticons.emoticonAndPicList().values();
	QString newContentText = "<qt>";

	for(QStringList::Iterator it = smileys.begin(); it != smileys.end(); ++it )
		newContentText += QString::fromLatin1("<img src=\"%1\"> ").arg(*it);

	newContentText += QString::fromLatin1("</qt>");
	d->mPrfsEmoticons->icon_theme_preview->setHtml(newContentText);
	emitChanged();
}

void AppearanceConfig::slotHighlightChanged()
{
//	bool value = mPrfsChatWindow->highlightEnabled->isChecked();
//	mPrfsChatWindow->foregroundColor->setEnabled ( value );
//	mPrfsChatWindow->backgroundColor->setEnabled ( value );
	slotUpdateChatPreview();
}

void AppearanceConfig::slotChangeFont()
{
	slotUpdateChatPreview();
	emitChanged();
}

void AppearanceConfig::slotChatStyleSelected()
{
	// Retrieve variant list.
	QString stylePath = d->styleItemMap[d->mPrfsChatWindow->styleList->selectedItem()];
	d->currentStyle = ChatWindowStyleManager::self()->getStyleFromPool( stylePath );
	
	if(d->currentStyle)
	{
		d->currentVariantMap = d->currentStyle->getVariants();
		kDebug(14000) << k_funcinfo << "Loading style: " << d->currentStyle->getStylePath() << endl;
	
		// Update the variant list based on current style.
		d->mPrfsChatWindow->variantList->clear();
	
		// Add the no variant item to the list
		// TODO: Use default name variant from Info.plist
		// TODO: Select default variant from Info.plist
		d->mPrfsChatWindow->variantList->addItem( i18n("(No Variant)") );

		ChatWindowStyle::StyleVariants::ConstIterator it, itEnd = d->currentVariantMap.constEnd();
		int currentIndex = 0;
		for(it = d->currentVariantMap.constBegin(); it != itEnd; ++it)
		{
			// Insert variant name into the combobox.
			d->mPrfsChatWindow->variantList->addItem( it.key() );
	
			if( it.value() == Kopete::AppearanceSettings::self()->styleVariant() )
				d->mPrfsChatWindow->variantList->setCurrentIndex(currentIndex+1);
	
			currentIndex++;
		}
		
		// Update the preview
		slotUpdateChatPreview();
		// Get the first variant to preview
		// Check if the current style has variants.
		if( !d->currentVariantMap.empty() )
			d->preview->setStyleVariant(d->currentVariantMap[0]);
	
		emitChanged();
	}
}

void AppearanceConfig::slotChatStyleVariantSelected(const QString &variantName)
{
// 	kDebug(14000) << k_funcinfo << variantName << endl;
// 	kDebug(14000) << k_funcinfo << d->currentVariantMap[variantName] << endl;

	// Update the preview
	d->preview->setStyleVariant(d->currentVariantMap[variantName]);
	emitChanged();
}

void AppearanceConfig::slotInstallChatStyle()
{
	KUrl styleToInstall = KFileDialog::getOpenUrl( KUrl(), QString::fromUtf8("application/x-zip application/x-tgz application/x-tbz"), this, i18n("Choose Chat Window style to install.") );

	if( !styleToInstall.isEmpty() )
	{
		QString stylePath;
		if( KIO::NetAccess::download( styleToInstall, stylePath, this ) )
		{
			int styleInstallReturn = 0;
			styleInstallReturn = ChatWindowStyleManager::self()->installStyle( stylePath );
			switch(styleInstallReturn)
			{
				case ChatWindowStyleManager::StyleCannotOpen:
				{
					KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("The specified archive cannot be openned.\nMake sure that the archive is valid ZIP or TAR archive."), i18n("Can't open archive") );
					break;
				}
				case ChatWindowStyleManager::StyleNoDirectoryValid:
				{
					KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("Could not find a suitable place to install the Chat Window style in user directory."), i18n("Can't find styles directory") );
					break;
				}
				case ChatWindowStyleManager::StyleNotValid:
					KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("The specified archive does not contain a valid Chat Window style."), i18n("Invalid Style") );
					break;
				case ChatWindowStyleManager::StyleInstallOk:
				{
					KMessageBox::queuedMessageBox( this, KMessageBox::Information, i18n("The Chat Window style was successfully installed !"), i18n("Install successful") );
					break;
				}
				case ChatWindowStyleManager::StyleUnknow:
				default:
				{
					KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("An unknow error occurred while trying to install the Chat Window style."), i18n("Unknow error") );
					break;
				}
			}
			
			// removeTempFile check if the file is a temp file, so it's ok for local files.
			KIO::NetAccess::removeTempFile( stylePath );
		}
	}
}

void AppearanceConfig::slotDeleteChatStyle()
{
//	QString styleName = d->mPrfsChatWindow->styleList->selectedItem()->text();
//	QString stylePathToDelete = d->styleItemMap[d->mPrfsChatWindow->styleList->selectedItem()];
//	if( ChatWindowStyleManager::self()->removeStyle(stylePathToDelete) )
//	{
//		KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18n("It's the deleted style name", "The style %1 was successfully deleted.").arg(styleName));
//		
		// Get the first item in the stye List.
//		QString stylePath = (*d->styleItemMap.begin());
//		d->currentStyle = ChatWindowStyleManager::self()->getStyleFromPool(stylePath);
//		emitChanged();
//	}
//	else
//	{
//		KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18n("It's the deleted style name", "An error occurred while trying to delete %1 style.").arg(styleName));
//	}
	emitChanged();
}

void AppearanceConfig::slotGetChatStyles()
{
	// we need this because KNewStuffGeneric's install function isn't clever enough
	KopeteStyleNewStuff *kopeteNewStuff = new KopeteStyleNewStuff( "kopete/chatstyle", this );
	KNS::Engine *engine = new KNS::Engine( kopeteNewStuff, "kopete/chatstyle", this );
	KNS::DownloadDialog *downloadDialog = new KNS::DownloadDialog( engine, this );
	downloadDialog->setCategory( "kopete/chatstyle" );
	// you have to do this by hand when providing your own Engine
	KNS::ProviderLoader *provider = new KNS::ProviderLoader( this );
	QObject::connect( provider, SIGNAL( providersLoaded(Provider::List*) ), downloadDialog, SLOT( slotProviders (Provider::List *) ) );
	provider->load( "kopete/chatstyle", "http://download.kde.org/khotnewstuff/kopetestyles12-providers.xml" );
	downloadDialog->exec();
}

// Reimplement Kopete::Contact and its abstract method
// This is for style preview.
class FakeContact : public Kopete::Contact
{
public:
	FakeContact (Kopete::Account *account, const QString &id, Kopete::MetaContact *mc ) : Kopete::Contact( account, id, mc ) {}
	virtual Kopete::ChatSession *manager(Kopete::Contact::CanCreateFlags /*c*/) { return 0L; }
	virtual void slotUserInfo() {};
};

// This is for style preview.
class FakeProtocol : public Kopete::Protocol
{
public:
FakeProtocol( KInstance *instance, QObject *parent ) : Kopete::Protocol(instance, parent){}
Kopete::Account* createNewAccount( const QString &/*accountId*/ ){return 0L;}
AddContactPage* createAddContactWidget( QWidget */*parent*/, Kopete::Account */*account*/){return 0L;}
KopeteEditAccountWidget* createEditAccountWidget( Kopete::Account */*account*/, QWidget */*parent */){return 0L;}
};

// This is for style preview.
class FakeAccount : public Kopete::Account
{
public:
FakeAccount(Kopete::Protocol *parent, const QString &accountID) : Kopete::Account(parent, accountID){}
~FakeAccount()
{}
bool createContact( const QString &/*contactId*/, Kopete::MetaContact */*parentContact*/ ){return true;}
void connect( const Kopete::OnlineStatus& /*initialStatus*/){}
void disconnect(){}
void setOnlineStatus( const Kopete::OnlineStatus& /*status*/ , const Kopete::StatusMessage &/*reason*/){}
void setStatusMessage(const Kopete::StatusMessage& /*statusMessage*/){}
};

void AppearanceConfig::createPreviewChatSession()
{
	d->previewProtocol = new FakeProtocol( new KInstance(QByteArray("kopete-preview-chatwindowstyle")), 0 ); d->previewProtocol->setObjectName( QLatin1String("kopete-preview-chatwindowstyle") );
	d->previewAccount = new FakeAccount(d->previewProtocol, QString("previewaccount"));

	// Create fake meta/contacts
	d->myselfMetaContact = new Kopete::MetaContact();
	d->myself = new FakeContact(d->previewAccount, i18nc("This is the myself preview contact id", "myself@preview"), d->myselfMetaContact);
	d->myself->setNickName(i18nc("This is the myself preview contact nickname", "Myself"));
	d->jackMetaContact = new Kopete::MetaContact();
	d->jack = new FakeContact(d->previewAccount, i18nc("This is the other preview contact id", "jack@preview"), d->jackMetaContact);
	d->jack->setNickName(i18nc("This is the other preview contact nickname", "Jack"));
	d->myselfMetaContact->setDisplayName(i18n("Myself"));
	d->myselfMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);
	d->jackMetaContact->setDisplayName(i18n("Jack"));
	d->jackMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);

	Kopete::ContactPtrList contactList;
	contactList.append(d->jack);
	// Create fakeChatSession
	d->previewChatSession = Kopete::ChatSessionManager::self()->create(d->myself, contactList, 0);
	d->previewChatSession->setDisplayName("Preview Session");	
}

void AppearanceConfig::createPreviewMessages()
{
	Kopete::Message msgIn( d->jack,d->myself, i18n( "Hello, this is an incoming message :-)" ), Kopete::Message::Inbound );
	Kopete::Message msgIn2( d->jack, d->myself, i18n( "Hello, this is an incoming consecutive message." ), Kopete::Message::Inbound );

	Kopete::Message msgOut( d->myself, d->jack, i18n( "Ok, this is an outgoing message" ), Kopete::Message::Outbound );
	Kopete::Message msgOut2( d->myself, d->jack, i18n( "Ok, a outgoing consecutive message." ), Kopete::Message::Outbound );
 
	Kopete::Message msgCol( d->jack, d->myself, i18n( "Here is an incoming colored message" ), Kopete::Message::Inbound );
	msgCol.setFg( QColor( "DodgerBlue" ) );
	msgCol.setBg( QColor( "LightSteelBlue" ) );
	Kopete::Message msgInt( d->jack, d->myself, i18n( "This is an internal message" ), Kopete::Message::Internal );
	Kopete::Message msgAct( d->jack, d->myself, i18n( "performed an action" ), Kopete::Message::Inbound,
				  Kopete::Message::PlainText, QString::null, Kopete::Message::TypeAction );
	Kopete::Message msgHigh( d->jack, d->myself, i18n( "This is a highlighted message" ), Kopete::Message::Inbound );
	msgHigh.setImportance( Kopete::Message::Highlight );
	// This is a UTF-8 string btw.
	Kopete::Message msgRightToLeft(d->myself, d->jack, i18nc("This special UTF-8 string is to test if the style support Right-to-Left language display.", "הודעות טקסט"), Kopete::Message::Outbound);
	Kopete::Message msgBye ( d->myself, d->jack,   i18n( "Bye" ), Kopete::Message::Outbound );

	// Add the messages to ChatMessagePart
	d->preview->appendMessage(msgIn);
	d->preview->appendMessage(msgIn2);
	d->preview->appendMessage(msgOut);
	d->preview->appendMessage(msgOut2);
	d->preview->appendMessage(msgCol);
	d->preview->appendMessage(msgInt);
	d->preview->appendMessage(msgAct);
	d->preview->appendMessage(msgHigh);
	d->preview->appendMessage(msgRightToLeft);
	d->preview->appendMessage(msgBye);
}

void AppearanceConfig::slotUpdateChatPreview()
{
	if(d->loading || !d->currentStyle)
		return;

	// Update the preview
	d->preview->setStyle(d->currentStyle);

	emitChanged();
}

void AppearanceConfig::emitChanged()
{
	emit changed( true );
}

void AppearanceConfig::installEmoticonTheme()
{
	KUrl themeURL = KUrlRequesterDlg::getURL(QString::null, this,
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

        int res = KMessageBox::warningContinueCancel(this, question, i18n("Confirmation"),KStdGuiItem::del());
	if (res!=KMessageBox::Continue)
		return;

	KUrl themeUrl(KGlobal::dirs()->findResource("emoticons", themeName+'/'));
	KIO::NetAccess::del(themeUrl, this);

	updateEmoticonlist();
}

void AppearanceConfig::slotGetEmoticonThemes()
{
	KConfig* config = KGlobal::config();
	config->setGroup( "KNewStuff" );
	config->writeEntry( "ProvidersUrl",
						"http://download.kde.org/khotnewstuff/emoticons-providers.xml" );
	config->writeEntry( "StandardResource", "emoticons" );
	config->writeEntry( "Uncompress", "application/x-gzip" );
	config->sync();

	KNS::DownloadDialog::open( "emoticons", i18n( "Get New Emoticons") );
	
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
