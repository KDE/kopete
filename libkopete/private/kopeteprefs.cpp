/*
    kopeteprefs.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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
	mShowOffline = config->readBoolEntry("ShowOfflineUsers", true);
	mShowEmptyGroups = config->readBoolEntry("ShowEmptyGroups", true);
	mGreyIdle = config->readBoolEntry("GreyIdleMetaContacts", true);
	mSortByGroup = config->readBoolEntry("SortByGroup" , true);
	mTreeView = config->readBoolEntry("TreeView", true);
	mStartDocked = config->readBoolEntry("StartDocked", false);
	mUseQueue = config->readBoolEntry("Use Queue", true);
	mRaiseMsgWindow = config->readBoolEntry("Raise Msg Window", false);
	mShowEvents = config->readBoolEntry("Show Events in Chat Window", true);
	mTrayflashNotify = config->readBoolEntry("Trayflash Notification", true);
	mBalloonNotify = config->readBoolEntry("Balloon Notification", true);
	mSoundIfAway = config->readBoolEntry("Sound Notification If Away", true);
	mChatWindowPolicy = config->readNumEntry("Chatwindow Policy", 0);
	mTransparencyEnabled = config->readBoolEntry("ChatView Transparency Enabled", false);
	mTransparencyValue = config->readNumEntry("ChatView Transparency Value", 50);
	mNotifyAway = config->readBoolEntry("Notification Away", false);
	mRichText = config->readBoolEntry("RichText editor", false);
	mChatWShowSend = config->readBoolEntry("Show Chatwindow Send Button", true);

	mTransparencyColor = config->readColorEntry("ChatView Transparency Tint Color", &Qt::white);
	mChatViewBufferSize = config->readNumEntry("ChatView BufferSize", 250);

	QColor tmpColor = KGlobalSettings::highlightColor();
	mHighlightBackground = config->readColorEntry("Highlight Background Color", &tmpColor);
	tmpColor = KGlobalSettings::highlightedTextColor();
	mHighlightForeground = config->readColorEntry("Highlight Foreground Color", &tmpColor);
	mHighlightEnabled = config->readBoolEntry("Highlighting Enabled", true);
	mBgOverride = config->readBoolEntry("ChatView Override Background", true);
	mInterfacePreference = config->readNumEntry("Interface Preference", 1);
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
	The stylesheet config value is now just the basename of the xsl file ie: Messenger, Kopete 
	T avoid having fallback duplicate code, I used the extract method pattern and left all in _setStyleSheet
	*/
	_setStyleSheet(config->readEntry("Stylesheet", QString::fromLatin1(KOPETE_DEFAULT_CHATSTYLE)));

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

	mWindowAppearanceChanged = false;
	mTransparencyChanged = false;
}

void KopetePrefs::save()
{
//	kdDebug(14010) << "KopetePrefs::save()" << endl;
	config->setGroup("Appearance");

	config->writeEntry("EmoticonTheme", mIconTheme);
	config->writeEntry("Use Emoticons", mUseEmoticons);
	config->writeEntry("ShowOfflineUsers", mShowOffline);
	config->writeEntry("ShowEmptyGroups", mShowEmptyGroups);
	config->writeEntry("GreyIdleMetaContacts", mGreyIdle);
	config->writeEntry("TreeView", mTreeView);
	config->writeEntry("SortByGroup", mSortByGroup);
	config->writeEntry("StartDocked", mStartDocked);
	config->writeEntry("Use Queue", mUseQueue);
	config->writeEntry("Raise Msg Window", mRaiseMsgWindow);
	config->writeEntry("Show Events in Chat Window", mShowEvents);
	config->writeEntry("Trayflash Notification", mTrayflashNotify);
	config->writeEntry("Balloon Notification", mBalloonNotify);
	config->writeEntry("Sound Notification If Away", mSoundIfAway);
	config->writeEntry("Notification Away", mNotifyAway);
	config->writeEntry("Chatwindow Policy", mChatWindowPolicy);
	config->writeEntry("ChatView Transparency Enabled", mTransparencyEnabled);
	config->writeEntry("ChatView Transparency Value", mTransparencyValue);
	config->writeEntry("ChatView Transparency Tint Color", mTransparencyColor);
	config->writeEntry("ChatView Override Background", mBgOverride);
	config->writeEntry("ChatView BufferSize", mChatViewBufferSize);
	config->writeEntry("Highlight Background Color", mHighlightBackground);
	config->writeEntry("Highlight Foreground Color", mHighlightForeground);
	config->writeEntry("Highlighting Enabled", mHighlightEnabled );
	config->writeEntry("Font Face", mFontFace);
	config->writeEntry("Text Color",mTextColor);
	config->writeEntry("Bg Color", mBgColor);
	config->writeEntry("Link Color", mLinkColor);
	config->writeEntry("Idle Contact Color", mIdleContactColor);
	config->writeEntry("RichText editor", mRichText);
	config->writeEntry("Show Chatwindow Send Button", mChatWShowSend);

	config->writeEntry("Interface Preference", mInterfacePreference);

	config->writeEntry("Show Systemtray", mShowTray);
	config->writeEntry("Stylesheet", mStyleSheet);

	config->writeEntry("ToolTipContents", mToolTipContents);

	config->sync();
	emit saved();

	if(mTransparencyChanged)
		emit(transparencyChanged());

	if(mWindowAppearanceChanged)
		emit(windowAppearanceChanged());

	mWindowAppearanceChanged = false;
	mTransparencyChanged = false;
}

void KopetePrefs::setIconTheme(const QString &value)
{
	mIconTheme = value;
}

void KopetePrefs::setUseEmoticons(bool value)
{
	mUseEmoticons = value;
}

void KopetePrefs::setShowOffline(bool value)
{
	mShowOffline = value;
}

void KopetePrefs::setShowEmptyGroups(bool value)
{
	mShowEmptyGroups = value;
}

void KopetePrefs::setTreeView(bool value)
{
	mTreeView = value;
}

void KopetePrefs::setSortByGroup(bool value)
{
	mSortByGroup = value;
}

void KopetePrefs::setGreyIdleMetaContacts(bool value)
{
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

void KopetePrefs::setRaiseMsgWindow(bool value)
{
	mRaiseMsgWindow = value;
}

void KopetePrefs::setShowEvents(bool value)
{
	mShowEvents = value;
}

void KopetePrefs::setTrayflashNotify(bool value)
{
	mTrayflashNotify = value;
}

void KopetePrefs::setBalloonNotify(bool value)
{
	mBalloonNotify = value;
}

void KopetePrefs::setSoundIfAway(bool value)
{
	mSoundIfAway = value;
}

void KopetePrefs::setStyleSheet(const QString &value)
{
	_setStyleSheet(value);
	emit( messageAppearanceChanged() );
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
	mStyleContents = fileContents(styleFileName);	
}

void KopetePrefs::setFontFace( const QFont &value )
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mFontFace);
	mFontFace = value;
}

void KopetePrefs::setTextColor( const QColor &value )
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mTextColor);
	mTextColor = value;
}

void KopetePrefs::setBgColor( const QColor &value )
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mBgColor);
	mBgColor = value;
}

void KopetePrefs::setLinkColor( const QColor &value )
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mLinkColor);
	mLinkColor = value;
}

void KopetePrefs::setChatWindowPolicy(int value)
{
	mChatWindowPolicy = value;
}

void KopetePrefs::setInterfacePreference(int value)
{
	mInterfacePreference = value;
}

void KopetePrefs::setTransparencyEnabled(bool value)
{
	mTransparencyChanged =  mTransparencyChanged || !(value == mTransparencyEnabled);
	mTransparencyEnabled = value;
}

void KopetePrefs::setTransparencyColor(const QColor &value)
{
	mTransparencyChanged =  mTransparencyChanged || !(value == mTransparencyColor);
	mTransparencyColor = value;
}

void KopetePrefs::setChatViewBufferSize( int value )
{
	mChatViewBufferSize = value;
}

void KopetePrefs::setHighlightBackground(const QColor &value)
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mHighlightBackground);
	mHighlightBackground = value;
}

void KopetePrefs::setHighlightForeground(const QColor &value)
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mHighlightForeground);
	mHighlightForeground = value;
}

void KopetePrefs::setHighlightEnabled(bool value)
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mHighlightEnabled);
	mHighlightEnabled = value;
}

void KopetePrefs::setTransparencyValue(int value)
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mTransparencyValue);
	mTransparencyValue = value;
}

void KopetePrefs::setBgOverride(bool value)
{
	mWindowAppearanceChanged = mWindowAppearanceChanged || !(value == mBgOverride);
	mBgOverride = value;
}

void KopetePrefs::setShowTray(bool value)
{
	mShowTray = value;
}

void KopetePrefs::setNotifyAway(bool value)
{
	mNotifyAway=value;
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

#include "kopeteprefs.moc"
// vim: set noet ts=4 sts=4 sw=4:
