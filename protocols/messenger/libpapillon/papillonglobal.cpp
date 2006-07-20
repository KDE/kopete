/*
   papillonglobal.cpp - Global utility functions for libpapillon

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "papillonglobal.h"

namespace Papillon
{

QString statusToString(Papillon::OnlineStatus::Status status)
{
	QString result;
	switch(status)
	{
		case OnlineStatus::Away:
			result = QLatin1String("AWY");
			break;
		case OnlineStatus::BeRightBack:
			result = QLatin1String("BRB");
			break;
		case OnlineStatus::Busy:
			result = QLatin1String("BSY");
			break;
		case OnlineStatus::Idle:
			result = QLatin1String("IDL");
			break;
		case OnlineStatus::Invisible:
			result = QLatin1String("HDN");
			break;
		case OnlineStatus::Offline:
			result = QLatin1String("FLN");
			break;
		case OnlineStatus::Online:
			result = QLatin1String("NLN");
			break;
		case OnlineStatus::OnThePhone:
			result = QLatin1String("PHN");
			break;
		case OnlineStatus::OutToLunch:
			result = QLatin1String("LUN");
			break;
	}

	return result;
}

Papillon::OnlineStatus::Status stringToStatus(const QString &status)
{
	Papillon::OnlineStatus::Status onlineStatus;

	if( status == QLatin1String("AWY") )
		onlineStatus = OnlineStatus::Away;
	else if( status == QLatin1String("BRB") )
		onlineStatus = OnlineStatus::BeRightBack;
	else if( status == QLatin1String("BSY") )
		onlineStatus = OnlineStatus::Busy;
	else if( status == QLatin1String("IDL") )
		onlineStatus = OnlineStatus::Idle;
	else if( status == QLatin1String("HDN") )
		onlineStatus = OnlineStatus::Invisible;
	else if( status == QLatin1String("FLN") )
		onlineStatus = OnlineStatus::Offline;
	else if( status == QLatin1String("NLN") )
		onlineStatus = OnlineStatus::Online;
	else if( status == QLatin1String("PHN") )
		onlineStatus = OnlineStatus::OnThePhone;
	else if( status == QLatin1String("LUN") )
		onlineStatus = OnlineStatus::OutToLunch;

	return onlineStatus;
}

}
