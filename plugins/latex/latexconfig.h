/*
    Kopete Latex Plugin

    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2001-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef LATEXCONFIG_H
#define LATEXCONFIG_H

class LatexConfig
{
public:
	LatexConfig();
	~LatexConfig();

	void load();
	void save();
private:
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
