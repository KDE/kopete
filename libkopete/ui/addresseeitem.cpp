/*
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "addresseeitem.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <q3groupbox.h>
#include <qregexp.h>
#include <QPixmap>

#include <klocale.h>
#include <kdebug.h>

AddresseeItem::AddresseeItem( Q3ListView *parent, const KABC::Addressee &addressee) :
  K3ListViewItem( parent ),
  mAddressee( addressee )
{
  //We can't save showphoto because we don't have a d pointer
  KABC::Picture pic = mAddressee.photo();
  if(!pic.isIntern())
    pic = mAddressee.logo();
  if(pic.isIntern())
  {
    QPixmap qpixmap = QPixmap::fromImage( pic.data().scaledToWidth(60) ); //60 pixels seems okay.. kmail uses 60 btw
    setPixmap( Photo,qpixmap );
  }

  setText( Name, addressee.realName() );
  setText( Email, addressee.preferredEmail() );
}

QString AddresseeItem::key( int column, bool ) const
{
  if (column == Email) {
    QString value = text(Email);
    QRegExp emailRe(QString::fromLatin1("<\\S*>"));
    int match = emailRe.indexIn(value);
    if (match > -1)
      value = value.mid(match + 1, emailRe.matchedLength() - 2);

    return value.toLower();
  }

  return text(column).toLower();
}


