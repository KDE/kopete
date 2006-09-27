/*
    appearanceconfig.cpp  -  Kopete Look Feel Config

    Copyright (c) 2005-2006 by Michaël Larouche       <michael.larouche@kdemail.net>
    Copyright (c) 2005-2006 by Olivier Goffart         <ogoffart at kde.org>

    Kopete    (c) 2005-2006 by the Kopete developers  <kopete-devel@kde.org>

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

#include "chatwindowconfig.h"

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

#include "kopeteglobal.h"

#include <qtabwidget.h>

#include "kopetechatwindowsettings.h"

typedef KGenericFactory<ChatWindowConfig, QWidget> KopeteChatWindowConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_chatwindowconfig, KopeteChatWindowConfigFactory( "kcm_kopete_chatwindowconfig" ) )



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
				KMessageBox::queuedMessageBox( this->parentWidget(), KMessageBox::Error, i18n("The specified archive cannot be openned.\nMake sure that the archive is valid ZIP or TAR archive."), i18n("Cannot open archive") );
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

ChatWindowConfig::ChatWindowConfig(QWidget *parent, const QStringList &args )
	: KCModule( KopeteChatWindowConfigFactory::instance(), parent, args ) 
		, m_currentStyle (0L), m_loading(false), m_styleChanged(false)  
{
	
	QVBoxLayout *layout = new QVBoxLayout(this);

	QTabWidget *chatWindowTabCtl = new QTabWidget(this);
	layout->addWidget( chatWindowTabCtl );
	setLayout(layout);

	KConfig *config = KGlobal::config();
	config->setGroup( "ChatWindowSettings" );


	// "Style" TAB ========================================================
	QWidget *styleWidget=new QWidget(chatWindowTabCtl);
	m_styleUi.setupUi(styleWidget);
	
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

	m_styleUi.htmlFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
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
	layout->addWidget(htmlWidget);
	m_styleUi.htmlFrame->setLayout(layout);
	// Add the preview message to the ChatMessagePart
	createPreviewMessages();

	chatWindowTabCtl->addTab( styleWidget, i18n("Style") );


	load();
}

ChatWindowConfig::~ChatWindowConfig()
{
}


void ChatWindowConfig::save()
{
	KCModule::save();
//	kDebug(14000) << k_funcinfo << "called." << endl;

	KopeteChatWindowSettings *settings = KopeteChatWindowSettings::self();
	
	// Get the stylePath
	if(m_currentStyle)
	{
		kDebug(14000) << k_funcinfo << m_currentStyle->getStylePath() << endl;
		settings->setStylePath( m_currentStyle->getStylePath() );
	}
	// Get and save the styleVariant
	if( !m_currentVariantMap.empty() )
	{
		kDebug(14000) << k_funcinfo << m_currentVariantMap[ m_styleUi.variantList->currentText()] << endl;
		settings->setStyleVariant( m_currentVariantMap[m_styleUi.variantList->currentText()] );
	}

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

	m_loading=false;
	slotUpdateChatPreview();
}

void ChatWindowConfig::slotLoadChatStyles()
{
	m_styleUi.styleList->clear();
	m_styleItemMap.clear();

	ChatWindowStyleManager::StyleList availableStyles;
	availableStyles = ChatWindowStyleManager::self()->getAvailableStyles();
	if( availableStyles.empty() )
		kDebug(14000) << k_funcinfo << "Warning, available styles is empty !" << endl;

	ChatWindowStyleManager::StyleList::ConstIterator it, itEnd = availableStyles.constEnd();
	for(it = availableStyles.constBegin(); it != itEnd; ++it)
	{
		// Insert style name into the listbox
		m_styleUi.styleList->insertItem( it.key(), 0 );
		// Insert the style class into the internal map for futher access.
		m_styleItemMap.insert( m_styleUi.styleList->firstItem(), it.value() );

		if( it.value() == KopeteChatWindowSettings::self()->stylePath() )
		{
			kDebug(14000) << k_funcinfo << "Restoring saved style: " << it.key() << endl;

			m_styleUi.styleList->setSelected( m_styleUi.styleList->firstItem(), true );
		}
	}

	m_styleUi.styleList->sort();
}


void ChatWindowConfig::slotChatStyleSelected()
{
	// Retrieve variant list.
	QString stylePath = m_styleItemMap[m_styleUi.styleList->selectedItem()];
	m_currentStyle = ChatWindowStyleManager::self()->getStyleFromPool( stylePath );
	
	if(m_currentStyle)
	{
		m_currentVariantMap = m_currentStyle->getVariants();
		kDebug(14000) << k_funcinfo << "Loading style: " << m_currentStyle->getStylePath() << endl;
	
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
// 	kDebug(14000) << k_funcinfo << variantName << endl;
// 	kDebug(14000) << k_funcinfo << m_currentVariantMap[variantName] << endl;

	// Update the preview
	m_preview->setStyleVariant(m_currentVariantMap[variantName]);
	emitChanged();
}

void ChatWindowConfig::slotInstallChatStyle()
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
					KMessageBox::queuedMessageBox( this, KMessageBox::Error, i18n("The specified archive cannot be openned.\nMake sure that the archive is valid ZIP or TAR archive."), i18n("Cannot open archive") );
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

void ChatWindowConfig::slotDeleteChatStyle()
{
//	QString styleName = m_styleUi.styleList->selectedItem()->text();
//	QString stylePathToDelete = m_styleItemMap[m_styleUi.styleList->selectedItem()];
//	if( ChatWindowStyleManager::self()->removeStyle(stylePathToDelete) )
//	{
//		KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18n("It's the deleted style name", "The style %1 was successfully deleted.").arg(styleName));
//		
		// Get the first item in the stye List.
//		QString stylePath = (*m_styleItemMap.begin());
//		m_currentStyle = ChatWindowStyleManager::self()->getStyleFromPool(stylePath);
//		emitChanged();
//	}
//	else
//	{
//		KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18n("It's the deleted style name", "An error occurred while trying to delete %1 style.").arg(styleName));
//	}
	emitChanged();
}

void ChatWindowConfig::slotGetChatStyles()
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

void ChatWindowConfig::createPreviewChatSession()
{
	m_previewProtocol = new FakeProtocol( new KInstance(QByteArray("kopete-preview-chatwindowstyle")), 0 ); m_previewProtocol->setObjectName( QLatin1String("kopete-preview-chatwindowstyle") );
	m_previewAccount = new FakeAccount(m_previewProtocol, QString("previewaccount"));

	// Create fake meta/contacts
	m_myselfMetaContact = new Kopete::MetaContact();
	m_myself = new FakeContact(m_previewAccount, i18nc("This is the myself preview contact id", "myself@preview"), m_myselfMetaContact);
	m_myself->setNickName(i18nc("This is the myself preview contact nickname", "Myself"));
	m_jackMetaContact = new Kopete::MetaContact();
	m_jack = new FakeContact(m_previewAccount, i18nc("This is the other preview contact id", "jack@preview"), m_jackMetaContact);
	m_jack->setNickName(i18nc("This is the other preview contact nickname", "Jack"));
	m_myselfMetaContact->setDisplayName(i18n("Myself"));
	m_myselfMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);
	m_jackMetaContact->setDisplayName(i18n("Jack"));
	m_jackMetaContact->setDisplayNameSource(Kopete::MetaContact::SourceCustom);

	Kopete::ContactPtrList contactList;
	contactList.append(m_jack);
	// Create fakeChatSession
	m_previewChatSession = Kopete::ChatSessionManager::self()->create(m_myself, contactList, 0);
	m_previewChatSession->setDisplayName("Preview Session");	
}

void ChatWindowConfig::createPreviewMessages()
{
	Kopete::Message msgIn( m_jack,m_myself, i18n( "Hello, this is an incoming message :-)" ), Kopete::Message::Inbound );
	Kopete::Message msgIn2( m_jack, m_myself, i18n( "Hello, this is an incoming consecutive message." ), Kopete::Message::Inbound );

	Kopete::Message msgOut( m_myself, m_jack, i18n( "Ok, this is an outgoing message" ), Kopete::Message::Outbound );
	Kopete::Message msgOut2( m_myself, m_jack, i18n( "Ok, a outgoing consecutive message." ), Kopete::Message::Outbound );
 
	Kopete::Message msgCol( m_jack, m_myself, i18n( "Here is an incoming colored message" ), Kopete::Message::Inbound );
	msgCol.setFg( QColor( "DodgerBlue" ) );
	msgCol.setBg( QColor( "LightSteelBlue" ) );
	Kopete::Message msgInt( m_jack, m_myself, i18n( "This is an internal message" ), Kopete::Message::Internal );
	Kopete::Message msgAct( m_jack, m_myself, i18n( "performed an action" ), Kopete::Message::Inbound,
				  Kopete::Message::PlainText, QString::null, Kopete::Message::TypeAction );
	Kopete::Message msgHigh( m_jack, m_myself, i18n( "This is a highlighted message" ), Kopete::Message::Inbound );
	msgHigh.setImportance( Kopete::Message::Highlight );
	// This is a UTF-8 string btw.
	Kopete::Message msgRightToLeft(m_myself, m_jack, i18nc("This special UTF-8 string is to test if the style support Right-to-Left language display.", "הודעות טקסט"), Kopete::Message::Outbound);
	Kopete::Message msgBye ( m_myself, m_jack,   i18n( "Bye" ), Kopete::Message::Outbound );

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

void ChatWindowConfig::emitChanged()
{
	emit changed( true );
}

#include "chatwindowconfig.moc"

