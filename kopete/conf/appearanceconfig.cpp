/*
    appearanceconfig.cpp  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Kopete    (c) 2002      by the Kopete developers  <kopete-devel@kde.org>

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
//#include <qdatetime.h>
#include <qslider.h>

#include <kcolorcombo.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <khtmlview.h>
#include <khtml_part.h>
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

#include "appearanceconfig_general.h"
#include "appearanceconfig_contactlist.h"
#include "appearanceconfig_chatwindow.h"
#include "appearanceconfig_chatappearance.h"
#include "configmodule.h"
#include "kopetechatwindow.h"
#include "kopeteprefs.h"
#include "kopetemessage.h"
#include "kopetecontact.h"

AppearanceConfig::AppearanceConfig(QWidget * parent) :
	ConfigModule (
		i18n("Behaviour"),
		i18n("Here You Can Personalize Kopete"),
		"appearance",
		parent )
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	KTabCtl *mAppearanceTabCtl = new KTabCtl(this);

	// "General" TAB =============================================================
	mPrfsGeneral = new AppearanceConfig_General(mAppearanceTabCtl);
	connect(mPrfsGeneral->configSound, SIGNAL(clicked()), this, SLOT(slotConfigSound()));
	connect(mPrfsGeneral->mSoundNotifyChk, SIGNAL(clicked()), this, SLOT(slotSoundChanged()));
	mAppearanceTabCtl->addTab( mPrfsGeneral, i18n("&General") );

	// "Contact List" TAB ========================================================
	mPrfsContactlist = new AppearanceConfig_Contactlist(mAppearanceTabCtl);
	#if KDE_VERSION < 306
		mPrfsContactlist->mNotifyOnlineUsers->hide();
	#endif
	mAppearanceTabCtl->addTab( mPrfsContactlist, i18n("Contact &List") );

	// "Emoticons" TAB ===========================================================
	mEmoticonsTab = new QFrame(mAppearanceTabCtl);
	(new QVBoxLayout(mEmoticonsTab, KDialog::marginHint(), KDialog::spacingHint()))->setAutoAdd(true);
	mUseEmoticonsChk = new QCheckBox ( i18n("&Use emoticons"), mEmoticonsTab );
	icon_theme_list = new KListBox ( mEmoticonsTab, "icon_theme_list" );
	connect(mUseEmoticonsChk, SIGNAL(toggled(bool)), this, SLOT(slotUseEmoticonsChanged( bool )));
	mAppearanceTabCtl->addTab( mEmoticonsTab, i18n("&Emoticons") );

	// "Chat Window" TAB =========================================================
	mPrfsChatWindow = new AppearanceConfig_ChatWindow(mAppearanceTabCtl);
	mAppearanceTabCtl->addTab( mPrfsChatWindow, i18n("Chat &Window") );
	connect(mPrfsChatWindow->mTransparencyEnabled, SIGNAL(toggled(bool)), this, SLOT(slotTransparencyChanged(bool)));

	// "Chat Appearance" TAB =====================================================
	mPrfsChatAppearance = new AppearanceConfig_ChatAppearance(mAppearanceTabCtl);
	mPrfsChatAppearance->htmlFrame->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	QVBoxLayout *l = new QVBoxLayout( mPrfsChatAppearance->htmlFrame );

	preview = new KHTMLPart( mPrfsChatAppearance->htmlFrame, "preview" );
	preview->setJScriptEnabled( false ) ;
	preview->setJavaEnabled( false );
	preview->setPluginsEnabled( false );
	preview->setMetaRefreshEnabled( false );

	KHTMLView *htmlWidget = preview->view();
	htmlWidget->setMarginWidth(4);
	htmlWidget->setMarginHeight(4);
	htmlWidget->setFocusPolicy( NoFocus );
	htmlWidget->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );
	l->addWidget( htmlWidget );

	mAppearanceTabCtl->addTab( mPrfsChatAppearance, i18n("Chat &Appearance") );
	connect(mPrfsChatAppearance->cb_Kind, SIGNAL(activated(int)), this, SLOT(slotSelectKind(int)));
	connect(mPrfsChatAppearance->previewButton, SIGNAL(pressed()), this, SLOT(slotUpdatePreview()));

	// ===========================================================================

	reopen(); // load settings from config
	slotSoundChanged(); //Disable Button if no checkboxes selected
	slotTransparencyChanged(mPrfsChatWindow->mTransparencyEnabled->isChecked());

	// sync actions, config and prefs-dialog
	connect(KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()));
}

AppearanceConfig::~AppearanceConfig()
{
}

void AppearanceConfig::save()
{
//	kdDebug(14000) k_funcinfo << "called." << endl;
	KopetePrefs *p = KopetePrefs::prefs();

	// "General" TAB
	p->setStartDocked ( mPrfsGeneral->mStartDockedChk->isChecked() );
	p->setUseQueue ( mPrfsGeneral->mUseQueueChk->isChecked() );
	p->setTrayflashNotify ( mPrfsGeneral->mTrayflashNotifyChk->isChecked() );
	p->setBalloonNotify ( mPrfsGeneral->mBalloonNotifyChk->isChecked() );
	p->setBeepNotify ( mPrfsGeneral->mBeepNotifyChk->isChecked() );
	p->setSoundNotify ( mPrfsGeneral->mSoundNotifyChk->isChecked() );
	p->setSoundIfAway( mPrfsGeneral->mSoundIfAwayChk->isChecked() );

	// "Contact List" TAB
	p->setTreeView ( mPrfsContactlist->mTreeContactList->isChecked() );
	p->setShowOffline ( mPrfsContactlist->mShowOfflineUsers->isChecked() );
	p->setSortByGroup ( mPrfsContactlist->mSortByGroup->isChecked() );
	p->setGreyIdleMetaContacts( mPrfsContactlist->mGreyIdleMetaContacts->isChecked() );
	#if KDE_VERSION >= 306
		p->setNotifyOnline ( mPrfsContactlist->mNotifyOnlineUsers->isChecked() );
	#endif

	// Another TAB
	p->setIconTheme( icon_theme_list->currentText() );
	p->setUseEmoticons ( mUseEmoticonsChk->isChecked() );

	// "Chat Appearance" TAB
	p->setKindMessagesHtml ( mPrfsChatAppearance->mle_codehtml->text() );

	// "Chat Window" TAB
	p->setRaiseMsgWindow( mPrfsChatWindow->cb_RaiseMsgWindowChk->isChecked() );
	p->setShowEvents( mPrfsChatWindow->cb_ShowEventsChk->isChecked() );
	p->setChatWindowPolicy ( mPrfsChatWindow->chatWindowGroup->id(mPrfsChatWindow->chatWindowGroup->selected()) );
	p->setTransparencyColor( mPrfsChatWindow->mTransparencyTintColor->color() );
	p->setTransparencyEnabled( mPrfsChatWindow->mTransparencyEnabled->isChecked() );
	p->setTransparencyValue( mPrfsChatWindow->mTransparencyValue->value() );
// 	p->setCTransparencyColor( mPrfsChatWindow->mCTransparencyColor->color() );
// 	p->setCTransparencyEnabled( mPrfsChatWindow->mCTransparencyEnabled->isChecked() );
// 	p->setCTransparencyValue( mPrfsChatWindow->mCTransparencyValue->value() );
	p->setBgOverride( mPrfsChatWindow->mTransparencyBgOverride->isChecked() );


	// disconnect or else we will end up in an endless loop
	disconnect(KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()));
//	kdDebug(14000) << "[AppearanceConfig] calling KopetePrefs::save()" << endl;
	p->save();
	connect(KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()));
}

void AppearanceConfig::slotConfigChanged(void)
{
//	kdDebug(14000) << k_funcinfo << "calling reopen() now..." << endl;
	reopen(); // I am lazy :P
}

void AppearanceConfig::reopen()
{
//	kdDebug(14000) << k_funcinfo "called" << endl;
	KopetePrefs *p = KopetePrefs::prefs();

	// "General" TAB
	mPrfsGeneral->mStartDockedChk->setChecked( p->startDocked() );
	mPrfsGeneral->mUseQueueChk->setChecked( p->useQueue() );
	mPrfsGeneral->mTrayflashNotifyChk->setChecked ( p->trayflashNotify() );
	mPrfsGeneral->mBalloonNotifyChk->setChecked ( p->balloonNotify() );
	mPrfsGeneral->mSoundNotifyChk->setChecked ( p->soundNotify() );
	mPrfsGeneral->mBeepNotifyChk->setChecked ( p->beepNotify() );

	// "Contact List" TAB
	mPrfsContactlist->mTreeContactList->setChecked( p->treeView() );
	mPrfsContactlist->mSortByGroup->setChecked( p->sortByGroup() );
	mPrfsContactlist->mShowOfflineUsers->setChecked( p->showOffline() );
	mPrfsContactlist->mGreyIdleMetaContacts->setChecked( p->greyIdleMetaContacts() );
	#if KDE_VERSION >= 306
		mPrfsContactlist->mNotifyOnlineUsers->setChecked( p->notifyOnline() );
	#endif

	// "Emoticons" TAB
	KStandardDirs dir;
	icon_theme_list->clear(); // Wipe out old list
	// Get a list of directories in our icon theme dir
	QStringList themeDirs = KGlobal::dirs()->findDirs("data", "kopete/pics/emoticons");
	// loop adding themes from all dirs into theme-list
	for(unsigned int x = 0;x < themeDirs.count();x++)
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

	// "Chat Window" TAB
	mPrfsChatWindow->cb_RaiseMsgWindowChk->setChecked( p->raiseMsgWindow() );
	mPrfsChatWindow->cb_ShowEventsChk->setChecked( p->showEvents() );
	mPrfsChatWindow->chatWindowGroup->setButton( p->chatWindowPolicy() );
	mPrfsChatWindow->mTransparencyEnabled->setChecked( p->transparencyEnabled() );
	mPrfsChatWindow->mTransparencyTintColor->setColor( p->transparencyColor() );
	mPrfsChatWindow->mTransparencyValue->setValue( p->transparencyValue() );
	mPrfsChatWindow->mTransparencyBgOverride->setChecked( p->bgOverride() );

	// "Chat Appearance" TAB
	mPrfsChatAppearance->mle_codehtml->setText( p->kindMessagesHtml() );

	slotUpdatePreview();
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
	if ( mPrfsGeneral->mSoundNotifyChk->isChecked() )
		mPrfsGeneral->configSound->setEnabled(true);
	else
		mPrfsGeneral->configSound->setEnabled(false);
}

void AppearanceConfig::slotUseEmoticonsChanged ( bool checked )
{
	icon_theme_list->setEnabled( checked );
}

void AppearanceConfig::slotTransparencyChanged ( bool checked )
{
	mPrfsChatWindow->mTransparencyTintColor->setEnabled( checked );
	mPrfsChatWindow->mTransparencyValue->setEnabled( checked );
	mPrfsChatWindow->mTransparencyBgOverride->setEnabled( checked );
}

void AppearanceConfig::slotSelectKind(int k)
{
	if(k > 0)
	{
		QString model = KopetePrefs::KindMessagesHTML(k-1);
		mPrfsChatAppearance->mle_codehtml->setText( model );
	}
	slotUpdatePreview();
}

void AppearanceConfig::slotUpdatePreview()
{
	KopeteContact *cFrom = new KopeteContact(0L, QString::fromLatin1("UserFrom"), 0L);
	KopeteContact *cTo = new KopeteContact(0L, QString::fromLatin1("UserTo"), 0L);

	KopeteContactPtrList toList = KopeteContactPtrList();
	toList.append( cTo );

	KopeteMessage *msgIn = new KopeteMessage( cFrom, toList, QString::fromLatin1("This is an incoming message"),KopeteMessage::Inbound );
	KopeteMessage *msgOut = new KopeteMessage( cFrom, toList, QString::fromLatin1("This is an outgoing message"),KopeteMessage::Outbound );
	KopeteMessage *msgInt = new KopeteMessage( cFrom, toList, QString::fromLatin1("This is an internal message"),KopeteMessage::Internal );

	QString model = mPrfsChatAppearance->mle_codehtml->text();

	preview->begin();
	preview->write( QString::fromLatin1( "<html><body>" ) );

	// incoming messages
	preview->write( msgIn->transformMessage( model ) );
	msgIn->setFg(Qt::white);
	msgIn->setBg(Qt::blue);
	msgIn->setBody( QString::fromLatin1("This is a colored incoming message") );
	preview->write( msgIn->transformMessage( model ) );
	// -------------------

	// outgoing messages
	preview->write( msgOut->transformMessage( model ) );
	msgOut->setFg(Qt::white);
	msgOut->setBg(Qt::blue);
	msgOut->setBody( QString::fromLatin1("This is a colored outgoing message") );
	preview->write( msgOut->transformMessage( model ) );
	// -------------------

	// internal message
	preview->write( msgInt->transformMessage( model ) );

	preview->write( QString::fromLatin1( "</body></html>" ) );
	preview->end();

	delete msgIn;
	delete msgOut;
	delete msgInt;
	delete cFrom;
	delete cTo;
}

#include "appearanceconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

