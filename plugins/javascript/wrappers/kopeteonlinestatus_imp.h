/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KOPETE_ONLINE_STATUS_IMP_H
#define _KOPETE_ONLINE_STATUS_IMP_H

#include <kopeteonlinestatus.h>
#include <kjs/object.h>

class StatusProperties
{
    public:
        StatusProperties() :
            description("description")
        {
        }

        const QString description;
};

class JSStatus : public KJS::ObjectImp
{
    public:
        JSStatus( const Kopete::OnlineStatus &status );

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

#endif

