/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KJSEMBED_BINDINGS_KOPETEONLINESTATUSIMP_H
#define KJSEMBED_BINDINGS_KOPETEONLINESTATUSIMP_H

#include <kopeteonlinestatus.h>
#include <kjs/object.h>

namespace KJSEmbed {
namespace Bindings {

class StatusProperties
{
public:
	StatusProperties() :
		description("description")
        {
        }

	const QString description;
};

class JSOnlineStatus : public KJS::ObjectImp
{
public:
	JSOnlineStatus( const Kopete::OnlineStatus &status );

	virtual KJS::UString className() const { return "OnlineStatus"; };

	virtual KJS::Value get(KJS::ExecState *exec, const KJS::Identifier &propertyName) const;

	virtual void put(KJS::ExecState *exec, const KJS::Identifier &propertyName,
		const KJS::Value &value, int attr = KJS::None);

	virtual bool canPut(KJS::ExecState *exec, const KJS::Identifier &propertyName) const;

	const Kopete::OnlineStatus &status() { return s; }

private:
	Kopete::OnlineStatus s;

	static const StatusProperties methods;
};

} // Bindings namespace
} // KJSEmbed namespace

#endif

