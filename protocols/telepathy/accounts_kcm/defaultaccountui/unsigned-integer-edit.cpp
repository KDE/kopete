/*
 * This file is part of telepathy-accounts-kcm
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "unsigned-integer-edit.h"

#include <KDebug>

#include <QtGui/QKeyEvent>

UnsignedIntegerEdit::UnsignedIntegerEdit(QWidget *parent)
 : QLineEdit(parent)
{
    connect(this, SIGNAL(textChanged(QString)), SLOT(onTextChanged(QString)));
}

UnsignedIntegerEdit::~UnsignedIntegerEdit()
{

}

void UnsignedIntegerEdit::setValue(uint unsignedInteger)
{
    setText(QString::number(unsignedInteger));
}

void UnsignedIntegerEdit::keyPressEvent(QKeyEvent *event)
{
    kDebug() << "Key:" << event->key() << "Text:" << event->text();

    // If the text is empty or a modifier, allow the keypress
    if ((event->text().isEmpty()) || (event->key() < Qt::Key_Space)) {
        event->ignore();
        QLineEdit::keyPressEvent(event);
        return;
    }

    // If the key is backspace or delete, allow it
    if ((event->key() == Qt::Key_Delete) || (event->key() == Qt::Key_Backspace)) {
        event->ignore();
        QLineEdit::keyPressEvent(event);
        return;
    }

    // Check for numbers (and ensure maximum input length is not expended
    // FIXME: Have a better check to make sure the user doesn't enter a number too large.
    QString validKeys("0123456789");
    if (validKeys.contains(event->text())) {
        if ((text().length() + 1) <= 4)
        {
            kDebug() << "Key is a number.";
            event->ignore();
            QLineEdit::keyPressEvent(event);
            return;
        }
    }

    // Anything else, reject the keypress.
    event->accept();
}

void UnsignedIntegerEdit::onTextChanged(const QString &text)
{
    Q_EMIT unsignedIntegerChanged(text.toUInt());
}


#include "unsigned-integer-edit.moc"

