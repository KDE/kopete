/*
    kopeteprefs.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteprefs.h"

#include <qfile.h>
#include <qfont.h>
#include <qmetaobject.h>

#include <kapplication.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#define KOPETE_DEFAULT_CHATSTYLE  "Kopete"

KopetePrefs *KopetePrefs::s_prefs = 0L;

KopetePrefs *KopetePrefs::prefs()
{
	if( !s_prefs )
		s_prefs = new KopetePrefs;
	return s_prefs;
}

KopetePrefs::KopetePrefs() : QObject( kapp, "KopetePrefs" )
{
	config = KGlobal::config();
	load();
}

void KopetePrefs::load()
{
//	kdDebug( 14010 ) << k_funcinfo << endl;
	config->setGroup("Appearance");

	mIconTheme = config->readEntry("EmoticonTheme", defaultTheme());
	mUseEmoticons = config->readBoolEntry("Use Emoticons", true);
	mEmoticonsRequireSpaces = config->readBoolEntry("EmoticonsRequireSpaces" , true );
	mShowOffline = config->readBoolEntry("ShowOfflineUsers", true);
	mShowEmptyGroups = config->readBoolEntry("ShowEmptyGroups", true);
	mGreyIdle = config->readBoolEntry("GreyIdleMetaContacts", true);
	mSortByGroup = config->readBoolEntry("SortByGroup" , true);
	mTreeView = config->readBoolEntry("TreeView", true);
	mStartDocked = config->readBoolEntry("StartDocked", false);
	mUseQueue = config->readBoolEntry("Use Queue", true);
	mUseStack = config->readBoolEntry("Use Stack", false);
	mRaiseMsgWindow = config->readBoolEntry("Raise Msg Window", false);
	mShowEvents = config->readBoolEntry("Show Events in Chat Window", true);
	mSpellCheck = config->readBoolEntry("SpellCheck", true);
	mQueueUnreadMessages = config->readBoolEntry("Queue Unread Messages", false);
	mQueueOnlyHighlightedMessagesInGroupChats = config->readBoolEntry("Queue Only Highlighted Messages In Group Chats", false);
	mQueueOnlyMessagesOnAnotherDesktop = config->readBoolEntry("Queue Only Messages On Another Desktop", false);
	mBalloonNotify = config->readBoolEntry("Balloon Notification", true);
	mBalloonNotifyIgnoreClosesChatView = config->readBoolEntry("Balloon Notification Ignore Closes Chat View", false);
	mBalloonCloseDelay = config->readNumEntry("Balloon Autoclose Delay", 30);
	mBalloonClose = config->readBoolEntry("Balloon Autoclose", false);
	mTrayflashNotify = config->readBoolEntry("Trayflash Notification", true);
	mTrayflashNotifyLeftClickOpensMessage = config->readBoolEntry("Trayflash Notification Left Click Opens Message", true);
	mTrayflashNotifySetCurrentDesktopToChatView = config->readBoolEntry("Trayflash Notification Set Current Desktop To Chat View", false);
	mSoundIfAway = config->readBoolEntry("Sound Notification If Away", true);
	mChatWindowPolicy = config->readNumEntry("Chatwindow Policy", 0);
	mTransparencyEnabled = config->readBoolEntry("ChatView Transparency Enabled", false);
	mTransparencyValue = config->readNumEntry("ChatView Transparency Value", 50);
	mRichText = config->readBoolEntry("RichText editor", false);
	mChatWShowSend = config->readBoolEntry("Show Chatwindow Send Button", true);
	mRememberedMessages = config->readNumEntry("Remembered Messages", 5);
	mTruncateContactNames = config->readBoolEntry("TruncateContactNames", false);
	mMaxContactNameLength = config->readNumEntry("MaxContactNameLength", 20);

	mTransparencyColor = config->readColorEntry("ChatView Transparency Tint Color", &Qt::white);
	mChatViewBufferSize = config->readNumEntry("ChatView BufferSize", 250);

	QColor tmpColor = KGlobalSettings::highlightColor();
	mHighlightBackground = config->readColorEntry("Highlight Background Color", &tmpColor);
	tmpColor = KGlobalSettings::highlightedTextColor();
	mHighlightForeground = config->readColorEntry("Highlight Foreground Color", &tmpColor);
	mHighlightEnabled = config->readBoolEntry("Highlighting Enabled", true);
	mBgOverride = config->readBoolEntry("ChatView Override Background", false);
	mFgOverride = config->readBoolEntry("ChatView Override Foreground", false);
	mRtfOverride = config->readBoolEntry("ChatView Override RTF", false);
	mInterfacePreference = config->readEntry("View Plugin", QString::fromLatin1("kopete_chatwindow") );
	tmpColor = KGlobalSettings::textColor();
	mTextColor = config->readColorEntry("Text Color", &tmpColor );
	tmpColor = KGlobalSettings::baseColor();
	mBgColor = config->readColorEntry("Bg Color", &tmpColor );
	tmpColor = KGlobalSettings::linkColor();
	mLinkColor = config->readColorEntry("Link Color", &tmpColor );
	mFontFace = config->readFontEntry("Font Face");
	tmpColor = darkGray;
	mIdleContactColor = config->readColorEntry("Idle Contact Color", &tmpColor);

	mShowTray = config->readBoolEntry("Show Systemtray", true);

	/*
	The stylesheet config value is now just the basename of the xsl file ie: Messenger, Kopete.
	To avoid having duplicated fallback code, I used the extract method refactoring and left all in
	_setStyleSheet.
	*/
	_setStyleSheet(config->readEntry("Stylesheet", QString::fromLatin1(KOPETE_DEFAULT_CHATSTYLE)));
	mStylePath = config->readEntry("StylePath"); 
	mStyleVariant = config->readEntry("StyleVariant");


	mToolTipContents = config->readListEntry("ToolTipContents");
	if(mToolTipContents.empty())
	{
		mToolTipContents
			<< QString::fromLatin1("FormattedName")
			<< QString::fromLatin1("userInfo")
			<< QString::fromLatin1("server")
			<< QString::fromLatin1("channels")
			<< QString::fromLatin1("FormattedIdleTime")
			<< QString::fromLatin1("channelMembers")
			<< QString::fromLatin1("channelTopic")
			<< QString::fromLatin1("emailAddress")
			<< QString::fromLatin1("homePage")
			<< QString::fromLatin1("onlineSince")
			<< QString::fromLatin1("lastOnline")
			<< QString::fromLatin1("awayMessage");
	}

	config->setGroup("ContactList");
	int n = metaObject()->findProperty( "contactListDisplayMode" );
	QString value = config->readEntry("DisplayMode",QString::fromLatin1("Default"));
	mContactListDisplayMode = (ContactDisplayMode)metaObject()->property( n )->keyToValue( value.latin1() );
	n = metaObject()->findProperty( "contactListIconMode" );
	value = config->readEntry("IconMode",
                                  QString::fromLatin1("IconDefault"));
	mContactListIconMode = (IconDisplayMode) metaObject()->property( n )->keyToValue( value.latin1() );
	mContactListIndentContacts = config->readBoolEntry("IndentContacts", false);
	mContactListHideVerticalScrollBar = config->readBoolEntry("HideVerticalScrollBar", false );
	mContactListUseCustomFonts = config->readBoolEntry("UseCustomFonts", false);
	QFont font = KGlobalSettings::generalFont();
	mContactListNormalFont = config->readFontEntry("NormalFont", &font);
	if ( font.pixelSize() != -1 )
		font.setPixelSize( (font.pixelSize() * 3) / 4 );
	else
		font.setPointSizeFloat( font.pointSizeFloat() * 0.75 );
	mContactListSmallFont = config->readFontEntry("SmallFont", &font);
	mContactListGroupNameColor = config->readColorEntry("GroupNameColor", &darkRed);
	mContactListAnimation = config->readBoolEntry("AnimateChanges", true);
	mContactListFading = config->readBoolEntry("FadeItems", true);
	mContactListFolding = config->readBoolEntry("FoldItems", true);
	mContactListMouseNavigation = config->readBoolEntry("MouseNavigation", false );
	mContactListAutoHide = config->readBoolEntry("AutoHide", false);
	mContactListAutoHideVScroll = config->readBoolEntry("AutoHideVScroll", true );
	mContactListAutoHideTimeout = config->readUnsignedNumEntry("AutoHideTimeout", 30);

	// Load the reconnection setting
	config->setGroup("General");
	mReconnectOnDisconnect = config->readBoolEntry("ReconnectOnDisconnect", true);
	mAutoConnect = config->readBoolEntry("AutoConnect", false);

	// Nothing has changed yet
	mWindowAppearanceChanged = false;
	mTransparencyChanged = false;
	mContactListAppearanceChanged = false;
	mMessageAppearanceChanged = false;
}

void KopetePrefs::save()
{
//	kdDebug(14010) << "KopetePrefs::save()" << endl;
	config->setGroup("Appearance");

	config->writeEntry("EmoticonTheme", mIconTheme);
	config->writeEntry("Use Emoticons", mUseEmoticons);
	config->writeEntry("EmoticonsRequireSpaces", mEmoticonsRequireSpaces);
	config->writeEntry("ShowOfflineUsers", mShowOffline);
	config->writeEntry("ShowEmptyGroups", mShowEmptyGroups);
	config->writeEntry("GreyIdleMetaContacts", mGreyIdle);
	config->writeEntry("TreeView", mTreeView);
	config->writeEntry("SortByGroup", mSortByGroup);
	config->writeEntry("StartDocked", mStartDocked);
	config->writeEntry("Use Queue", mUseQueue);
	config->writeEntry("Use Stack", mUseStack);
	config->writeEntry("Raise Msg Window", mRaiseMsgWindow);
	config->writeEntry("Show Events in Chat Window", mShowEvents);
	config->writeEntry("SpellCheck", mSpellCheck);
	config->writeEntry("Queue Unread Messages", mQueueUnreadMessages);
	config->writeEntry("Queue Only Highlighted Messages In Group Chats", mQueueOnlyHighlightedMessagesInGroupChats);
	config->writeEntry("Queue Only Messages On Another Desktop", mQueueOnlyMessagesOnAnotherDesktop);
	config->writeEntry("Balloon Notification", mBalloonNotify);
	config->writeEntry("Balloon Notification Ignore Closes Chat View", mBalloonNotifyIgnoreClosesChatView);
	config->writeEntry("Balloon Autoclose Delay", mBalloonCloseDelay);
	config->writeEntry("Balloon Autoclose", mBalloonClose);
	config->writeEntry("Trayflash Notification", mTrayflashNotify);
	config->writeEntry("Trayflash Notification Left Click Opens Message", mTrayflashNotifyLeftClickOpensMessage);
	config->writeEntry("Trayflash Notification Set Current Desktop To Chat View", mTrayflashNotifySetCurrentDesktopToChatView);
	config->writeEntry("Sound Notification If Away", mSoundIfAway);
	config->writeEntry("Chatwindow Policy", mChatWindowPolicy);
	config->writeEntry("ChatView Transparency Enabled", mTransparencyEnabled);
	config->writeEntry("ChatView Transparency Value", mTransparencyValue);
	config->writeEntry("ChatView Transparency Tint Color", mTransparencyColor);
	config->writeEntry("ChatView Override Background", mBgOverride);
	config->writeEntry("ChatView Override Foreground", mFgOverride);
	config->writeEntry("ChatView Override RTF", mRtfOverride);
	config->writeEntry("ChatView BufferSize", mChatViewBufferSize);
	config->writeEntry("Highlight Background Color", mHighlightBackground);
	config->writeEntry("Highlight Foreground Color", mHighlightForeground);
	config->writeEntry("Highlighting Enabled", mHighlightEnabled );
	config->writeEntry("Font Face", mFontFace);
	config->writeEntry("Text Color",mTextColor);
	config->writeEntry("Remembered Messages",mRememberedMessages);
	config->writeEntry("Bg Color", mBgColor);
	config->writeEntry("Link Color", mLinkColor);
	config->writeEntry("Idle Contact Color", mIdleContactColor);
	config->writeEntry("RichText editor", mRichText);
	config->writeEntry("Show Chatwindow Send Button", mChatWShowSend);
	config->writeEntry("TruncateContactNames", mTruncateContactNames);
	config->writeEntry("MaxContactNameLength", mMaxContactNameLength);

	config->writeEntry("View Plugin", mInterfacePreference);

	config->writeEntry("Show Systemtray", mShowTray);

	//Style
	//for XSLT
	config->writeEntry("Stylesheet", mStyleSheet);
	//for xhtml+css
	config->writeEntry("StylePath", mStylePath);
	config->writeEntry("StyleVariant", mStyleVariant);


	config->writeEntry("ToolTipContents", mToolTipContents);

	config->setGroup("ContactList");
	int n = metaObject()->findProperty( "contactListDisplayMode" );
	config->writeEntry("DisplayMode", metaObject()->property( n )->valueToKey( mContactListDisplayMode ));
	n = metaObject()->findProperty( "contactListIconMode" );
	config->writeEntry("IconMode", metaObject()->property( n )->valueToKey( mContactListIconMode ));
	config->writeEntry("IndentContacts", mContactListIndentContacts);
	config->writeEntry("HideVerticalScrollBar", mContactListHideVerticalScrollBar );
	config->writeEntry("UseCustomFonts", mContactListUseCustomFonts);
	config->writeEntry("NormalFont", mContactListNormalFont);
	config->writeEntry("SmallFont", mContactListSmallFont);
	config->writeEntry("GroupNameColor", mContactListGroupNameColor);
	config->writeEntry("AnimateChanges", mContactListAnimation);
	config->writeEntry("FadeItems", mContactListFading);
	config->writeEntry("FoldItems", mContactListFolding);
	config->writeEntry("MouseNavigation", mContactListMouseNavigation );
	config->writeEntry("AutoHide", mContactListAutoHide);
	config->writeEntry("AutoHideVScroll", mContactListAutoHideVScroll );
	config->writeEntry("AutoHideTimeout", mContactListAutoHideTimeout);

	//Save the reconnection setting
	config->setGroup("General");
	config->writeEntry("ReconnectOnDisconnect", mReconnectOnDisconnect);
	config->writeEntry("AutoConnect", mAutoConnect);

	config->sync();
	emit saved();

	if(mTransparencyChanged)
		emit transparencyChanged();

	if(mWindowAppearanceChanged)
		emit windowAppearanceChanged();

	if(mContactListAppearanceChanged)
		emit contactListAppearanceChanged();

	if(mMessageAppearanceChanged)
		emit messageAppearanceChanged();

	// Clear all *Changed flags. This will cause breakage if someone makes some
	// changes but doesn't save them in a slot connected to a *Changed signal.
	mWindowAppearanceChanged = false;
	mTransparencyChanged = false;
	mContactListAppearanceChanged = false;
	mMessageAppearanceChanged = false;
}

void KopetePrefs::setIconTheme(const QString &value)
{
	if( mIconTheme != value )
	{
		mMessageAppearanceChanged = true;
		mContactListAppearanceChanged = true;
	}
	mIconTheme = value;
}

void KopetePrefs::setUseEmoticons(bool value)
{
	if( mUseEmoticons != value )
	{
		 mMessageAppearanceChanged = true;
		 mContactListAppearanceChanged = true;
	}
	mUseEmoticons = value;
}

void KopetePrefs::setShowOffline(bool value)
{
	if( value != mShowOffline ) mContactListAppearanceChanged = true;
	mShowOffline = value;
}

void KopetePrefs::setShowEmptyGroups(bool value)
{
	if( value != mShowEmptyGroups ) mContactListAppearanceChanged = true;
	mShowEmptyGroups = value;
}

void KopetePrefs::setTreeView(bool value)
{
	if( value != mTreeView ) mContactListAppearanceChanged = true;
	mTreeView = value;
}

void KopetePrefs::setSortByGroup(bool value)
{
	if( value != mSortByGroup ) mContactListAppearanceChanged = true;
	mSortByGroup = value;
}

void KopetePrefs::setGreyIdleMetaContacts(bool value)
{
	if( value != mGreyIdle ) mContactListAppearanceChanged = true;
	mGreyIdle = value;
}

void KopetePrefs::setStartDocked(bool value)
{
	mStartDocked = value;
}

void KopetePrefs::setUseQueue(bool value)
{
	mUseQueue = value;
}

void KopetePrefs::setUseStack(bool value)
{
	mUseStack = value;
}


void KopetePrefs::setRaiseMsgWindow(bool value)
{
	mRaiseMsgWindow = value;
}

void KopetePrefs::setRememberedMessages(int value)
{
	mRememberedMessages = value;
}

void KopetePrefs::setShowEvents(bool value)
{
	mShowEvents = value;
}

void KopetePrefs::setTrayflashNotify(bool value)
{
	mTrayflashNotify = value;
}

void KopetePrefs::setSpellCheck(bool value)
{
	mSpellCheck = value;
}

void KopetePrefs::setQueueUnreadMessages(bool value)
{
	mQueueUnreadMessages = value;
}

void KopetePrefs::setQueueOnlyHighlightedMessagesInGroupChats(bool value)
{
	mQueueOnlyHighlightedMessagesInGroupChats = value;
}

void KopetePrefs::setQueueOnlyMessagesOnAnotherDesktop(bool value)
{
	mQueueOnlyMessagesOnAnotherDesktop = value;
}

void KopetePrefs::setTrayflashNotifyLeftClickOpensMessage(bool value)
{
	mTrayflashNotifyLeftClickOpensMessage = value;
}

void KopetePrefs::setTrayflashNotifySetCurrentDesktopToChatView(bool value)
{
	mTrayflashNotifySetCurrentDesktopToChatView = value;
}

void KopetePrefs::setBalloonNotify(bool value)
{
	mBalloonNotify = value;
}

void KopetePrefs::setBalloonNotifyIgnoreClosesChatView(bool value)
{
	mBalloonNotifyIgnoreClosesChatView = value;
}

void KopetePrefs::setBalloonClose( bool value ) 
{
	mBalloonClose = value;
}

void KopetePrefs::setBalloonDelay( int value )
{
	mBalloonCloseDelay = value;
}

void KopetePrefs::setSoundIfAway(bool value)
{
	mSoundIfAway = value;
}

void KopetePrefs::setStyleSheet(const QString &value)
{
	if( mStyleSheet != value ) mMessageAppearanceChanged = true;
	_setStyleSheet(value);
}

void KopetePrefs::_setStyleSheet(const QString &value)
{
	QString styleFileName  = locate( "appdata", QString::fromLatin1("styles/") + value + QString::fromLatin1(".xsl"));

	/* In case the user had selected a style not available now */
	if ( !QFile::exists(styleFileName) || value.isEmpty() )
	{
		/* Try to fallback to default style */
		mStyleSheet = QString::fromLatin1(KOPETE_DEFAULT_CHATSTYLE);
		// FIXME: Duncan: we could check here if Kopete XSL exists too and show a msgbox about a broken install in case it is not found
	}
	else
	{
		mStyleSheet = value;
	}
	
	styleFileName = locate( "appdata", QString::fromLatin1("styles/") + mStyleSheet + QString::fromLatin1(".xsl"));

	/* We must find the path of style data to replace $appdata by correct value in xsl that Kopete XSLT doesn't processing well (and impossible to fix without changing lots of things).  */
	mStyleDataPath = styleFileName;
	mStyleDataPath.replace(mStyleSheet + QString::fromLatin1(".xsl"),QString::fromLatin1("data/"));

	mStyleContents = fileContents(styleFileName);

}

void KopetePrefs::setFontFace( const QFont &value )
{
	if( value != mFontFace ) mWindowAppearanceChanged = true;
	mFontFace = value;
}

void KopetePrefs::setTextColor( const QColor &value )
{
	if( value != mTextColor ) mWindowAppearanceChanged = true;
	mTextColor = value;
}

void KopetePrefs::setBgColor( const QColor &value )
{
	if( value != mBgColor ) mWindowAppearanceChanged = true;
	mBgColor = value;
}

void KopetePrefs::setLinkColor( const QColor &value )
{
	if( value != mLinkColor ) mWindowAppearanceChanged = true;
	mLinkColor = value;
}

void KopetePrefs::setChatWindowPolicy(int value)
{
	mChatWindowPolicy = value;
}

void KopetePrefs::setTruncateContactNames( bool value )
{
	mTruncateContactNames = value;
}

void KopetePrefs::setMaxContactNameLength( int value )
{
	mMaxContactNameLength = value;
}

void KopetePrefs::setInterfacePreference(const QString &value)
{
	mInterfacePreference = value;
}

void KopetePrefs::setTransparencyEnabled(bool value)
{
	if( value != mTransparencyEnabled ) mTransparencyChanged = true;
	mTransparencyEnabled = value;
}

void KopetePrefs::setTransparencyColor(const QColor &value)
{
	if( value != mTransparencyColor ) mTransparencyChanged = true;
	mTransparencyColor = value;
}

void KopetePrefs::setChatViewBufferSize( int value )
{
	mChatViewBufferSize = value;
}

void KopetePrefs::setHighlightBackground(const QColor &value)
{
	if( value != mHighlightBackground ) mWindowAppearanceChanged = true;
	mHighlightBackground = value;
}

void KopetePrefs::setHighlightForeground(const QColor &value)
{
	if( value != mHighlightForeground ) mWindowAppearanceChanged = true;
	mHighlightForeground = value;
}

void KopetePrefs::setHighlightEnabled(bool value)
{
	if( value != mHighlightEnabled ) mWindowAppearanceChanged = true;
	mHighlightEnabled = value;
}

void KopetePrefs::setTransparencyValue(int value)
{
	if( value != mTransparencyValue ) mTransparencyChanged = true;
	mTransparencyValue = value;
}

void KopetePrefs::setBgOverride(bool value)
{
	if( value != mBgOverride ) mMessageAppearanceChanged = true;
	mBgOverride = value;
}

void KopetePrefs::setFgOverride(bool value)
{
	if( value != mFgOverride ) mMessageAppearanceChanged = true;
	mFgOverride = value;
}

void KopetePrefs::setRtfOverride(bool value)
{
	if( value != mRtfOverride ) mMessageAppearanceChanged = true;
	mRtfOverride = value;
}

void KopetePrefs::setShowTray(bool value)
{
	mShowTray = value;
}


QString KopetePrefs::fileContents(const QString &path)
{
 	QString contents;
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
		QTextStream stream( &file );
		contents = stream.read();
		file.close();
	}
	return contents;
}

void KopetePrefs::setIdleContactColor(const QColor &value)
{
	if( value != mIdleContactColor ) mContactListAppearanceChanged = true;
	mIdleContactColor = value;
}

void KopetePrefs::setRichText(bool value)
{
	mRichText=value;
}

void KopetePrefs::setToolTipContents(const QStringList &value)
{
	mToolTipContents=value;
}

void KopetePrefs::setContactListIndentContacts( bool v )
{
	if( v != mContactListIndentContacts ) mContactListAppearanceChanged = true;
	mContactListIndentContacts = v;
}

void KopetePrefs::setContactListHideVerticalScrollBar( bool v )
{
	if( v != mContactListHideVerticalScrollBar ) mContactListAppearanceChanged = true;
	mContactListHideVerticalScrollBar = v;
}

void KopetePrefs::setContactListDisplayMode( ContactDisplayMode v )
{
	if( v != mContactListDisplayMode ) mContactListAppearanceChanged = true;
	mContactListDisplayMode = v;
}

void KopetePrefs::setContactListIconMode( IconDisplayMode v )
{
	if( v != mContactListIconMode ) mContactListAppearanceChanged = true;
	mContactListIconMode = v;
}

void KopetePrefs::setContactListUseCustomFonts( bool v )
{
	if( v != mContactListUseCustomFonts ) mContactListAppearanceChanged = true;
	mContactListUseCustomFonts = v;
}

void KopetePrefs::setContactListCustomNormalFont( const QFont & v )
{
	if( v != mContactListNormalFont ) mContactListAppearanceChanged = true;
	mContactListNormalFont = v;
}

void KopetePrefs::setContactListCustomSmallFont( const QFont & v )
{
	if( v != mContactListSmallFont ) mContactListAppearanceChanged = true;
	mContactListSmallFont = v;
}

QFont KopetePrefs::contactListSmallFont() const
{
	if ( mContactListUseCustomFonts )
		return contactListCustomSmallFont();
	QFont smallFont = KGlobalSettings::generalFont();
	if ( smallFont.pixelSize() != -1 )
		smallFont.setPixelSize( (smallFont.pixelSize() * 3) / 4 );
	else
		smallFont.setPointSizeFloat( smallFont.pointSizeFloat() * 0.75 );
	return smallFont;
}

void KopetePrefs::setContactListGroupNameColor( const QColor & v )
{
	if( v != mContactListGroupNameColor ) mContactListAppearanceChanged = true;
	mContactListGroupNameColor = v;
}

void KopetePrefs::setContactListAnimation( bool n )
{
	if( n != mContactListAnimation ) mContactListAppearanceChanged = true;
	mContactListAnimation = n;
}

void KopetePrefs::setContactListFading( bool n )
{
	if( n != mContactListFading ) mContactListAppearanceChanged = true;
	mContactListFading = n;
}

void KopetePrefs::setContactListFolding( bool n )
{
	if( n != mContactListFolding ) mContactListAppearanceChanged = true;
	mContactListFolding = n;
}

void KopetePrefs::setContactListMouseNavigation( bool n )
{
	if( n != mContactListMouseNavigation ) mContactListAppearanceChanged = true;
	mContactListMouseNavigation = n;
}

void KopetePrefs::setContactListAutoHide( bool n )
{
	if( n != mContactListAutoHide ) mContactListAppearanceChanged = true;
	mContactListAutoHide = n;
}

void KopetePrefs::setContactListAutoHideVScroll( bool n )
{
	if( n != mContactListAutoHideVScroll ) mContactListAppearanceChanged = true;
	mContactListAutoHideVScroll = n;
}

void KopetePrefs::setContactListAutoHideTimeout( unsigned int n )
{
	if( n != mContactListAutoHideTimeout ) mContactListAppearanceChanged = true;
	mContactListAutoHideTimeout = n;
}

void KopetePrefs::setReconnectOnDisconnect( bool newSetting )
{
	mReconnectOnDisconnect = newSetting;
}

void KopetePrefs::setAutoConnect(bool b)
{
	mAutoConnect=b;
}

void KopetePrefs::setEmoticonsRequireSpaces( bool b )
{
	if( mEmoticonsRequireSpaces != b )
	{
		 mMessageAppearanceChanged = true;
		 mContactListAppearanceChanged = true;
	}
	mEmoticonsRequireSpaces=b;
}

#include "kopeteprefs.moc"
// vim: set noet ts=4 sts=4 sw=4:
