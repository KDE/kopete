/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KOPETE_MESSAGE_IMP_H
#define _KOPETE_MESSAGE_IMP_H

#include <kopetemessage.h>
#include "bindingobject.h"

class Message : public BindingObject
{
	Q_OBJECT

	Q_PROPERTY( QColor bgColor READ bgColor WRITE setBgColor );
	Q_PROPERTY( QColor fgColor READ fgColor WRITE setFgColor );
	Q_PROPERTY( QFont font READ font WRITE setFont );
	Q_PROPERTY( QString plainBody READ plainBody WRITE setPlainBody );
	Q_PROPERTY( QString richBody READ richBody WRITE setRichBody );
	Q_PROPERTY( int type READ type );
	Q_PROPERTY( QString xml READ xml );
	Q_PROPERTY( int importance READ importance WRITE setImportance );

	public:
		Message( KopeteMessage *m, QObject *parent=0, const char *name=0 );

		void setBgColor( const QColor & );
		QColor bgColor() const;

		void setFgColor( const QColor & );
		QColor fgColor() const;

		void setFont( const QFont & );
		QFont font() const;

		void setPlainBody( const QString &body );
		QString plainBody() const;

		void setRichBody( const QString &body );
		QString richBody() const;

		void setImportance( int importance );
		int importance() const;

		int type() const;

		QString xml() const;

	private:
		KopeteMessage *msg;
};

#endif

