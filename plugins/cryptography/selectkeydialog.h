/***************************************************************************
   Copyright (c) 2002      by y0k0            <bj@altern.org>
   Copyright (c) 2007      by Charles Connell <charles@connells.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LISTKEYS_H
#define LISTKEYS_H

#include <kdialog.h>
#include <QProcess>
#include <QPixmap>

class K3ListView;
class QCheckBox;
class Q3ListViewItem;

class SelectKeyDialog : public KDialog
{
		Q_OBJECT

	public:
		explicit SelectKeyDialog ( QString &keyId, QWidget *parent = 0 );
		~SelectKeyDialog();

	private slots:
		void slotReadKey ();
		void slotOk();
		void slotSelect ( Q3ListViewItem *item );
		
	private:
		QProcess* fp;
		K3ListView *mKeysListpr;
		QPixmap mKeyPair;
		QString * mKeyId;
};



#endif
