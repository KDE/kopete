/*
    contactnotesedit.h  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef CONTACTNOTESEDIT_H
#define CONTACTNOTESEDIT_H

#include <qwidget.h>
#include <qstring.h>
#include <QLabel>
#include <kdialog.h>

class QLabel;
class QTextEdit;
namespace Kopete { class MetaContact; }
class ContactNotesPlugin;

/**
  *@author Olivier Goffart
  */
  
class ContactNotesEdit : public KDialog  {
   Q_OBJECT
public: 
	explicit ContactNotesEdit(Kopete::MetaContact *m,ContactNotesPlugin *p=0);
	~ContactNotesEdit();

private:
	ContactNotesPlugin *m_plugin;
	Kopete::MetaContact *m_metaContact;

	QLabel *m_label;
	QTextEdit *m_linesEdit;
	
protected slots: // Protected slots
	virtual void slotButtonClicked(int buttonCode);
signals: // Signals
	void notesChanged(const QString&, Kopete::MetaContact*);
};

#endif
