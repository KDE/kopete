/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kjs/ustring.h>
#include <kjs/value.h>

#include "kopetecontact.h"
#include "kopeteonlinestatus_imp.h"

const StatusProperties JSStatus::methods;

JSStatus::JSStatus( const KopeteOnlineStatus &status ) : KJS::ObjectImp()
{
    s = status;
}

KJS::Value JSStatus::get( KJS::ExecState *exec, const KJS::Identifier &propertyName ) const
{
    QString prop = propertyName.qstring();

    if( prop ==  methods.description )
        return KJS::String( s.description() );
    else
        return KJS::ObjectImp::get( exec, propertyName );
}

void JSStatus::put( KJS::ExecState *exec, const KJS::Identifier &propertyName,
                        const KJS::Value &value, int attr )
{
    QString prop = propertyName.qstring();
    KJS::ObjectImp::put( exec, propertyName, value, attr );
}

bool JSStatus::canPut( KJS::ExecState *exec, const KJS::Identifier &propertyName ) const
{
    QString prop = propertyName.qstring();

    if( prop == methods.description )
    {
        return false;
    }
    else
    {
        return KJS::ObjectImp::canPut( exec, propertyName );
    }
}
