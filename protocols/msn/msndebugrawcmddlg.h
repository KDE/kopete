/*
    msndebugrawcmddlg.h - Send a raw MSN command for debugging

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001 by Olaf Lueg              <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNDEBUGRAWCMDDLG_H
#define MSNDEBUGRAWCMDDLG_H

#include <kdialogbase.h>

class MSNDebugRawCommand_base;

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 * Simple debugging help
 */
class MSNDebugRawCmdDlg : public KDialogBase
{
	Q_OBJECT

public:
	MSNDebugRawCmdDlg( QWidget *parent );
	~MSNDebugRawCmdDlg();

	QString command();
	QString params();
	bool addNewline();
	bool addId();
	QString msg();

private:
	MSNDebugRawCommand_base *m_main;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

