/***************************************************************************
                          contactnotesedit.h  -  description
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

#ifndef CONTACTNOTESEDIT_H
#define CONTACTNOTESEDIT_H

#include <qwidget.h>
#include <qstring.h>
#include <kdialogbase.h>

class QLabel;
class QTextEdit;
namespace Kopete { class MetaContact; }
class ContactNotesPlugin;

/**
  *@author Olivier Goffart
  */
  
class ContactNotesEdit : public KDialogBase  {
   Q_OBJECT
public: 
	ContactNotesEdit(Kopete::MetaContact *m,ContactNotesPlugin *p=0 ,const char *name=0);
	~ContactNotesEdit();

private:
	ContactNotesPlugin *m_plugin;
	Kopete::MetaContact *m_metaContact;

	QLabel *m_label;
	QTextEdit *m_linesEdit;
	
protected slots: // Protected slots
	virtual void slotOk();
signals: // Signals
	void notesChanged(const QString, Kopete::MetaContact*);
};

#endif
