/*
    kopeteonlinestatusmanager.h

    Copyright (c) 2004-2005 by Olivier Goffart  <ogoffart@kde.org>

    Kopete    (c) 2004-2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEONLINESTATUSACTION_H
#define KOPETEONLINESTATUSACTION_H

#include <kaction.h>

namespace Kopete
{
	class OnlineStatus;

/**
 * 
 */
class OnlineStatusAction : public KAction
{
	Q_OBJECT
  public:
	OnlineStatusAction ( const OnlineStatus& status, const QString &text, const QIcon &pix, QObject *parent );
	~OnlineStatusAction();

  signals:
	void activated( const Kopete::OnlineStatus& status );
  private slots:
	void slotActivated();
  private:
	class Private;
	Private * const d;
};

}  //END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:

