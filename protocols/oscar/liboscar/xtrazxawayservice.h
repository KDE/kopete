/*
   Kopete Oscar Protocol
   xtrazxawayservice.h - Xtraz XAwayService

   Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

   Kopete (c) 2007 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef XTRAZXAWAYSERVICE_H
#define XTRAZXAWAYSERVICE_H

#include "xtrazxservice.h"

namespace Xtraz
{

class XAwayService : public XService
{
public:
	XAwayService();
	
	void setSenderId( const QString& uni );
	QString senderId() const;

	void setIconIndex( int index );
	int iconIndex() const;

	void setDescription( const QString& description );
	QString description() const;

	void setMessage( const QString& message );
	QString message() const;

	QString serviceId() const Q_DECL_OVERRIDE;

protected:
	void createRequest( QDomDocument& doc, QDomElement &e ) const Q_DECL_OVERRIDE;
	void createResponse( QDomDocument& doc, QDomElement &e ) const Q_DECL_OVERRIDE;
	void handleRequest( QDomElement& eRoot ) Q_DECL_OVERRIDE;
	void handleResponse( QDomElement& eRoot ) Q_DECL_OVERRIDE;

private:
	int m_iconIndex;
	QString m_description;
	QString m_message;
	QString m_senderId;
};

}

#endif

