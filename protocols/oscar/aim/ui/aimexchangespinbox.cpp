// aimexchangespinbox.cpp

// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
// 02110-1301, USA.


AIMExchangeSpinBox::AIMExchangeSpinBox( QWidget* parent, const char* name )
{

}
AIMExchangeSpinBox::~AIMExchangeSpinBox()
{

}

void AIMExchangeSpinBox::setExchangeList( const QValueList<int>& list )
{
    int min = 999;
    int max = 0;
    kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "list empty?" << list.isEmpty() << endl;
    QValueList<int>::const_iterator it, itEnd = list.constEnd();
    for ( it = list.constBegin(); it != itEnd; ++it )
    {
        min = QMIN( ( *it ), min );
        max = QMAX( ( *it ), max );
    }

    kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "min: " << min << " max: " << max << endl;
    setMinValue( min );
    setMaxValue( max );
}

void AIMExchangeSpinBox::stepUp()
{
    //find the next correct value
    //set the line step to that then call base class
    //reset line step

}

void AIMExchangeSpinBox::stepDown()
{
    //find the next correct value
    //set the line step to that then call base class
    //reset line step

}

#include "aimexchangespinbox.h"

