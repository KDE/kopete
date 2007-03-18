/*
   papillonglobal.cpp - Global utility functions for libpapillon

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Global"

namespace Papillon
{
namespace Global
{

QString presenceToString(Papillon::Presence::Status status)
{
	QString result;
	switch(status)
	{
		case Presence::Away:
			result = QLatin1String("AWY");
			break;
		case Presence::BeRightBack:
			result = QLatin1String("BRB");
			break;
		case Presence::Busy:
			result = QLatin1String("BSY");
			break;
		case Presence::Idle:
			result = QLatin1String("IDL");
			break;
		case Presence::Invisible:
			result = QLatin1String("HDN");
			break;
		case Presence::Offline:
			result = QLatin1String("FLN");
			break;
		case Presence::Online:
			result = QLatin1String("NLN");
			break;
		case Presence::OnThePhone:
			result = QLatin1String("PHN");
			break;
		case Presence::OutToLunch:
			result = QLatin1String("LUN");
			break;
	}

	return result;
}

Papillon::Presence::Status stringToPresence(const QString &status)
{
	Papillon::Presence::Status presence = Presence::Offline;

	if( status == QLatin1String("AWY") )
		presence = Presence::Away;
	else if( status == QLatin1String("BRB") )
		presence = Presence::BeRightBack;
	else if( status == QLatin1String("BSY") )
		presence = Presence::Busy;
	else if( status == QLatin1String("IDL") )
		presence = Presence::Idle;
	else if( status == QLatin1String("HDN") )
		presence = Presence::Invisible;
	else if( status == QLatin1String("FLN") )
		presence = Presence::Offline;
	else if( status == QLatin1String("NLN") )
		presence = Presence::Online;
	else if( status == QLatin1String("PHN") )
		presence = Presence::OnThePhone;
	else if( status == QLatin1String("LUN") )
		presence = Presence::OutToLunch;

	return presence;
}

} // Global
} // Papillon
