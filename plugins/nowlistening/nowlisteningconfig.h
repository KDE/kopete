/*
    nowlisteningconfig.h

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

#ifndef NOWLISTENINGCONFIG_H
#define NOWLISTENINGCONFIG_H

#include <qstring.h>

class NowListeningConfig
{
public:
	NowListeningConfig();

	void load();
	void save();

	QString header() const;
	QString perTrack() const;
	QString conjunction() const;
	bool autoAdvertising() const;

	void setHeader(const QString& newHeader);
	void setPerTrack(const QString& newPerTrack);
	void setConjunction(const QString &newConjunction);
	void setAutoAdvertising(bool aa);

private:
	QString mHeader;
	QString mPerTrack;
	QString mConjunction;
	bool    mAutoAdvertising;
};

#endif
