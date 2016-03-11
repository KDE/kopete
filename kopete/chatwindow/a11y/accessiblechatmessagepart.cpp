/*
    Copyright 2011  José Millán Soto <fid@gpul.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "accessiblechatmessagepart.h"
#include <khtml_part.h>
#include <khtmlview.h>
#include <dom/html_document.h>

/** FIXME:
#if QT_VERSION >= 0x40800
QString Q_GUI_EXPORT qTextAfterOffset( int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset);
QString Q_GUI_EXPORT qTextBeforeOffset( int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset);
QString Q_GUI_EXPORT qTextAtOffset( int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset);
#endif
*/
AccessibleChatMessagePart::AccessibleChatMessagePart( KHTMLView* widget ):
  QAccessibleWidget( widget, QAccessible::Document )
{
  m_part = widget->part();
}

int AccessibleChatMessagePart::childCount() const
{
    return 0;
}

// DEPRECATED: QAccessibleInterface::navigate() is replaced by child() and parent()
/*int AccessibleChatMessagePart::navigate(QAccessible::RelationFlag rel, int entry, QAccessibleInterface** target) const
{
    if (rel == QAccessible::Child) {
      *target = 0;
      return -1;
    }
    return QAccessibleWidgetEx::navigate(rel, entry, target);
}*/

int AccessibleChatMessagePart::characterCount()
{
    return plainText().length();
}

int AccessibleChatMessagePart::selectionCount()
{
    return m_part->hasSelection() ? 1 : 0;
}

void AccessibleChatMessagePart::addSelection( int startOffset, int endOffset )
{
}

void AccessibleChatMessagePart::removeSelection( int selectionIndex )
{
}

void AccessibleChatMessagePart::setSelection( int selectionIndex, int startOffset, int endOffset )
{
}

void AccessibleChatMessagePart::setCursorPosition( int position )
{
}

QString AccessibleChatMessagePart::plainText() const
{
    DOM::Node body = m_part->htmlDocument().getElementsByTagName("body").item(0);

    if (body.isNull())
        return m_part->documentSource();

    QString text = body.textContent().string();

    //Usually textContent has a lot of superfluous lines and spaces
    QStringList lines = text.split(QChar('\n'));
    lines = lines.replaceInStrings(QRegExp(QLatin1String("^\\s+")), QString());
    lines.removeAll(QString());

    return lines.join(QLatin1String("\n"));
}

QString AccessibleChatMessagePart::text( int startOffset, int endOffset )
{
    QString text = plainText();
    text.truncate( endOffset );
    text.remove( 0, startOffset );
    return text;
}

QString AccessibleChatMessagePart::attributes( int offset, int* startOffset, int* endOffset )
{
    return QString();
}

void AccessibleChatMessagePart::selection( int selectionIndex, int* startOffset, int* endOffset )
{
    *startOffset = -1;
    *endOffset = -1;
}

QRect AccessibleChatMessagePart::characterRect( int offset)
{
    return QRect();
}

int AccessibleChatMessagePart::offsetAtPoint( const QPoint& point)
{
    return 0;
}

int AccessibleChatMessagePart::cursorPosition()
{
    return 0;
}

void AccessibleChatMessagePart::scrollToSubstring( int startIndex, int endIndex )
{
}

/**
 * FIXME:
QString AccessibleChatMessagePart::textAfterOffset( int offset, QAccessible::TextBoundaryType boundaryType, int* startOffset, int* endOffset )
{
#if QT_VERSION >= 0x40800
    return QAccessibleTextInterface::textAfterOffset( offset, boundaryType, startOffset, endOffset );
#else
    return QString();
#endif
}

QString AccessibleChatMessagePart::textBeforeOffset( int offset, QAccessible::TextBoundaryType boundaryType, int* startOffset, int* endOffset )
{
#if QT_VERSION >= 0x40800
    return QAccessibleTextInterface::textBeforeOffset( offset, boundaryType, startOffset, endOffset );
#else
    return QString();
#endif
}

QString AccessibleChatMessagePart::textAtOffset( int offset, QAccessible::TextBoundaryType boundaryType, int* startOffset, int* endOffset )
{
#if QT_VERSION >= 0x40800
    return QAccessibleTextInterface::textAtOffset( offset, boundaryType, startOffset, endOffset );
#else
    return QString();
#endif
}
*/