/*
    kopeteprefs.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002      by Stefan Gehn            <sgehn@gmx.net>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteprefs.h"

#include <kapplication.h>
#include <klocale.h>
#include <qcolor.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
//#include <kopetechatwindow.h> // FIXME: I do not like this dependency; mETz

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

KopetePrefs::~KopetePrefs()
{
//	save(); // crashes because KopeteApp is already destructed!
}

void KopetePrefs::load()
{

//	kdDebug(14010) << "KopetePrefs::load()" << endl;
	config->setGroup("Appearance");

	mIconTheme		= config->readEntry("EmoticonTheme", defaultTheme());
	mUseEmoticons		= config->readBoolEntry("Use Emoticons", true);
	mShowOffline		= config->readBoolEntry("ShowOfflineUsers", true);
	mGreyIdle		= config->readBoolEntry("GreyIdleMetaContacts", true);
	mSortByGroup		= config->readBoolEntry("SortByGroup" , true);
	mTreeView		= config->readBoolEntry("TreeView", true);
	mStartDocked		= config->readBoolEntry("StartDocked", false);
	mUseQueue		= config->readBoolEntry("Use Queue", true);
	mRaiseMsgWindow		= config->readBoolEntry("Raise Msg Window", false);
	mShowEvents		= config->readBoolEntry("Show Events in Chat Window", true);
	mTrayflashNotify	= config->readBoolEntry("Trayflash Notification", true);
	mBalloonNotify		= config->readBoolEntry("Balloon Notification", true);
	mSoundIfAway		= config->readBoolEntry("Sound Notification If Away", false);
	mChatWindowPolicy	= config->readNumEntry("Chatwindow Policy", 0);
	mTransparencyEnabled	= config->readBoolEntry("ChatView Transparency Enabled", false);
	mTransparencyValue	= config->readNumEntry("ChatView Transparency Value", 50);

	mTransparencyColor	= config->readColorEntry("ChatView Transparency Tint Color", &Qt::white);
	mChatViewBufferSize	= config->readNumEntry("ChatView BufferSize", 250);

	mHighlightBackground	= config->readColorEntry("Highlight Background Color", &Qt::black);
	mHighlightForeground	= config->readColorEntry("Highlight Foreground Color", &Qt::yellow);
	mHighlightEnabled	= config->readBoolEntry("Highlighting Enabled", true);
	mBgOverride		= config->readBoolEntry("ChatView Override Background", true);
	mInterfacePreference	= config->readNumEntry("Interface Preference", 1);
	mTextColor		= config->readColorEntry("Text Color", &Qt::black );
	mBgColor		= config->readColorEntry("Bg Color", &Qt::white );
	mLinkColor		= config->readColorEntry("Link Color", &Qt::blue );
	mFontFace		= config->readFontEntry("Font Face");

	mShowTray		= config->readBoolEntry( "Show Systemtray", true);
	mKindMessagesHtml    	= config->readEntry("MessagesHTML", QString::null );

	config->setGroup( QString::fromLatin1("XSL Chat Styles") );
	QString keyBase = QString::fromLatin1("Style");
	int loopPosition = 0;
	QString styleNum = keyBase + QString::number(loopPosition);
	while( config->hasKey( styleNum + QString::fromLatin1(" Name") ) )
	{
		QString styleName = config->readEntry( styleNum + QString::fromLatin1(" Name") );
		QString styleCode = config->readEntry( styleNum + QString::fromLatin1(" Code") );
		loopPosition++;
		styleNum = keyBase + QString::number(loopPosition);
		mChatStyles.insert(styleName, styleCode );
	}

	if( mChatStyles.isEmpty() )
	{
		QString start = QString::fromLatin1( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n<xsl:output method=\"html\"/>\n<xsl:template match=\"message\">\n" );
		QString end =   QString::fromLatin1( "\n</xsl:template>\n</xsl:stylesheet>" );

  		QString msnStyle = start + QString::fromLatin1(
			"<div style=\"padding-bottom:10px;\" class=\"KopeteMessage\"><xsl:attribute name=\"id\"><xsl:value-of select=\"@id\"/></xsl:attribute>\n"
			"\t<div style=\"color:lightgray\">\n"
			"\t\t(<xsl:value-of select=\"@timestamp\"/>) <span class=\"KopeteDisplayName\"><xsl:value-of select=\"from/contact/@metaContactDisplayName\"/></span> says:\n"
			"\t</div>\n"
			"\t<xsl:text disable-output-escaping=\"yes\">&#160;&#160;&#160;&#160;</xsl:text>\n"
			"\t<span>\n"
			"\t<xsl:attribute name=\"style\">\t\t<xsl:if test=\"body/@color\"><xsl:text>color:</xsl:text><xsl:value-of select=\"body/@color\"/><xsl:text>;</xsl:text></xsl:if><xsl:if test=\"body/@bgcolor\"><xsl:text>background-color:</xsl:text><xsl:value-of select=\"body/@bgcolor\"/></xsl:if></xsl:attribute>\n"
			"\t<xsl:if test=\"@importance='2'\">\n\t\t<xsl:attribute name=\"class\"><xsl:text>highlight</xsl:text></xsl:attribute>\n"
			"\t</xsl:if>\n<xsl:value-of disable-output-escaping=\"yes\" select=\"body\"/></span>\n</div>"
		) + end;

		QString xchatStyle = start + QString::fromLatin1(
			"<div class=\"KopeteMessage\"><xsl:attribute name=\"id\"><xsl:value-of select=\"@id\"/></xsl:attribute>\n"
			"<xsl:attribute name=\"style\">\t\t<xsl:if test=\"body/@color\"><xsl:text>color:</xsl:text><xsl:value-of select=\"body/@color\"/><xsl:text>;</xsl:text></xsl:if><xsl:if test=\"body/@bgcolor\"><xsl:text>background-color:</xsl:text><xsl:value-of select=\"body/@bgcolor\"/></xsl:if></xsl:attribute>\n"
			"\n<xsl:if test=\"@direction='3'\">\n"
			"\t<xsl:attribute name=\"style\"><xsl:text>color:darkgreen</xsl:text></xsl:attribute>\n"
			"</xsl:if>\n[<xsl:value-of select=\"@timestamp\"/>] \n"
			"<!-- Choose based on message direction -->\n<xsl:choose>\n"
			"\t<xsl:when test=\"@direction='2'\"><!--internal message-->\n"
			"\t\t-<font color=\"cyan\">--</font>\n\t</xsl:when>\n"
			"\t<xsl:when test=\"@direction='3'\"><!--action message-->\n"
			"\t\t<span class=\"KopeteDisplayName\"><xsl:value-of select=\"from/contact/@metaContactDisplayName\"/></span><xsl:text disable-output-escaping=\"yes\">&#160;</xsl:text>\n"
			"\t</xsl:when>\n\t<xsl:otherwise>\n\t\t<font color=\"blue\">&lt;</font>\n"
			"\t\t<font>\n\t\t\t<xsl:attribute name=\"color\">\n"
			"\t\t\t\t<xsl:choose>\n\t\t\t\t\t<xsl:when test=\"@direction='1'\"> <!-- Outgoing -->\n"
			"\t\t\t\t\t\t<xsl:text>yellow</xsl:text>\n\t\t\t\t\t</xsl:when>\n"
			"\t\t\t\t\t<xsl:otherwise> <!-- Incoming -->\n\t\t\t\t\t\t<xsl:value-of select=\"from/contact/@color\"/>\n"
			"\t\t\t\t\t</xsl:otherwise>\n\t\t\t\t</xsl:choose>\n\t\t\t</xsl:attribute>\n"
			"\t\t\t<span class=\"KopeteDisplayName\"><xsl:value-of select=\"from/contact/@metaContactDisplayName\"/></span>\n\t\t</font>\n"
			"\t\t<font color=\"blue\">&gt; </font>\n\t</xsl:otherwise>\n</xsl:choose>\n<span>\n"
			"<xsl:if test=\"@importance='2'\">\n"
			"\t<xsl:attribute name=\"class\"><xsl:text>KopeteMessage highlight</xsl:text></xsl:attribute>\n"
			"</xsl:if>\n<xsl:value-of disable-output-escaping=\"yes\" select=\"body\"/>\n</span>\n</div>"
		 ) + end;

		QString kopeteStyle = start + QString::fromLatin1(
			"<div class=\"KopeteMessage\" style=\"padding-bottom:10px;\"><xsl:attribute name=\"id\"><xsl:value-of select=\"@id\"/></xsl:attribute>\n"
			"\t<xsl:if test=\"@direction='0' or @direction='1'\">\n"
			"\t\t<div>\n"
			"\t\t\t<xsl:choose>\n"
			"\t\t\t<xsl:when test=\"@direction='1'\">\n"
			"\t\t\t\t<xsl:attribute name=\"style\"><xsl:text>color:red;font-weight:bold;</xsl:text></xsl:attribute>\n"
			"\t\t\t\t<xsl:text>Message to </xsl:text><span class=\"KopeteDisplayName\"><xsl:value-of select=\"to/contact/@metaContactDisplayName\"/></span>\n"
			"\t\t\t</xsl:when>\n"
			"\t\t\t<xsl:otherwise>\n"
			"\t\t\t\t<xsl:attribute name=\"style\"><xsl:text>color:blue;font-weight:bold;</xsl:text></xsl:attribute>\n"
			"\t\t\t\t<xsl:text>Message from </xsl:text><span class=\"KopeteDisplayName\"><xsl:value-of select=\"from/contact/@metaContactDisplayName\"/></span>\n"
			"\t\t\t</xsl:otherwise>\n"
			"\t\t\t</xsl:choose>\n"
			"\t\t\t<xsl:text> at </xsl:text><xsl:value-of select=\"@timestamp\"/>\n"
			"\t\t</div>\n"
			"\t\t<xsl:text disable-output-escaping=\"yes\">&#160;&#160;&#160;&#160;</xsl:text>\n"
			"\t</xsl:if>\n"
			"\t<span>\n"
			"\t<xsl:attribute name=\"style\">\t\t<xsl:if test=\"body/@color\"><xsl:text>color:</xsl:text><xsl:value-of select=\"body/@color\"/><xsl:text>;</xsl:text></xsl:if><xsl:if test=\"body/@bgcolor\"><xsl:text>background-color:</xsl:text><xsl:value-of select=\"body/@bgcolor\"/></xsl:if></xsl:attribute>\n"
			"\t\t<xsl:if test=\"@importance='2'\">\n"
			"\t\t\t<xsl:attribute name=\"class\"><xsl:text>highlight</xsl:text></xsl:attribute>\n"
			"\t\t</xsl:if>\n"
			"\t\t<xsl:choose>\n"
			"\t\t<xsl:when test=\"@direction='3'\">\n\t\t\t<span style=\"color:darkgreen\">\n"
			"\t\t\t\t<span class=\"KopeteDisplayName\"><xsl:value-of select=\"from/contact/@metaContactDisplayName\"/></span> <xsl:value-of disable-output-escaping=\"yes\" select=\"body\"/>\n"
			"\t\t\t</span>\n"
			"\t\t</xsl:when>\n"
			"\t\t<xsl:otherwise>\n"
			"\t\t\t<xsl:value-of disable-output-escaping=\"yes\" select=\"body\"/>\n"
			"\t\t</xsl:otherwise>\n"
			"\t\t</xsl:choose>\n\t</span>\n</div>"
		) + end;

		mChatStyles.insert( i18n("MSN Style"), msnStyle );
		mChatStyles.insert( i18n("XChat Style"), xchatStyle );
		mChatStyles.insert( i18n("Kopete Style"), kopeteStyle );

		if( mKindMessagesHtml.isNull() )
			mKindMessagesHtml = kopeteStyle;
		else if( mKindMessagesHtml != xchatStyle && mKindMessagesHtml != msnStyle && mKindMessagesHtml != kopeteStyle )
			mChatStyles.insert( i18n("Personal Style"), mKindMessagesHtml );
	}

	config->setGroup("Appearance");
	mTransparancyChanged = false;
}


void KopetePrefs::save()
{
//	kdDebug(14010) << "KopetePrefs::save()" << endl;

	config->setGroup ( "Appearance");

	config->writeEntry("EmoticonTheme", mIconTheme);
	config->writeEntry("Use Emoticons", mUseEmoticons);
	config->writeEntry("ShowOfflineUsers", mShowOffline);
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

	config->writeEntry("Interface Preference", mInterfacePreference);

//	config->writeEntry("ContactList Transparency Enabled", mCTransparencyEnabled);
//	config->writeEntry("ContactList Transparency Value", mCTransparencyValue);
//	config->writeEntry("ContactList Transparency Tint Color", mCTransparencyColor);

	config->writeEntry("Show Systemtray", mShowTray);
	config->writeEntry("MessagesHTML", mKindMessagesHtml);

	config->deleteGroup( QString::fromLatin1("XSL Chat Styles") );
	config->setGroup( QString::fromLatin1("XSL Chat Styles") );
	QString keyBase = QString::fromLatin1("Style");
	int loopPosition = 0;
	for( KopeteChatStyleMap::Iterator it = mChatStyles.begin(); it != mChatStyles.end(); ++it)
	{
		QString scriptNum = keyBase + QString::number( loopPosition );
		config->writeEntry( scriptNum + QString::fromLatin1(" Name"), it.key() );
		config->writeEntry( scriptNum + QString::fromLatin1(" Code"), it.data() );
		loopPosition++;
	}

	config->sync();
	emit saved();

	if( mTransparancyChanged )
		emit( transparancyChanged() );

	mTransparancyChanged = false;
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

void KopetePrefs::setKindMessagesHtml(const QString &value)
{
	if( mAppearanceChanged || mKindMessagesHtml != value )
	{
		mKindMessagesHtml = value;
		emit( messageAppearanceChanged() );
	}
}

void KopetePrefs::setChatStyles( const KopeteChatStyleMap &value )
{
	mChatStyles = value;
}

void KopetePrefs::setFontFace( const QFont &value )
{
	mAppearanceChanged = mAppearanceChanged || !(value == mFontFace);
	mFontFace = value;
}

void KopetePrefs::setTextColor( const QColor &value )
{
	mAppearanceChanged = mAppearanceChanged || !(value == mTextColor);
	mTextColor = value;
}

void KopetePrefs::setBgColor( const QColor &value )
{
	mAppearanceChanged = mAppearanceChanged || !(value == mBgColor);
	mBgColor = value;
}

void KopetePrefs::setLinkColor( const QColor &value )
{
	mAppearanceChanged = mAppearanceChanged || !(value == mLinkColor);
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
	mTransparancyChanged =  mTransparancyChanged || !(value == mTransparencyEnabled);
	mTransparencyEnabled = value;
}

void KopetePrefs::setTransparencyColor(const QColor &value)
{
	mTransparancyChanged =  mTransparancyChanged || !(value == mTransparencyColor);
	mTransparencyColor = value;
}

void KopetePrefs::setChatViewBufferSize(const int value)
{
	mChatViewBufferSize = value;
}

void KopetePrefs::setHighlightBackground(const QColor &value)
{
	mAppearanceChanged = mAppearanceChanged || !(value == mHighlightBackground);
	mHighlightBackground = value;
}

void KopetePrefs::setHighlightForeground(const QColor &value)
{
	mAppearanceChanged = mAppearanceChanged || !(value == mHighlightForeground);
	mHighlightForeground = value;
}

void KopetePrefs::setHighlightEnabled(bool value)
{
	mAppearanceChanged = mAppearanceChanged || !(value == mHighlightEnabled);
	mHighlightEnabled = value;
}

void KopetePrefs::setTransparencyValue(int value)
{
	mTransparancyChanged = mTransparancyChanged || !(value == mTransparencyValue);
	mTransparencyValue = value;
}

void KopetePrefs::setBgOverride(bool value)
{
	mAppearanceChanged = mAppearanceChanged || !(value == mBgOverride);
	mBgOverride = value;
}

void KopetePrefs::setShowTray(bool value)
{
	mShowTray = value;
}


#include "kopeteprefs.moc"

// vim: set noet ts=4 sts=4 sw=4:
