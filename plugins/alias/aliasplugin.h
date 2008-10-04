/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AliasPLUGIN_H
#define AliasPLUGIN_H

#include "kopeteplugin.h"

class AliasPlugin : public Kopete::Plugin
{
	Q_OBJECT

	public:
		static AliasPlugin *plugin();
	
		AliasPlugin( QObject *parent, const QVariantList &args );
		~AliasPlugin();
	
	private:
		static AliasPlugin * pluginStatic_;
};

#endif


