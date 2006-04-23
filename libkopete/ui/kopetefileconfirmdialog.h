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

#include <QWidget>
#include <kdialog.h>
#include "kopetetransfermanager.h"
#include "ui_fileconfirmbase.h"

/**
 * @author Olivier Goffart
 */
class KopeteFileConfirmDialog : public KDialog, private Ui::FileConfirmBase
{
Q_OBJECT

public:
	KopeteFileConfirmDialog( const Kopete::FileTransferInfo &info,
	                         const QString& description = QString::null,
	                         QWidget *parent = 0 );
	~KopeteFileConfirmDialog();

private:
	QWidget* m_view;
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
