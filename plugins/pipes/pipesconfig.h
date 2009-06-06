/*
    pipesconfig.h

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PIPESCONFIG_H
#define PIPESCONFIG_H

#include "pipesplugin.h"

/**
 * Static config storage. Ballin'.
 * @author Charles Connell <charles@connells.org>
 */

class PipesConfig
{
	public:
		static PipesConfig * self();
		static PipesPlugin::PipeOptionsList pipes();
		static void setPipes (PipesPlugin::PipeOptionsList);
		void save();
		void load();
		
	private:
		PipesConfig();
		
		PipesPlugin::PipeOptionsList mPipesList;
		static PipesConfig * mSelf;
};

#endif

