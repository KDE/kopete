/*
    appearanceconfig.cpp  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qcheckbox.h>
#include <qdir.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qhbuttongroup.h>
#include <qstringlist.h>
#include <qtextedit.h>
#include <qvgroupbox.h>
#include <qdatetime.h>
#include <qslider.h>

#include <kcolorcombo.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#if KDE_VERSION >= 306
#include <knotifydialog.h>
#else
#include <kprocess.h>
#endif
#include <kstddirs.h>
#include <ktabctl.h>
#include <kglobalsettings.h>
#include <khtml_part.h>

#include "appearanceconfig_chatwindow.h"
#include "configmodule.h"
#include "kopetechatwindow.h"
#include "kopeteprefs.h"

AppearanceConfig::AppearanceConfig(QWidget * parent) :
	ConfigModule (
		i18n("Appearance"),
		i18n("Here You Can Personalize Kopete's Look & Feel"),
		"appearance",
		parent )
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	KTabCtl *mAppearanceTab = new KTabCtl(this);

	/* ============================================================== */
	mGeneralTab = new QFrame(mAppearanceTab);

	QVBoxLayout *generalLayout = new QVBoxLayout ( mGeneralTab, KDialog::marginHint(), KDialog::spacingHint(), "generalLayout" );

	mStartDockedChk = new QCheckBox( i18n("Start &docked"), mGeneralTab );
	generalLayout->addWidget( mStartDockedChk );

	mUseQueueChk = new QCheckBox( i18n("Use message &queue (don't popup messages)"), mGeneralTab );
	generalLayout->addWidget( mUseQueueChk );

	notifyGroupBox = new QVGroupBox ( i18n("&Notifications"), mGeneralTab, "notifyGroupBox" );
	mBalloonNotifyChk	= new QCheckBox ( i18n("S&how bubble"), notifyGroupBox );
	mTrayflashNotifyChk	= new QCheckBox ( i18n("Flash system &tray"), notifyGroupBox );
	mBeepNotifyChk		= new QCheckBox ( i18n("&Beep"), notifyGroupBox );
	mSoundNotifyChk		= new QCheckBox ( i18n("Play &sounds"), notifyGroupBox );
	mSoundIfAwayChk		= new QCheckBox ( i18n("Play sounds if away"), notifyGroupBox );
	configSound		= new QPushButton( i18n("C&onfigure Sounds..."), notifyGroupBox );
	generalLayout->addWidget( notifyGroupBox );
		
	generalLayout->addStretch();

		
	connect(configSound, SIGNAL(clicked()), this, SLOT(slotConfigSound()));
	connect(mSoundNotifyChk, SIGNAL(clicked()), this, SLOT(slotSoundChanged()));

	mAppearanceTab->addTab ( mGeneralTab, i18n("&General") );
	/* ============================================================== */


	/* ============================================================== */
	mContactListTab = new QFrame(mAppearanceTab);

	QVBoxLayout *contactListLayout = new QVBoxLayout( mContactListTab, KDialog::marginHint(), KDialog::spacingHint(), "contactListLayout" );

	mTreeContactList = new QCheckBox ( i18n("Show as &tree"), mContactListTab );
	contactListLayout->addWidget( mTreeContactList, 0, 0 );
	mSortByGroup = new QCheckBox ( i18n("Sort by &group"), mContactListTab );
	contactListLayout->addWidget( mSortByGroup, 1, 0 );
	mShowOfflineUsers = new QCheckBox ( i18n("Show offline &users"), mContactListTab );
	contactListLayout->addWidget( mShowOfflineUsers, 2, 0 );
	mHideMetaContacts = new QCheckBox ( i18n("&Hide meta contact which contains only one subcontact"), mContactListTab );
	contactListLayout->addWidget( mHideMetaContacts, 3, 0 );
	mGreyIdleMetaContacts = new QCheckBox ( i18n("Grey idle meta contacts"), mContactListTab );
	contactListLayout->addWidget( mGreyIdleMetaContacts, 4, 0 );

	#if KDE_VERSION >= 306
	mNotifyOnlineUsers = new QCheckBox ( i18n("Notify when a user comes online"), mContactListTab );
	contactListLayout->addWidget( mNotifyOnlineUsers, 5, 0 );
	#endif

	QSpacerItem* spacer2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	contactListLayout->addItem( spacer2 );
	
	/*mCTransparancyGroupBox = new QVGroupBox( i18n("Contact List Translucency"), mContactListTab );
		
	mCTransparancyEnabled = new QCheckBox ( i18n("Enable Translucency"), mCTransparancyGroupBox );
	QObject::connect( mCTransparancyEnabled, SIGNAL(toggled(bool)), this, SLOT(slotCTransparancyChanged( bool )));
	
	new QLabel( i18n( "Tint Color:"), mCTransparancyGroupBox );
	mCTransparancyColor = new KColorCombo(mCTransparancyGroupBox);
	
	new QLabel( i18n( "Translucency:"), mCTransparancyGroupBox );
	mCTransparancyValue = new QSlider ( 0, 100, 1, 50, Qt::Horizontal, mCTransparancyGroupBox);
	mCTransparancyValue->setTickmarks( QSlider::Below );
	mCTransparancyValue->setTickInterval( 50 );
	
	contactListLayout->addWidget( mCTransparancyGroupBox );
	
	generalLayout->addWidget( mCTransparancyGroupBox );*/

	mAppearanceTab->addTab( mContactListTab, i18n("Contact &List") );
	/* ============================================================== */

	
	/* ============================================================== */
	mEmoticonsTab = new QFrame(mAppearanceTab);
	(new QVBoxLayout(mEmoticonsTab, KDialog::marginHint(), KDialog::spacingHint()))->setAutoAdd(true);

	// Add a simple list of themes, populated in AppearanceConfig::reopen()
	mUseEmoticonsChk = new QCheckBox ( i18n("&Use emoticons"), mEmoticonsTab );
	icon_theme_list = new KListBox ( mEmoticonsTab, "icon_theme_list" );
	QObject::connect( mUseEmoticonsChk, SIGNAL(toggled(bool)), this, SLOT(slotUseEmoticonsChanged( bool )));

	mAppearanceTab->addTab( mEmoticonsTab, i18n( "&Emoticons" ) );

	/* ============================================================== */
	mChatAppearanceTab = new QFrame(mAppearanceTab);
	generalLayout = new QVBoxLayout ( mChatAppearanceTab, KDialog::marginHint(), KDialog::spacingHint(), "generalLayout" );
	
	cb_RaiseMsgWindowChk = new QCheckBox( i18n( "&Raise window on new messages" ), mChatAppearanceTab, "cb_RaiseMsgWindowChk" );
	generalLayout->addWidget( cb_RaiseMsgWindowChk );

	cb_ShowEventsChk = new QCheckBox( i18n("&Show events in chat window"), mChatAppearanceTab, "cb_ShowEventsChk" );
	generalLayout->addWidget( cb_ShowEventsChk );
	
	ButtonGroup1 = new QHButtonGroup( i18n("Send Message With"), mChatAppearanceTab, "ButtonGroup1");
	ButtonGroup1->setExclusive( true );
	cb_Enter = new QRadioButton( i18n("Enter"), ButtonGroup1);
	cb_CtrlEnter = new QRadioButton( i18n("Ctrl+Enter"), ButtonGroup1);
	cb_ShiftEnter = new QRadioButton( i18n("Shift+Enter"), ButtonGroup1);
	generalLayout->addWidget( ButtonGroup1 );
	
	chatWindowGroup = new QVButtonGroup( i18n("Chat Grouping Policy"), mChatAppearanceTab, "chatWindowGroup");
	chatWindowGroup->setExclusive( true );
	mNewWindow = new QRadioButton( i18n("Open all messages in a new chat window"), chatWindowGroup);
	mTabProtocolWindow = new QRadioButton( i18n("Group messages from the same protocol in the same chat window"), chatWindowGroup);
	mTabWindow = new QRadioButton( i18n("Group all messages in the same chat window"), chatWindowGroup);
	generalLayout->addWidget( chatWindowGroup );
	
	mTransparancyGroupBox = new QVGroupBox( i18n("Chat Window Translucency"), mChatAppearanceTab );
	
	mTransparancyEnabled = new QCheckBox ( i18n("Enable Translucency"), mTransparancyGroupBox );
	QObject::connect( mTransparancyEnabled, SIGNAL(toggled(bool)), this, SLOT(slotTransparancyChanged( bool )));
	
	mBgOverride = new QCheckBox ( i18n("Don't show user specified background color"), mTransparancyGroupBox );
	QObject::connect( mTransparancyEnabled, SIGNAL(toggled(bool)), this, SLOT(slotTransparancyChanged( bool )));
	
	new QLabel( i18n( "Tint Color:"), mTransparancyGroupBox );
	mTransparancyColor = new KColorCombo(mTransparancyGroupBox);
	
	new QLabel( i18n( "Translucency:"), mTransparancyGroupBox );
	mTransparancyValue = new QSlider ( 0, 100, 1, 50, Qt::Horizontal, mTransparancyGroupBox);
	mTransparancyValue->setTickmarks( QSlider::Below );
	mTransparancyValue->setTickInterval( 50 );

	generalLayout->addWidget( mTransparancyGroupBox );
	
	generalLayout->addStretch();
	
	mAppearanceTab->addTab( mChatAppearanceTab, i18n("Chat &Window") );
	/* ============================================================== */
	
	mPrfsChatWindow = new AppearanceConfig_ChatWindow(mAppearanceTab);
	mAppearanceTab->addTab( mPrfsChatWindow, i18n("Chat &Appearance") );
	connect( mPrfsChatWindow->cb_Kind, SIGNAL( activated(int) ), this, SLOT( slotSelectKind(int) ) );

	reopen();
	slotSoundChanged(); //Disable Button if no checkboxes selected
	slotTransparancyChanged( mTransparancyEnabled->isChecked() );
	
	// sync actions, config and prefs-dialog
	connect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
}

AppearanceConfig::~AppearanceConfig()
{
}

void AppearanceConfig::save()
{
	kdDebug(14000) << "[AppearanceConfig] save()" << endl;

	KopetePrefs *p = KopetePrefs::prefs();

	p->setIconTheme( icon_theme_list->currentText() );
	p->setUseEmoticons ( mUseEmoticonsChk->isChecked() );
	p->setTreeView ( mTreeContactList->isChecked() );
	p->setStartDocked ( mStartDockedChk->isChecked() );
	p->setChatWindowPolicy ( chatWindowGroup->id( chatWindowGroup->selected() ) );
	p->setUseQueue ( mUseQueueChk->isChecked() );
	p->setTrayflashNotify ( mTrayflashNotifyChk->isChecked() );
	p->setBalloonNotify ( mBalloonNotifyChk->isChecked() );
	p->setBeepNotify ( mBeepNotifyChk->isChecked() );
	p->setSoundNotify ( mSoundNotifyChk->isChecked() );
	p->setSoundIfAway( mSoundIfAwayChk->isChecked() );
	p->setShowOffline ( mShowOfflineUsers->isChecked() );
	p->setSortByGroup ( mSortByGroup->isChecked() );
	p->setHideMetaContacts( mHideMetaContacts->isChecked() );
	p->setGreyIdleMetaContacts( mGreyIdleMetaContacts->isChecked() );
	#if KDE_VERSION >= 306
	p->setNotifyOnline ( mNotifyOnlineUsers->isChecked() );
	#endif
	p->setRaiseMsgWindow( cb_RaiseMsgWindowChk->isChecked() );
	p->setShowEvents( cb_ShowEventsChk->isChecked() );

	p->setKindMessagesHtml ( mPrfsChatWindow->mle_codehtml->text() );
	
	p->setSendMessageEnter(cb_Enter->isChecked());
	p->setSendMessageCtrlEnter(cb_CtrlEnter->isChecked());
	p->setSendMessageShiftEnter(cb_ShiftEnter->isChecked());
	p->setTransparancyColor( mTransparancyColor->color() );
	p->setTransparancyEnabled( mTransparancyEnabled->isChecked() );
	p->setTransparancyValue( mTransparancyValue->value() );
	/*p->setCTransparancyColor( mCTransparancyColor->color() );
	p->setCTransparancyEnabled( mCTransparancyEnabled->isChecked() );
	p->setCTransparancyValue( mCTransparancyValue->value() );*/
	
	p->setBgOverride( mBgOverride->isChecked() );
	
	disconnect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
	kdDebug(14000) << "[AppearanceConfig] calling KopetePrefs::save()" << endl;
	p->save();
	connect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
}

void AppearanceConfig::slotConfigChanged(void)
{
	kdDebug(14000) << "[AppearanceConfig] slotConfigChanged(), calling reopen() now..." << endl;

	reopen(); // I am lazy :P
}

void AppearanceConfig::reopen()
{
	kdDebug(14000) << "[AppearanceConfig] reopen()" << endl;

	KopetePrefs *p = KopetePrefs::prefs();

	KStandardDirs dir;
	// Wipe out the old list
	icon_theme_list->clear();
	// Get a list of directories in our icon theme dir
	QStringList themeDirs = KGlobal::dirs()->findDirs("data", "kopete/pics/emoticons");
	// This loop adds them all to our theme list
	for(unsigned int x = 0;x < themeDirs.count();x++)
	{
		QDir themeQDir(themeDirs[x]);

		// We only want directories, although there shouldn't be anything else
		themeQDir.setFilter( QDir::Dirs );
		// I guess name is as good as any
		themeQDir.setSorting( QDir::Name );

		for ( unsigned int y = 0; y < themeQDir.count(); y++ )
		{
			QStringList themes = themeQDir.entryList(QDir::Dirs, QDir::Name);
			// We don't care for '.' and '..'
			if ( themeQDir[y] != "." && themeQDir[y] != ".." )
			{
				// Add ourselves to the list, using our directory name
				QPixmap previewPixmap = QPixmap( locate("data","kopete/pics/emoticons/"+themeQDir[y]+"/smile.png") );
				icon_theme_list->insertItem ( previewPixmap,themeQDir[y] );
			}
		}
	}

	// Where is that theme in our big-list-o-themes?
	QListBoxItem *item = icon_theme_list->findItem( p->iconTheme() );

	if (item) // found it... make it the currently selected theme
		icon_theme_list->setCurrentItem( item );
	else // Er, it's not there... select the current item
		icon_theme_list->setCurrentItem( 0 );

	mUseEmoticonsChk->setChecked( p->useEmoticons() );
	icon_theme_list->setEnabled ( p->useEmoticons() );

	mShowOfflineUsers->setChecked( p->showOffline() );
	mHideMetaContacts->setChecked( p->hideMetaContacts() );
	mGreyIdleMetaContacts->setChecked( p->greyIdleMetaContacts() );
	#if KDE_VERSION >= 306
	mNotifyOnlineUsers->setChecked( p->notifyOnline() );
	#endif
	chatWindowGroup->setButton( p->chatWindowPolicy() );
	mTreeContactList->setChecked( p->treeView() );
	mSortByGroup->setChecked( p->sortByGroup() );
	mStartDockedChk->setChecked( p->startDocked() );
	mUseQueueChk->setChecked( p->useQueue() );
	mTrayflashNotifyChk->setChecked ( p->trayflashNotify() );
	mBalloonNotifyChk->setChecked ( p->balloonNotify() );
	mSoundNotifyChk->setChecked ( p->soundNotify() );
	mBeepNotifyChk->setChecked ( p->beepNotify() );
	cb_RaiseMsgWindowChk->setChecked( p->raiseMsgWindow() );
	cb_ShowEventsChk->setChecked( p->showEvents() );

	mPrfsChatWindow->mle_codehtml->setText( p->kindMessagesHtml() );

	cb_Enter->setChecked(p->sendMessageEnter());
	cb_CtrlEnter->setChecked(p->sendMessageCtrlEnter());
	cb_ShiftEnter->setChecked(p->sendMessageShiftEnter());
	
	mTransparancyEnabled->setChecked( p->transparancyEnabled() );
	mTransparancyColor->setColor( p->transparancyColor() );
	mTransparancyValue->setValue( p->transparancyValue() );
	/*mCTransparancyEnabled->setChecked( p->ctransparancyEnabled() );
	mCTransparancyColor->setColor( p->ctransparancyColor() );
	mCTransparancyValue->setValue( p->ctransparancyValue() );*/
	mBgOverride->setChecked( p->bgOverride() );
}

void AppearanceConfig::slotConfigSound()
{
#if KDE_VERSION >= 306
	KNotifyDialog::configure(this);
#else
	KProcess kcm;
	kcm << "kcmshell" << "Sound/kcmnotify";
	if (!kcm.start(KProcess::DontCare))
	{
		KMessageBox::information( this,
		i18n( "<qt>The KControl Module to configure event notifications did not start.\n"
		"Make sure \"kcmshell\" is in your PATH and is working properly</qt>" ),
		i18n( "KControl Launch failed" ) );
	}
#endif
}

void AppearanceConfig::slotSoundChanged()
{
	if ( mSoundNotifyChk->isChecked() )
		configSound->setEnabled(true);
	else
		configSound->setEnabled(false);
}

void AppearanceConfig::slotUseEmoticonsChanged ( bool checked )
{
	icon_theme_list->setEnabled( checked );
}

void AppearanceConfig::slotTransparancyChanged ( bool checked )
{
	mTransparancyColor->setEnabled( checked );
	mTransparancyValue->setEnabled( checked );
	mBgOverride->setEnabled( checked );
}

void AppearanceConfig::slotCTransparancyChanged ( bool checked )
{
	mCTransparancyColor->setEnabled( checked );
	mCTransparancyValue->setEnabled( checked );
}

void AppearanceConfig::slotSelectKind(int k)
{
	if(k > 0)
	{
		QString model = KopeteChatWindow::KindMessagesHTML(k-1);
		mPrfsChatWindow->mle_codehtml->setText( model );
	}
}

#include "appearanceconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

