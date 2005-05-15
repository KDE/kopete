/*
    kopetefileconfirmdialog.h

    Copyright (c) 2003 by Olivier Goffart <ogoffart @ kde.org>

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

#ifndef KOPETEFILECONFIRMDIALOG_H
#define KOPETEFILECONFIRMDIALOG_H

#include <qwidget.h>
#include <kdialogbase.h>
#include "kopetetransfermanager.h"

class FileConfirmBase;

/**
  *@author Olivier Goffart
  */

class KopeteFileConfirmDialog : public KDialogBase
{
Q_OBJECT

public:
	KopeteFileConfirmDialog(const Kopete::FileTransferInfo &info,const QString& description=QString::null, QWidget *parent=0, const char* name=0);
	~KopeteFileConfirmDialog();

private:
	FileConfirmBase* m_view;
	Kopete::FileTransferInfo m_info;
	bool m_emited;

public slots:
	void slotBrowsePressed();

protected slots:
	virtual void slotUser2();
	virtual void slotUser1();
	virtual void closeEvent( QCloseEvent *e);

signals:
	void accepted(const Kopete::FileTransferInfo &info, const QString &filename);
	void refused(const Kopete::FileTransferInfo &info);
};

#endif
