/***************************************************************************
                          highlightplugin.h  -  description
                             -------------------
    begin                : mar 14 2003
    copyright            : (C) 2003 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HighlightPLUGIN_H
#define HighlightPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
class QString;
class QTimer;
class KListAction;

class KopeteMessage;
class KopeteMetaContact;
class KopeteMessageManager;

class HighlightPreferences;
class Filter;

/**
  * @author Olivier Goffart
  */

class HighlightPlugin : public KopetePlugin
{
	Q_OBJECT

public:
	static HighlightPlugin  *plugin();

	HighlightPlugin( QObject *parent, const char *name, const QStringList &args );
	~HighlightPlugin();

	QPtrList<Filter> filters();
	Filter* newFilter();
	void removeFilter(Filter *f);


	/***************************************************************************
	 *   Re-implementation of KopetePlugin class methods                       *
	 ***************************************************************************/

//	virtual KActionCollection *customContextMenuActions(KopeteMetaContact*);


public slots:

	void slotIncomingMessage( KopeteMessage& msg );


private:
	static HighlightPlugin* pluginStatic_;
	HighlightPreferences *m_prefs;
	QPtrList<Filter> m_filters;
};

#endif
