/*
    nowlisteningconfig.cpp

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2003      by Matt Rogers <matt@matt.rogers.name>
    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

#include "nowlisteningconfig.h"

NowListeningConfig::NowListeningConfig()
{
	load();
}

void NowListeningConfig::load()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Now Listening Plugin");
	mHeader = config->readEntry("Header", i18n("Now Listening To: "));
	mPerTrack = config->readEntry("PerTrack", i18n("%track( by %artist)( on %album)"));
	mConjunction = config->readEntry("Conjunction", i18n(", and "));
	mAutoAdvertising = config->readBoolEntry("AutoAdvertising", false);
}

void NowListeningConfig::save()
{
	KConfig *config=KGlobal::config();
	config->setGroup("Now Listening Plugin");
	config->writeEntry("Header", mHeader);
	config->writeEntry("PerTrack", mPerTrack);
	config->writeEntry("Conjunction", mConjunction);
	config->writeEntry("AutoAdvertising", mAutoAdvertising);
	config->sync();
}

QString NowListeningConfig::header() const
{
	return mHeader;
}

QString NowListeningConfig::perTrack() const
{
	return mPerTrack;
}

QString NowListeningConfig::conjunction() const
{
	return mConjunction;
}

bool NowListeningConfig::autoAdvertising() const
{
	return mAutoAdvertising;
}

void NowListeningConfig::setHeader(const QString& newHeader)
{
	mHeader = newHeader;
}

void NowListeningConfig::setPerTrack(const QString& newPerTrack)
{
	mPerTrack = newPerTrack;
}

void NowListeningConfig::setConjunction(const QString& newConjunction)
{
	mConjunction = newConjunction;
}

void NowListeningConfig::setAutoAdvertising(bool aa)
{
	mAutoAdvertising = aa;
}
