/*
    chatwindowconfig.cpp  -  Kopete Look Feel Config

    Copyright (c) 2005-2006 by Michaël Larouche       <larouche@kde.org>
    Copyright (c) 2005-2006 by Olivier Goffart         <ogoffart at kde.org>
    Copyright (c) 2007      by Gustavo Pichorim Boiko  <gustavo.boiko@kdemail.net>

    Kopete    (c) 2005-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "chatwindowconfig.h"
#include "emoticonthemedelegate.h"
#include "emoticonthemeitem.h"

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

// For Kopete Chat Window Style configuration and preview.
#include <kopetechatwindowstylemanager.h>
#include <kopetechatwindowstyle.h>
#include <chatmessagepart.h>


// Things we fake to get the message preview to work
#include <kopeteprotocol.h>
#include <kopetemetacontact.h>
#include <kopeteaccount.h>
#include <kopeteidentity.h>
#include <kopetecontact.h>
#include <kopetemessage.h>
#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>
#include <kopetestatusmessage.h>
#include <kopeteappearancesettings.h>

#include "kopeteemoticons.h"

#include "kopeteglobal.h"

#include "kopetechatwindowsettings.h"

K_PLUGIN_FACTORY( KopeteChatWindowConfigFactory,
		registerPlugin<ChatWindowConfig>(); )
K_EXPORT_PLUGIN( KopeteChatWindowConfigFactory("kcm_kopete_chatwindowconfig") )

// Reimplement Kopete::Contact and its abstract method
// This is for style preview.
class FakeContact : public Kopete::Contact
{
public:
	FakeContact (Kopete::Account *account, const QString &id, Kopete::MetaContact *mc ) : Kopete::Contact( account, id, mc ) {}
	virtual Kopete::ChatSession *manager(Kopete::Contact::CanCreateFlags /*c*/) { return 0L; }
	virtual void slotUserInfo() {}
};

// This is for style preview.
class FakeProtocol : public Kopete::Protocol
{
public:
FakeProtocol( const KComponentData &instance, QObject *parent ) : Kopete::Protocol(instance, parent){}
Kopete::Account* createNewAccount( const QString &/*accountId*/ ){return 0L;}
AddContactPage* createAddContactWidget( QWidget */*parent*/, Kopete::Account */*account*/){return 0L;}
KopeteEditAccountWidget* createEditAccountWidget( Kopete::Account */*account*/, QWidget */*parent */){return 0L;}
};

// This is for style preview.
class FakeIdentity : public Kopete::Identity
{
	public:
		FakeIdentity() : Kopete::Identity("Preview Identity") {};
};

// This is for style preview.
class FakeAccount : public Kopete::Account
{
public:
	FakeAccount(Kopete::Protocol *parent, const QString &accountID) : Kopete::Account(parent, accountID)
	{
		m_identity = new FakeIdentity();
		setIdentity(m_identity);
	}

	void setMyself(Kopete::Contact *myself)
	{
		Kopete::Account::setMyself(myself);
	}

	~FakeAccount()
	{
		delete m_identity;
	}

bool createContact( const QString &/*contactId*/, Kopete::MetaContact */*parentContact*/ ){return true;}
void connect( const Kopete::OnlineStatus& /*initialStatus*/){}
void disconnect(){}
void setOnlineStatus( const Kopete::OnlineStatus& /*status*/ , const Kopete::StatusMessage &/*reason*/){}
void setStatusMessage(const Kopete::StatusMessage& /*statusMessage*/){}

private:
	FakeIdentity *m_identity;
};


#ifdef __GNUC__
#warning "Port KNS changes!"
#endif
#if 0
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
				KMessageBox::queuedMessageBox( this->parentWidget(), KMessageBox::Error, i18n("The specified archive cannot be opened.\nMake sure that the archive is valid ZIP or TAR archive."), i18n("Cannot open archive") );
				break;
			}
			case ChatWindowStyleManager::StyleNoDirectoryValid:
			{
				KMessageBox::queuedMessageBox( this->parentWidget(), KMessageBox::Error, i18n("Could not find a suitable place to install the Chat Window style in user directory."), i18n("Cannot find styles directory") );
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
#endif

ChatWindowConfig::ChatWindowConfig(QWidget *parent, const QVariantList &args )
	: KCModule( KopeteChatWindowConfigFactory::componentData(), parent, args ),
		m_currentStyle (0L), m_loading(false), m_styleChanged(false),
		m_previewProtocol(0L), m_previewAccount(0L), m_myselfMetaContact(0L),
		m_jackMetaContact(0L), m_myself(0L), m_jack(0L)
{
	KConfigGroup config(KGlobal::config(), "ChatWindowSettings");

	QVBoxLayout *layout = new QVBoxLayout(this);
	// since KSetting::Dialog has margins here, we don't need our own.
	layout->setContentsMargins( 0, 0, 0, 0);
	m_tab = new QTabWidget(this);
	layout->addWidget(m_tab);

//--------- style tab ---------------------
	QWidget *styleWidget = new QWidget(m_tab);
	m_styleUi.setupUi(styleWidget);
	m_tab->addTab(styleWidget, i18n("&Style"));
	addConfig( KopeteChatWindowSettings::self(), styleWidget );

	connect(m_styleUi.styleList, SIGNAL(selectionChanged(Q3ListBoxItem *)),
		this, SLOT(slotChatStyleSelected()));
	connect(m_styleUi.variantList, SIGNAL(activated(const QString&)),
		this, SLOT(slotChatStyleVariantSelected(const QString &)));
	connect(m_styleUi.deleteButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteChatStyle()));
	connect(m_styleUi.installButton, SIGNAL(clicked()),
		this, SLOT(slotInstallChatStyle()));
	connect(m_styleUi.btnGetStyles, SIGNAL(clicked()),
		this, SLOT(slotGetChatStyles()));

	// Show the available styles when the Manager has finish to load the styles.
	connect(ChatWindowStyleManager::self(), SIGNAL(loadStylesFinished()), this, SLOT(slotLoadChatStyles()));

	// Create the fake Chat Session
	createPreviewChatSession();
	m_preview = new ChatMessagePart(m_previewChatSession, m_styleUi.htmlFrame);
	m_preview->setJScriptEnabled(false);
	m_preview->setJavaEnabled(false);
	m_preview->setPluginsEnabled(false);
	m_preview->setMetaRefreshEnabled(false);
	KHTMLView *htmlWidget = m_preview->view();
	htmlWidget->setMarginWidth(4);
	htmlWidget->setMarginHeight(4);
	htmlWidget->setFocusPolicy(Qt::NoFocus);
	htmlWidget->setSizePolicy(
		QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	layout = new QVBoxLayout(m_styleUi.htmlFrame);
	layout->setMargin(0);
	layout->addWidget(htmlWidget);
	m_styleUi.htmlFrame->setLayout(layout);
	// Add the preview message to the ChatMessagePart
	createPreviewMessages();


//--------- emoticons tab ---------------------
	QWidget *emoticonsWidget = new QWidget(m_tab);
	m_emoticonsUi.setupUi(emoticonsWidget);
	m_tab->addTab(emoticonsWidget, i18n("&Emoticons"));

	m_emoticonsUi.icon_theme_list->setItemDelegate(new EmoticonThemeDelegate(this));
	addConfig( Kopete::AppearanceSettings::self(), emoticonsWidget );

	connect(m_emoticonsUi.icon_theme_list, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotSelectedEmoticonsThemeChanged()));
	connect(m_emoticonsUi.btnInstallTheme, SIGNAL(clicked()),
		this, SLOT(slotInstallEmoticonTheme()));

	connect(m_emoticonsUi.btnGetThemes, SIGNAL(clicked()),
		this, SLOT(slotGetEmoticonThemes()));
	connect(m_emoticonsUi.btnRemoveTheme, SIGNAL(clicked()),
		this, SLOT(slotRemoveEmoticonTheme()));

//--------- colors tab --------------------------
	QWidget *colorsWidget = new QWidget(m_tab);
	m_colorsUi.setupUi(colorsWidget);
	m_tab->addTab(colorsWidget, i18n("Colors && Fonts"));
	addConfig( Kopete::AppearanceSettings::self(), colorsWidget );

	load();
}

ChatWindowConfig::~ChatWindowConfig()
{
	if (m_previewChatSession)
	{
		Kopete::ChatSessionManager::self()->removeSession(m_previewChatSession);
	}

	// Deleting the account will delete jack and myself
	delete m_previewAccount;

	delete m_myselfMetaContact;
	delete m_jackMetaContact;

 	delete m_previewProtocol;
}


void ChatWindowConfig::save()
{
	KCModule::save();
//	kDebug(14000) << "called.";

	KopeteChatWindowSettings *settings = KopeteChatWindowSettings::self();

	// Get the styleName
	if(m_currentStyle)
	{
		kDebug(14000) << m_currentStyle->getStyleName();
		settings->setStyleName( m_currentStyle->getStyleName() );
	}
	// Get and save the styleVariant
	if( !m_currentVariantMap.empty() )
	{
		kDebug(14000) << m_currentVariantMap[ m_styleUi.variantList->currentText()];
		settings->setStyleVariant( m_currentVariantMap[m_styleUi.variantList->currentText()] );
	}

	Kopete::AppearanceSettings *appearanceSettings = Kopete::AppearanceSettings::self();
	QListWidgetItem *item = m_emoticonsUi.icon_theme_list->currentItem();
	
	if (item)
		appearanceSettings->setEmoticonTheme( item->text() );

	appearanceSettings->writeConfig();
	settings->writeConfig();
	m_styleChanged = false;

	load();
}

void ChatWindowConfig::load()
{
	KCModule::load();

	//we will change the state of somme controls, which will call some signals.
	//so to don't refresh everything several times, we memorize we are loading.
	m_loading=true;

	// Look for available chat window styles.
	slotLoadChatStyles();

	// Look for available emoticons themes
	updateEmoticonList();

	m_loading=false;
	slotUpdateChatPreview();
}

void ChatWindowConfig::slotLoadChatStyles()
{
	m_styleUi.styleList->clear();

	QStringList availableStyles;
	availableStyles = ChatWindowStyleManager::self()->getAvailableStyles();
	if( availableStyles.empty() )
		kDebug(14000) << "Warning, available styles is empty !";

	foreach( const QString& styleName, availableStyles )
	{
		// Insert style name into the listbox
		m_styleUi.styleList->insertItem( styleName, 0 );

		if( styleName == KopeteChatWindowSettings::self()->styleName() )
		{
			kDebug(14000) << "Restoring saved style: " << styleName;

			m_styleUi.styleList->setSelected( m_styleUi.styleList->firstItem(), true );
		}
	}

	m_styleUi.styleList->sort();
}


void ChatWindowConfig::slotChatStyleSelected()
{
	// Retrieve variant list.
	QString styleName = m_styleUi.styleList->selectedItem()->text();
	m_currentStyle = ChatWindowStyleManager::self()->getStyleFromPool( styleName );

	if(m_currentStyle)
	{
		m_currentVariantMap = m_currentStyle->getVariants();
		kDebug(14000) << "Loading style: " << m_currentStyle->getStyleName();

		// Update the variant list based on current style.
		m_styleUi.variantList->clear();

		// Add the no variant item to the list
		// TODO: Use default name variant from Info.plist
		// TODO: Select default variant from Info.plist
		m_styleUi.variantList->addItem( i18n("(No Variant)") );

		ChatWindowStyle::StyleVariants::ConstIterator it, itEnd = m_currentVariantMap.constEnd();
		int currentIndex = 0;
		for(it = m_currentVariantMap.constBegin(); it != itEnd; ++it)
		{
			// Insert variant name into the combobox.
			m_styleUi.variantList->addItem( it.key() );

			if( it.value() == KopeteChatWindowSettings::self()->styleVariant() )
				m_styleUi.variantList->setCurrentIndex(currentIndex+1);

			currentIndex++;
		}

		// Update the preview
		slotUpdateChatPreview();
		// Get the first variant to preview
		// Check if the current style has variants.
		if( !m_currentVariantMap.empty() )
			m_preview->setStyleVariant(m_currentVariantMap[0]);

		emitChanged();
	}
}

void ChatWindowConfig::slotChatStyleVariantSelected(const QString &variantName)
{
// 	kDebug(14000) << variantName;
// 	kDebug(14000) << m_currentVariantMap[variantName];

	// Update the preview
	m_preview->setStyleVariant(m_currentVariantMap[variantName]);
	emitChanged();
}

void ChatWindowConfig::slotInstallChatStyle()
{
	KUrl styleToInstall = KFileDialog::getOpenUrl( KUrl(), QString::fromUtf8("application/zip application/x-compressed-tar application/x-bzip-compressed-tar"), this, i18n("Choose Chat Window style to install.") );

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
					KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("The specified archive cannot be opened.\nMake sure that the archive is valid ZIP or TAR archive."), i18n("Cannot open archive") );
					break;
				}
				case ChatWindowStyleManager::StyleNoDirectoryValid:
				{
					KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("Could not find a suitable place to install the Chat Window style in user directory."), i18n("Cannot find styles directory") );
					break;
				}
				case ChatWindowStyleManager::StyleNotValid:
					KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("The specified archive does not contain a valid Chat Window style."), i18n("Invalid Style") );
					break;
				case ChatWindowStyleManager::StyleInstallOk:
				{
					KMessageBox::queuedMessageBox( this, KMessageBox::Information, i18n("The Chat Window style was successfully installed."), i18n("Install successful") );
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

void ChatWindowConfig::slotDeleteChatStyle()
{
	if (!m_styleUi.styleList->selectedItem())
	{
		return; // nothing selected
	}

	QString styleName = m_styleUi.styleList->selectedItem()->text();
	if( ChatWindowStyleManager::self()->removeStyle(styleName) )
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18nc("@info", "The style <resource>%1</resource> was successfully deleted.", styleName));
		emitChanged();
	}
	else
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Sorry, i18nc("@info", "An error occurred while trying to delete the <resource>%1</resource> style. Your account might not have permission to remove it.", styleName));
	}
}

void ChatWindowConfig::slotGetChatStyles()
{
#ifdef __GNUC__
#warning "Port KNS changes!"
#endif
#if 0
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
	delete downloadDialog;
	delete kopeteNewStuff;
	delete engine;
#endif
}

void ChatWindowConfig::createPreviewChatSession()
{
	m_previewProtocol = new FakeProtocol( KComponentData(QByteArray("kopete-preview-chatwindowstyle")), 0 ); m_previewProtocol->setObjectName( QLatin1String("kopete-preview-chatwindowstyle") );
	m_previewAccount = new FakeAccount(m_previewProtocol, QString("previewaccount"));

	// Create fake meta/contacts
	m_myselfMetaContact = new Kopete::MetaContact();
	m_myselfMetaContact->setTemporary();
	m_myselfMetaContact->setDisplayName(i18n("Myself"));
	m_myselfMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);

	m_myself = new FakeContact(m_previewAccount, i18nc("This is the myself preview contact id", "myself@preview"), m_myselfMetaContact);
	m_myself->setNickName(i18nc("This is the myself preview contact nickname", "Myself"));

	m_jackMetaContact = new Kopete::MetaContact();
	m_jackMetaContact->setTemporary();
	m_jackMetaContact->setDisplayName(i18n("Jack"));
	m_jackMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);

	m_jack = new FakeContact(m_previewAccount, i18nc("This is the other preview contact id", "jack@preview"), m_jackMetaContact);
	m_jack->setNickName(i18nc("This is the other preview contact nickname", "Jack"));

	m_previewAccount->setMyself(m_myself);

	Kopete::ContactPtrList contactList;
	contactList.append(m_jack);
	// Create fakeChatSession
	m_previewChatSession = Kopete::ChatSessionManager::self()->create(m_myself, contactList, m_previewProtocol);
	m_previewChatSession->setDisplayName("Preview Session");
}

void ChatWindowConfig::createPreviewMessages()
{
	Kopete::Message msgIn( m_jack,m_myself );
	msgIn.setPlainBody( i18n( "Hello, this is an incoming message :-)" ) );
	msgIn.setDirection( Kopete::Message::Inbound );

	Kopete::Message msgIn2( m_jack, m_myself );
	msgIn2.setPlainBody( i18n( "Hello, this is an incoming consecutive message." ) );
	msgIn2.setDirection( Kopete::Message::Inbound );

	Kopete::Message msgOut( m_myself, m_jack );
	msgOut.setPlainBody( i18n( "Ok, this is an outgoing message" ) );
	msgOut.setDirection( Kopete::Message::Outbound );

	Kopete::Message msgOut2( m_myself, m_jack );
	msgOut2.setPlainBody( i18n( "Ok, an outgoing consecutive message." ) );
	msgOut2.setDirection( Kopete::Message::Outbound );

	Kopete::Message msgCol( m_jack, m_myself );
	msgCol.setPlainBody( i18n("Here is an incoming colored message.") );
	msgCol.setDirection( Kopete::Message::Inbound );
	msgCol.setForegroundColor( QColor( "DodgerBlue" ) );
	msgCol.setBackgroundColor( QColor( "LightSteelBlue" ) );

	Kopete::Message msgInt( m_jack, m_myself );
	msgInt.setPlainBody( i18n( "This is an internal message" ) );
	msgInt.setDirection( Kopete::Message::Internal );

	Kopete::Message msgAct( m_jack, m_myself );
	msgAct.setPlainBody( i18n( "performed an action" ) );
	msgAct.setType( Kopete::Message::TypeAction );
	msgAct.setDirection( Kopete::Message::Inbound );

	Kopete::Message msgHigh( m_jack, m_myself );
	msgHigh.setPlainBody( i18n( "This is a highlighted message" ) );
	msgHigh.setDirection( Kopete::Message::Inbound );
	msgHigh.setImportance( Kopete::Message::Highlight );

	// This is a UTF-8 string btw.
	Kopete::Message msgRightToLeft( m_myself, m_jack );
	msgRightToLeft.setPlainBody( i18nc("This special UTF-8 string is to test if the style support Right-to-Left language display.", "הודעות טקסט") );
	msgRightToLeft.setDirection( Kopete::Message::Outbound );

	Kopete::Message msgBye ( m_myself, m_jack );
	msgBye.setPlainBody( i18n("Bye") );
	msgBye.setDirection( Kopete::Message::Outbound );

	// Add the messages to ChatMessagePart
	m_preview->appendMessage(msgIn);
	m_preview->appendMessage(msgIn2);
	m_preview->appendMessage(msgOut);
	m_preview->appendMessage(msgOut2);
	m_preview->appendMessage(msgCol);
	m_preview->appendMessage(msgInt);
	m_preview->appendMessage(msgAct);
	m_preview->appendMessage(msgHigh);
	m_preview->appendMessage(msgRightToLeft);
	m_preview->appendMessage(msgBye);
}

void ChatWindowConfig::slotUpdateChatPreview()
{
	if(m_loading || !m_currentStyle)
		return;

	// Update the preview
	m_preview->setStyle(m_currentStyle);

	emitChanged();
}

void ChatWindowConfig::slotUpdateEmoticonsButton(bool _b)
{
	QListWidgetItem *item = m_emoticonsUi.icon_theme_list->currentItem();
	if (!item)
		return;
	QString themeName = item->text();
	QFileInfo fileInf(KGlobal::dirs()->findResource("emoticons", themeName+'/'));
	m_emoticonsUi.btnRemoveTheme->setEnabled( _b && fileInf.isWritable());
	m_emoticonsUi.btnGetThemes->setEnabled( false );
}

void ChatWindowConfig::updateEmoticonList()
{
	KStandardDirs dir;

	m_emoticonsUi.icon_theme_list->clear(); // Wipe out old list
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
				QListWidgetItem *item = new EmoticonThemeItem(themeQDir[y]);
				m_emoticonsUi.icon_theme_list->addItem(item);
			}
		}
	}

	// Where is that theme in our big-list-o-themes?

	QList<QListWidgetItem*> items = m_emoticonsUi.icon_theme_list->findItems( Kopete::AppearanceSettings::self()->emoticonTheme(), Qt::MatchExactly );

	if (items.count()) // found it... make it the currently selected theme
		m_emoticonsUi.icon_theme_list->setCurrentItem( items.first() );
	else // Er, it's not there... select the current item
		m_emoticonsUi.icon_theme_list->setCurrentItem( 0 );
}

void ChatWindowConfig::slotSelectedEmoticonsThemeChanged()
{
	QListWidgetItem *item = m_emoticonsUi.icon_theme_list->currentItem();
	if (!item)
		return;
	QString themeName = item->text();
	QFileInfo fileInf(KGlobal::dirs()->findResource("emoticons", themeName+'/'));
	m_emoticonsUi.btnRemoveTheme->setEnabled( fileInf.isWritable() );

	emitChanged();
}

void ChatWindowConfig::slotInstallEmoticonTheme()
{
	KUrl themeURL = KUrlRequesterDialog::getUrl(QString::null, this,	//krazy:exclude=nullstrassign for old broken gcc
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
	updateEmoticonList();
}

void ChatWindowConfig::slotRemoveEmoticonTheme()
{
	QListWidgetItem *selected = m_emoticonsUi.icon_theme_list->currentItem();
	if(!selected)
		return;

	QString themeName = selected->text();

	QString question=i18n("<qt>Are you sure you want to remove the "
			"<strong>%1</strong> emoticon theme?<br />"
			"<br />"
			"This will delete the files installed by this theme.</qt>",
		themeName);

        int res = KMessageBox::warningContinueCancel(this, question, i18n("Confirmation"),KStandardGuiItem::del());
	if (res!=KMessageBox::Continue)
		return;

	KUrl themeUrl(KGlobal::dirs()->findResource("emoticons", themeName+'/'));
	KIO::NetAccess::del(themeUrl, this);

	updateEmoticonList();
}

void ChatWindowConfig::slotGetEmoticonThemes()
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

	updateEmoticonList();
}


void ChatWindowConfig::emitChanged()
{
	emit changed( true );
}

#include "chatwindowconfig.moc"
