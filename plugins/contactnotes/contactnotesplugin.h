/***************************************************************************
                          contactnotesplugin.h  -  description
                             -------------------
    begin                : lun sep 16 2002
    copyright            : (C) 2002 by Olivier Goffart
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

#ifndef BABELFISHPLUGIN_H
#define BABELFISHPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include "kopeteplugin.h"

class QString;
class KAction;
class KActionCollection;

class KopeteMetaContact;

/**
  * @author Olivier Goffart <ogoffart@tiscalinet.be>
  *
  * Kopete Contact Notes Plugin
  *
  */

class ContactNotesPlugin : public KopetePlugin
{
	Q_OBJECT

public:
    static ContactNotesPlugin  *plugin();

	ContactNotesPlugin( QObject *parent, const char *name, const QStringList &args );
	~ContactNotesPlugin();

	QString notes(KopeteMetaContact *m);

	/***************************************************************************
	 *   Re-implementation of KopetePlugin class methods                       *
	 ***************************************************************************/

	bool serialize( KopeteMetaContact *metaContact, QStringList &strList) const;
	void deserialize( KopeteMetaContact *metaContact, const QStringList& data );

	virtual KActionCollection *customContextMenuActions(KopeteMetaContact*);

public slots:
	void setNotes(QString n, KopeteMetaContact *m);

private:
	QMap<const KopeteMetaContact*, QString> m_infoMap;

	KActionCollection* m_actionCollection;
	KAction* m_actionEdit;
	KopeteMetaContact *m_currentMetaContact;

	static ContactNotesPlugin* pluginStatic_;

private slots: // Private slots
  /** No descriptions */
  void slotEditInfo();
};

#endif


