/*
    pluginconfig.h  -  Kopete Plugin Module

    Copyright (c) 2001-2002 Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2001-2002 The Kopete developers  <kopete-devel@kde.org>

    Based on Noatun plugin selection code
    Copyright (c) 2000-2001 Charles Samuels        <charles@kde.org>
    Copyright (c) 2000-2001 Neil Stevens           <neil@qualityassistant.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef PLUGINCONFIG_H
#define PLUGINCONFIG_H

#include <klistview.h>

#include "configmodule.h"
#include "pluginloader.h"

class KTabCtl;
struct KopeteLibraryInfo;

class PluginListItem : public QCheckListItem
{
public:
	PluginListItem(const bool _exclusive, bool _checked, const KopeteLibraryInfo &_info, QListView *_parent);
	const KopeteLibraryInfo &info() const { return mInfo; }

	// This will toggle the state without "emitting" the stateChange
	void setChecked(bool);

protected:
	virtual void stateChange(bool);
	virtual void paintCell(QPainter *, const QColorGroup &, int, int, int);

private:
	KopeteLibraryInfo mInfo;
	bool silentStateChange;
	bool exclusive;
};

class PluginListView : public KListView
{
	Q_OBJECT

friend class PluginListItem;

public:
	PluginListView( QWidget *parent = 0, const char *name = 0 );

	virtual void clear();

signals:
	void stateChange(PluginListItem *, bool);

private:
	void stateChanged( PluginListItem *, bool );

	unsigned m_count;
};

class PluginConfig : public ConfigModule
{
	Q_OBJECT

public:
	PluginConfig(QObject *_parent = 0);
	virtual void save();
	virtual void reopen();

private slots:
	void stateChange(PluginListItem *, bool);

private:
	void addPlugin(const KopeteLibraryInfo &);
	void removePlugin(const KopeteLibraryInfo &);
	PluginListItem *findItem(const KopeteLibraryInfo &) const;

	QStringList mAdded, mDeleted;
	PluginListView *protocolList;
	PluginListView *otherList;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

