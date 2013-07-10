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
#include "kopetebehaviorsettings.h"

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
#include <kurlrequesterdialog.h>
#include <krun.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kemoticons.h>
#include <KCMultiDialog>

// KNewStuff
#include <knewstuff2/engine.h>

// For Kopete Chat Window Style configuration and preview.
#include <kopetechatwindowstylemanager.h>
#include <kopetechatwindowstyle.h>
#include <chatmessagepart.h>
#include <kopetecontactlist.h>


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
void setOnlineStatus( const Kopete::OnlineStatus& /*status*/, const Kopete::StatusMessage &/*reason*/, const OnlineStatusOptions&/*options*/){}
void setStatusMessage(const Kopete::StatusMessage& /*statusMessage*/){}

private:
	FakeIdentity *m_identity;
};

ChatWindowConfig::ChatWindowConfig(QWidget *parent, const QVariantList &args )
	: KCModule( KopeteChatWindowConfigFactory::componentData(), parent, args ),
		m_currentStyle (0L), m_loading(false),
		m_previewProtocol(0L), m_previewAccount(0L),
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

	connect(m_styleUi.styleList, SIGNAL(currentTextChanged(QString)),
		this, SLOT(slotChatStyleSelected(QString)));
	connect(m_styleUi.variantList, SIGNAL(activated(QString)),
		this, SLOT(slotChatStyleVariantSelected(QString)));
	connect(m_styleUi.deleteButton, SIGNAL(clicked()),
		this, SLOT(slotDeleteChatStyle()));
	connect(m_styleUi.installButton, SIGNAL(clicked()),
		this, SLOT(slotInstallChatStyle()));
	connect(m_styleUi.btnGetStyles, SIGNAL(clicked()),
		this, SLOT(slotGetChatStyles()));

	m_styleUi.deleteButton->setIcon(KIcon("edit-delete"));
	m_styleUi.installButton->setIcon(KIcon("document-import"));
	m_styleUi.btnGetStyles->setIcon(KIcon("get-hot-new-stuff"));

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
		this, SLOT(emitChanged()));

	connect(m_emoticonsUi.btnManageThemes, SIGNAL(clicked()),
		this, SLOT(slotManageEmoticonThemes()));

//--------- colors tab --------------------------
	QWidget *colorsWidget = new QWidget(m_tab);
	m_colorsUi.setupUi(colorsWidget);
	m_tab->addTab(colorsWidget, i18n("Colors && Fonts"));
	addConfig( Kopete::AppearanceSettings::self(), colorsWidget );

//--------- tab tab --------------------------
	QWidget *tabWidget = new QWidget(m_tab);
	m_tabUi.setupUi(tabWidget);
	m_tab->addTab(tabWidget, i18n("&Tabs"));
	addConfig( Kopete::BehaviorSettings::self(), tabWidget );

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
		KEmoticons::setTheme( item->text() );

	// Ugly hacks, this will emit the kcfg signals
	appearanceSettings->setChatTextColor(m_colorsUi.kcfg_chatTextColor->color());
	appearanceSettings->setUseEmoticons(m_emoticonsUi.kcfg_useEmoticons->isChecked());
	settings->setChatFmtOverride(m_colorsUi.kcfg_chatFmtOverride->isChecked());

	appearanceSettings->writeConfig();
	settings->writeConfig();

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
		m_styleUi.styleList->insertItem( 0, styleName );
	}

	m_styleUi.styleList->setSortingEnabled( true );

	QString currentStyle = KopeteChatWindowSettings::self()->styleName();
	QList<QListWidgetItem *> items = m_styleUi.styleList->findItems( currentStyle, Qt::MatchFixedString | Qt::MatchCaseSensitive );
	if( items.count() > 0 )
	{
		kDebug(14000) << "Restoring saved style: " << currentStyle;

		m_styleUi.styleList->setCurrentItem( items[0] );
		m_styleUi.styleList->scrollToItem( items[0] );
	}
}


void ChatWindowConfig::slotChatStyleSelected(const QString &styleName)
{
	if (styleName.isEmpty())
		return;
	// Retrieve variant list.
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
		if( !m_currentVariantMap.empty() ) {
			m_preview->setStyleVariant(m_currentVariantMap[0]);
			m_styleUi.kcfg_useCompact->setEnabled(m_currentStyle->hasCompact( QString() ) );
		}

		emitChanged();
	}
	else {
		m_styleUi.variantList->clear();
		slotUpdateChatPreview();
	}
}

void ChatWindowConfig::slotChatStyleVariantSelected(const QString &variantName)
{
// 	kDebug(14000) << variantName;
// 	kDebug(14000) << m_currentVariantMap[variantName];

	// enable the 'Use compact' checkbox depending on whether the selected variant exists in compact
	// form
	QString styleName = m_styleUi.styleList->currentItem()->text();
	m_currentStyle = ChatWindowStyleManager::self()->getStyleFromPool( styleName );
	if ( !m_currentStyle )
		return;

	if ( m_styleUi.variantList->currentIndex() == 0 ) {
		m_styleUi.kcfg_useCompact->setEnabled(m_currentStyle->hasCompact( "" ) );
	}
	else {
		m_styleUi.kcfg_useCompact->setEnabled(m_currentStyle->hasCompact( variantName ) );
	}
	// Update the preview
	m_preview->setStyleVariant(m_currentVariantMap[variantName]);
	emitChanged();
}

void ChatWindowConfig::slotInstallChatStyle()
{
	KUrl styleUrl = KFileDialog::getOpenUrl( KUrl(), QString::fromUtf8("application/zip application/x-compressed-tar application/x-bzip-compressed-tar"), this, i18n("Choose Chat Window Style to Install") );

	if ( styleUrl.isEmpty() ) // dialog got canceled
		return;

	int styleInstallReturn = installChatStyle( styleUrl );

	switch(styleInstallReturn)
	{
		case ChatWindowStyleManager::StyleCannotOpen:
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("The specified archive cannot be opened.\nMake sure that the archive is a valid ZIP or TAR archive."), i18n("Cannot Open Archive") );
			break;
		}
		case ChatWindowStyleManager::StyleNoDirectoryValid:
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("Could not find a suitable place to install the chat window style."), i18n("Cannot Find Styles Directory") );
			break;
		}
		case ChatWindowStyleManager::StyleNotValid:
			KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("The specified archive does not contain a valid chat window style."), i18n("Invalid Style") );
			break;
		case ChatWindowStyleManager::StyleInstallOk:
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Information, i18n("The chat window style was successfully installed."), i18n("Install Successful") );
			break;
		}
		case ChatWindowStyleManager::StyleUnknow:
		default:
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("An unknown error occurred while trying to install the chat window style."), i18n("Unknown Error") );
			break;
		}
	}
}

int ChatWindowConfig::installChatStyle(const KUrl &styleToInstall)
{
	int styleInstallReturn = ChatWindowStyleManager::StyleCannotOpen;

	if( !styleToInstall.isEmpty() )
	{
		QString stylePath;
		if( KIO::NetAccess::download( styleToInstall, stylePath, this ) )
		{
			styleInstallReturn = ChatWindowStyleManager::self()->installStyle( stylePath );

			// removeTempFile check if the file is a temp file, so it's ok for local files.
			KIO::NetAccess::removeTempFile( stylePath );
		}
	}

	return styleInstallReturn;
}

void ChatWindowConfig::slotDeleteChatStyle()
{
	if (!m_styleUi.styleList->currentItem())
	{
		return; // nothing selected
	}

	QString styleName = m_styleUi.styleList->currentItem()->text();
	if( ChatWindowStyleManager::self()->removeStyle(styleName) )
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18nc("@info", "The Chat Window Style <resource>%1</resource> was successfully deleted.", styleName));
		emitChanged();
	}
	else
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Sorry, i18nc("@info", "An error occurred while trying to delete the <resource>%1</resource> Chat Window Style. Your account might not have permission to remove it.", styleName));
	}
	slotUpdateChatPreview();
}

void ChatWindowConfig::slotGetChatStyles()
{
	KConfigGroup configGrp(KGlobal::config(), "KNewStuff2");
	configGrp.writeEntry("ProvidersUrl", "http://download.kde.org/khotnewstuff/kopetestyles12-providers.xml");
	configGrp.writeEntry("TargetDir", "kopete_chatstyles");
	configGrp.sync();
	
	KNS::Engine *engine = new KNS::Engine();
	engine->init(configGrp.config()->name());
	
	// FIXME: Upon closing the Settings KCMultiDialog all KCMs are deleted and when reopening
	// the settings dialog there is no active valid KComponentData, which KNS2 relies on.
	// Forcing an active one below works around bug 163382, but the problem is somewhere else.
	KGlobal::setActiveComponent(KopeteChatWindowConfigFactory::componentData());

	KNS::Entry::List entries = engine->downloadDialogModal(this);

	if ( entries.size() > 0 )
	{
		int correctlyInstalled(0);
		foreach( KNS::Entry* entry, entries )
		{
			if ( entry->status() == KNS::Entry::Installed && entry->installedFiles().size() > 0 )
			{
				KUrl styleFile( entry->installedFiles().at(0) );
				int result = installChatStyle( styleFile );

				QString packageName(entry->name().representation());
				QString errorTitle = i18nc("@title:window", "Chat Window Style <resource>%1</resource> installation", packageName);
				switch(result)
				{
					case ChatWindowStyleManager::StyleCannotOpen:
					{
						KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18nc("@info", "The specified archive <filename>%1</filename> cannot be opened.\nMake sure that the archive is a valid ZIP or TAR archive.", styleFile.pathOrUrl()), errorTitle);
						break;
					}
					case ChatWindowStyleManager::StyleNoDirectoryValid:
					{
						KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18nc("@info", "Could not find a suitable place to install the Chat Window Style <resource>%1</resource>.", packageName), errorTitle );
						break;
					}
					case ChatWindowStyleManager::StyleNotValid:
						KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18nc("@info", "The specified archive <filename>%1</filename> does not contain a valid Chat Window Style.", styleFile.pathOrUrl()), errorTitle );
						break;
					case ChatWindowStyleManager::StyleInstallOk:
					{
						++correctlyInstalled;
						break;
					}
					case ChatWindowStyleManager::StyleUnknow:
					default:
					{
						KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18nc("@info", "An unknown error occurred while trying to install the Chat Window Style <resource>%1</resource>.", packageName), errorTitle );
						break;
					}
				}
			}
		}

		if ( correctlyInstalled > 0)
		{
			KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18np("One Chat Window Style package has been installed.", "%1 Chat Window Style packages have been installed.", correctlyInstalled));
		}
	}

	delete engine;
}

void ChatWindowConfig::createPreviewChatSession()
{
	m_previewProtocol = new FakeProtocol( KComponentData(QByteArray("kopete-preview-chatwindowstyle")), 0 ); m_previewProtocol->setObjectName( QLatin1String("kopete-preview-chatwindowstyle") );
	m_previewAccount = new FakeAccount(m_previewProtocol, QString("previewaccount"));

	m_myself = new FakeContact(m_previewAccount, i18nc("This is the myself preview contact id", "myself@preview"), Kopete::ContactList::self()->myself());
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
	m_previewChatSession->setDisplayName(i18nc("preview of a chat session", "Preview Session"));
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

	Kopete::Message msgOut3( m_myself, m_jack );
	msgOut3.setPlainBody( i18n( "Message that is being sent." ) );
	msgOut3.setDirection( Kopete::Message::Outbound );
	msgOut3.setState( Kopete::Message::StateSending );

	Kopete::Message msgOut4( m_myself, m_jack );
	msgOut4.setPlainBody( i18n( "Delivered message." ) );
	msgOut4.setDirection( Kopete::Message::Outbound );
	msgOut4.setState( Kopete::Message::StateSent );

	Kopete::Message msgOut5( m_myself, m_jack );
	msgOut5.setPlainBody( i18n( "Message that cannot be delivered." ) );
	msgOut5.setDirection( Kopete::Message::Outbound );
	msgOut5.setState( Kopete::Message::StateError );

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

	Kopete::Message msgFTRequest( m_jack, m_myself );
	msgFTRequest.setPlainBody( i18n( "Hello, this is an incoming file transfer request" ) );
	msgFTRequest.setDirection( Kopete::Message::Inbound );
	msgFTRequest.setType( Kopete::Message::TypeFileTransferRequest );
	msgFTRequest.setFileName( "data.pdf" );
	msgFTRequest.setFileSize( 10000000 );

	Kopete::Message msgFTRequestDisabled( m_jack, m_myself );
	msgFTRequestDisabled.setPlainBody( i18n( "Hello, this is a disabled incoming file transfer request" ) );
	msgFTRequestDisabled.setDirection( Kopete::Message::Inbound );
	msgFTRequestDisabled.setType( Kopete::Message::TypeFileTransferRequest );
	msgFTRequestDisabled.setFileName( "data.pdf" );
	msgFTRequestDisabled.setFileSize( 10000000 );
	msgFTRequestDisabled.setFileTransferDisabled( true );

	// This is a UTF-8 string btw.
	Kopete::Message msgRightToLeft( m_myself, m_jack );
	msgRightToLeft.setPlainBody( i18nc("This special UTF-8 string is to test if the style supports Right-to-Left language display.", "הודעות טקסט") );
	msgRightToLeft.setDirection( Kopete::Message::Outbound );

	Kopete::Message msgBye ( m_myself, m_jack );
	msgBye.setPlainBody( i18n("Bye") );
	msgBye.setDirection( Kopete::Message::Outbound );

	// Add the messages to ChatMessagePart
	m_preview->appendMessage(msgIn);
	m_preview->appendMessage(msgIn2);
	m_preview->appendMessage(msgOut);
	m_preview->appendMessage(msgOut2);
	m_preview->appendMessage(msgOut3);
	m_preview->appendMessage(msgOut4);
	m_preview->appendMessage(msgOut5);
	m_preview->appendMessage(msgCol);
	m_preview->appendMessage(msgInt);
	m_preview->appendMessage(msgAct);
	m_preview->appendMessage(msgHigh);
	m_preview->appendMessage(msgFTRequest);
	m_preview->appendMessage(msgFTRequestDisabled);
	m_preview->appendMessage(msgRightToLeft);
	m_preview->appendMessage(msgBye);
}

void ChatWindowConfig::slotUpdateChatPreview()
{
	if(m_loading)
		return;

	// Update the preview
	m_preview->setStyle(m_currentStyle);

	emitChanged();
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

	QList<QListWidgetItem*> items = m_emoticonsUi.icon_theme_list->findItems( KEmoticons::currentThemeName(), Qt::MatchExactly );

	if (items.count()) // found it... make it the currently selected theme
		m_emoticonsUi.icon_theme_list->setCurrentItem( items.first() );
	else // Er, it's not there... select the current item
		m_emoticonsUi.icon_theme_list->setCurrentItem( 0 );
}


void ChatWindowConfig::slotManageEmoticonThemes()
{
	// FIXME: Upon closing the Settings KCMultiDialog all KCMs are deleted and when reopening
	// the settings dialog there is no active valid KComponentData, which KNS2 relies on.
	// Forcing an active one below works around bug 165919, but the problem is somewhere else.
	KGlobal::setActiveComponent(KopeteChatWindowConfigFactory::componentData());

	KCMultiDialog *kcm = new KCMultiDialog( this );
	kcm->setCaption( i18n( "Configure Emoticon Themes" ) );
	kcm->addModule( "emoticons" );
	kcm->exec();
	updateEmoticonList();
}


void ChatWindowConfig::emitChanged()
{
	emit changed( true );
}

#include "chatwindowconfig.moc"
