/*
 * Copyright (C) 2003  Justin Karneges <justin@affinix.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef XMPP_RESOURCE_H
#define XMPP_RESOURCE_H

#include <QString>

#include <iris_export.h>

#include "xmpp_status.h"

namespace XMPP
{
	class IRIS_EXPORT Resource
	{
	public:
		Resource(const QString &name="", const Status &s=Status());
		~Resource();

		const QString & name() const;
		int priority() const;
		const Status & status() const;

		void setName(const QString &);
		void setStatus(const Status &);

	private:
		QString v_name;
		Status v_status;
#ifdef IRIS_FULL_TEMPLATE_EXPORT_INSTANTIATION
    public:
        bool operator==(const Resource&) const {
            qWarning("Resource::operator==(const Resource&) was called");
            return false;
        }
#endif
	};
#ifdef IRIS_FULL_TEMPLATE_EXPORT_INSTANTIATION
    inline uint qHash(const Resource&) {
        qWarning("inline uint qHash(const Resource&) was called");
        return 0;
    }
#endif
}

#endif
