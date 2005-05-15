/***************************************************************************
                          contactnotesplugin.h  -  description
                             -------------------
    begin                : lun sep 16 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
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

namespace Kopete { class MetaContact; }

/**
  * @author Olivier Goffart <ogoffart @ kde.org>
  *
  * Kopete Contact Notes Plugin
  *
  */

class ContactNotesPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
    static ContactNotesPlugin  *plugin();

	ContactNotesPlugin( QObject *parent, const char *name, const QStringList &args );
	~ContactNotesPlugin();

	QString notes(Kopete::MetaContact *m);


public slots:
	void setNotes(const QString n, Kopete::MetaContact *m);

private:
	static ContactNotesPlugin* pluginStatic_;

private slots: // Private slots
  /** No descriptions */
  void slotEditInfo();
};

#endif


