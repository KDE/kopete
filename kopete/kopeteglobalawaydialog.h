/*
  kopeteglobalawaydialog.h  -  Kopete Global Away Dialog

  Copyright (c) 2002 by Christopher TenHarmsel <tenharmsel@users.sourceforge.net>
  
  Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#ifndef KOPETEGLOBALAWAYDIALOG_H
#define KOPETEGLOBALAWAYDIALOG_H

#include "kopeteawaydialog.h"

/**
 * This class implements the KopeteAwayDialog
 * for global away settings.  It is very simple,
 * as most of the functionality is in the parent
 * class.
 * @author Christopher TenHarmsel <tenharmsel@users.sourceforge.net>
 */
class KopeteGlobalAwayDialog : public KopeteAwayDialog {
    Q_OBJECT

public:
    /** Constructor */
    KopeteGlobalAwayDialog(QWidget *parent=0, const char *name=0);

protected:
    /**
     * Sets the user away, called when the user
     * clicks on OK
     */
    virtual void setAway(QString type);
};
#endif

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */

// vim: set noet ts=4 sts=4 sw=4:

