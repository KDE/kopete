/******************************************************************************
 * Copyright (C) 2008 Teo Mrnjavac <teo.mrnjavac@gmail.com>                   *
 *           (C) 2008-2009 Seb Ruiz <ruiz@kde.org>                            *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#ifndef AMAROK_TOKEN_H
#define AMAROK_TOKEN_H

#include <QIcon>
#include <QWidget>
#include <QLabel>
#include <QPixmap>

class Token;

class TokenFactory
{

public:
    virtual ~TokenFactory() {}
    virtual Token * createToken( const QString &text, const QString &iconName, int value, QWidget *parent = 0 );
};

//Defines a part of a filename, drag&droppable in the TokenLayoutWidget bar from the TokenPool list.
class Token : public QWidget
{
        Q_OBJECT

    public:

        explicit Token( const QString &text, const QString &iconName, int value, QWidget *parent = 0 );

        KIcon icon() const;
        QString iconName() const;
        QString name() const;
        int value() const;

    signals:
        void changed();

    protected:
        virtual void paintEvent(QPaintEvent *pe);

    protected:

        QString     m_name;
        KIcon       m_icon;
        QString     m_iconName;
        int         m_value;

        QLabel      *m_iconContainer;
        QLabel      *m_label;
};

#endif // AMAROK_TOKEN_H

