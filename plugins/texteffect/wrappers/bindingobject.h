// -*- c++ -*-

/*
 *  Copyright (C) 2003, Richard J. Moore <rich@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef KJSEMBED_BINDING_OBJECT_H
#define KJSEMBED_BINDING_OBJECT_H

#include <qobject.h>

/**
 * A baseclass for QObject bindings. This class is currently just a
 * stub, but I hope it will contain utilities for QObject wrapper
 * classes.
 *
 * @author Richard Moore, rich@kde.org
 * @version $Id$
 */
class BindingObject : public QObject
{
    Q_OBJECT

public:
    BindingObject( QObject *parent, const char *name=0 );
    virtual ~BindingObject();

    const char *jsClassName() const { return jsClazz; }

protected:
    void setJSClassName( const char *clazz ) { jsClazz = clazz; }

private:
    QCString jsClazz;
    class BindingObjectPrivate *d;
};

#endif // KJSEMBED_BINDING_OBJECT_H


