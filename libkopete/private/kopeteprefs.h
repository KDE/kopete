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

#ifndef __KOPETEPREFS_H__
#define __KOPETEPREFS_H__

#include <qobject.h>
#include <kdeversion.h>
#include <qcolor.h>
#include <qfont.h>

class KConfig;

class KopetePrefs : public QObject
{
	Q_OBJECT
	// here so we can use Qt to translate enums<-->strings
	Q_PROPERTY( ContactDisplayMode contactListDisplayMode READ contactListDisplayMode WRITE setContactListDisplayMode )
	Q_ENUMS( ContactDisplayMode )

public:
	/**
	 * The prefs container-class is a singleton object. Use this method to retrieve
	 * the instance.
	 */
	static KopetePrefs *prefs();

	/**
	 * Reads all pref-variables from KConfig
	 * usually you don't need this as KopetePrefs loads settings
	 * when an instance is created
	 */
	void load();

	/**
	 * Stores all pref-variables into KConfig
	 */
	void save();

	QString iconTheme() const { return mIconTheme; }
	bool useEmoticons() const { return mUseEmoticons; }
	bool showOffline() const { return mShowOffline; }
	bool showEmptyGroups() const { return mShowEmptyGroups; }
	bool treeView() const { return mTreeView; }
	bool sortByGroup() const { return mSortByGroup; }
	bool greyIdleMetaContacts() const { return mGreyIdle; }
	bool startDocked() const { return mStartDocked; }
	bool useQueue() const { return mUseQueue; }
	bool raiseMsgWindow() const{ return mRaiseMsgWindow; }
	bool showEvents() const{ return mShowEvents; }
	bool trayflashNotify() const { return mTrayflashNotify; }
	bool balloonNotify() const { return mBalloonNotify; }
	bool soundIfAway() const { return mSoundIfAway; }
	bool transparencyEnabled() const { return mTransparencyEnabled; }
	int transparencyValue() const { return mTransparencyValue; }
	QColor transparencyColor() const { return mTransparencyColor; }
	int chatViewBufferSize() const { return mChatViewBufferSize; }
	int rememberedMessages() const { return mRememberedMessages; }
	const QColor &highlightBackground() const { return mHighlightBackground; }
	const QColor &highlightForeground() const { return mHighlightForeground; }
	const QColor &textColor() const { return mTextColor; }
	const QColor &bgColor() const { return mBgColor; }
	const QColor &linkColor() const { return mLinkColor; }
	const QFont &fontFace() const { return mFontFace; }
	const QColor &idleContactColor() const { return mIdleContactColor; }
	bool highlightEnabled() const { return mHighlightEnabled; }
	bool bgOverride() const { return mBgOverride; }

	enum ChatWindowPref { EmailWindow=0, ChatWindow };
	ChatWindowPref interfacePreference() const { return mInterfacePreference; }
	bool showTray() const { return mShowTray; }
	bool notifyAway() const { return mNotifyAway; }
	bool richText() const { return mRichText; }
	bool chatWShowSend() const { return mChatWShowSend; }

	int chatWindowPolicy() const { return mChatWindowPolicy; }
	QString styleSheet() const { return mStyleSheet; }
	QString styleContents() const { return mStyleContents; }
	QString defaultTheme() const { return QString::fromLatin1("Default"); }

	QStringList toolTipContents() const { return mToolTipContents; }

	///
	enum ContactDisplayMode { Classic, RightAligned, Detailed, Yagami, Default = Classic };
	bool contactListIndentContacts() const { return mContactListIndentContacts; }
	ContactDisplayMode contactListDisplayMode() const { return mContactListDisplayMode; }
	bool contactListUseCustomFonts() const { return mContactListUseCustomFonts; }
	QFont contactListCustomNormalFont() const { return mContactListNormalFont; }
	QFont contactListCustomSmallFont() const { return mContactListSmallFont; }
	QFont contactListSmallFont() const;
	QColor contactListGroupNameColor() const { return mContactListGroupNameColor; }

	void setIconTheme(const QString &value);
	void setUseEmoticons(bool value);
	void setShowOffline(bool value);
	void setShowEmptyGroups(bool value);
	void setTreeView(bool);
	void setSortByGroup(bool);
	void setGreyIdleMetaContacts(bool);
	void setStartDocked(bool);
	void setUseQueue(bool);
	void setRaiseMsgWindow(bool);
	void setShowEvents(bool);
	void setTrayflashNotify(bool);
	void setBalloonNotify(bool);
	void setSoundIfAway(bool);
	void setBeepNotify(bool);
	void setChatWindowPolicy(int);
	void setStyleSheet (const QString &);
	void setTransparencyEnabled(bool);
	void setTransparencyColor(const QColor &);
	void setChatViewBufferSize(int);
	void setHighlightBackground(const QColor &);
	void setHighlightForeground(const QColor &);
	void setHighlightEnabled(bool);
	void setTransparencyValue(int);
	void setBgOverride(bool);
	void setInterfacePreference(ChatWindowPref);
	void setTextColor(const QColor &);
	void setBgColor(const QColor &);
	void setLinkColor(const QColor &);
	void setFontFace(const QFont &);
	void setIdleContactColor(const QColor &);
	void setShowTray(bool);
	void setNotifyAway(bool);
	void setRichText(bool);
	void setRememberedMessages(int);
	void setToolTipContents(const QStringList &);
	void setContactListIndentContacts( bool v );
	void setContactListDisplayMode( ContactDisplayMode v );
	void setContactListUseCustomFonts( bool v );
	void setContactListCustomNormalFont( const QFont & v );
	void setContactListCustomSmallFont( const QFont & v );
	void setContactListGroupNameColor( const QColor & v );

signals:
	/**
	 * Emitted when config gets saved by save()
	 */
	void saved();
	/**
	 * Emitted when config gets saved by save() and a certain
	 * setting has changed.
	 * Naming scheme is the same as with the config vars.
	 */
	void windowAppearanceChanged();
	void messageAppearanceChanged();
	void contactListAppearanceChanged();
	void transparencyChanged();

private:
	/**
	 * Private constructor: we are a singleton
	 */
	KopetePrefs();

	/**
	 * Our instance
	 */
	static KopetePrefs *s_prefs;

	KConfig *config;

	QString mIconTheme;
	bool mUseEmoticons;
	bool mShowOffline;
	bool mShowEmptyGroups;
	bool mGreyIdle;
	bool mTreeView;
	bool mSortByGroup;
	bool mStartDocked;
	bool mUseQueue;
	bool mRaiseMsgWindow;
	bool mShowEvents;
	bool mTrayflashNotify;
	bool mBalloonNotify;
	bool mSoundIfAway;
	bool mTransparencyEnabled;
	int mTransparencyValue;
	int mRememberedMessages;
	ChatWindowPref mInterfacePreference;
	QColor mTransparencyColor;
	int mChatViewBufferSize;
	QColor mHighlightBackground;
	QColor mHighlightForeground;
	QColor mTextColor;
	QColor mBgColor;
	QColor mLinkColor;
	QFont mFontFace;
	QColor mIdleContactColor;
	bool mHighlightEnabled;
	bool mBgOverride;
	bool mShowTray;
	bool mTransparencyChanged;
	bool mWindowAppearanceChanged;
	bool mContactListAppearanceChanged;
	bool mNotifyAway;
	bool mChatWShowSend;

	int mChatWindowPolicy;

	bool mRichText;
	QString mStyleSheet;
	QString mStyleContents;

	QStringList mToolTipContents;

	bool mContactListIndentContacts;
	ContactDisplayMode mContactListDisplayMode;
	bool mContactListUseCustomFonts;
	QFont mContactListNormalFont;
	QFont mContactListSmallFont;
	QColor mContactListGroupNameColor;

	QString fileContents(const QString &path);
	void _setStyleSheet (const QString &);
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
