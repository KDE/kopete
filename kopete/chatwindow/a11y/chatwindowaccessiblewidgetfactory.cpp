/*
        Copyright 2011    José Millán Soto <fid@gpul.org>

        This library is free software; you can redistribute it and/or
        modify it under the terms of the GNU Lesser General Public
        License as published by the Free Software Foundation; either
        version 2.1 of the License, or (at your option) any later version.

        This library is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the GNU
        Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public
        License along with this library.    If not, see <http://www.gnu.org/licenses/>.
 */

#include "chatwindowaccessiblewidgetfactory.h"

#include <QtCore/qplugin.h>
#include "accessiblechatmessagepart.h"

Q_EXPORT_PLUGIN(ChatWindowAccessibleWidgetFactory)

QAccessibleInterface *ChatWindowAccessibleWidgetFactory::create( const QString &key, QObject *object )
{
    if ( key == QLatin1String( "KHTMLView" ) ) {
        return new AccessibleChatMessagePart( reinterpret_cast< KHTMLView* >( object ) );
    }
    return 0;
}

ChatWindowAccessibleWidgetFactory::ChatWindowAccessibleWidgetFactory( QObject *parent ):
    QAccessiblePlugin( parent )
{
}

QStringList ChatWindowAccessibleWidgetFactory::keys() const
{
    QStringList l;
    l.append( QLatin1String( "KHTMLView" ) );
    return l;
}
