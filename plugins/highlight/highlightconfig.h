/*
    highlightconfig.h

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Matt Rogers           <matt@matt.rogers.name>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef HIGHLIGHTCONFIG_H
#define HIGHLIGHTCONFIG_H

#include <QList>

class Filter;

class HighlightConfig
{
public:
	HighlightConfig();
	~HighlightConfig();

	void load();
	void save();

	QList<Filter*> filters() const;
	void removeFilter (Filter *f);
	void appendFilter (Filter *f);
	Filter* newFilter();

private:
	QList<Filter*> m_filters;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
