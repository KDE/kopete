//////////////////////////////////////////////////////////////////////////////
// gaduprotocol.cpp                                                         //
//                                                                          //
// Copyright (C)  2002  Zack Rusin <zack@kde.org>                           //
//                                                                          //
// This program is free software; you can redistribute it and/or            //
// modify it under the terms of the GNU General Public License              //
// as published by the Free Software Foundation; either version 2           //
// of the License, or (at your option) any later version.                   //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU General Public License for more details.                             //
//                                                                          //
// You should have received a copy of the GNU General Public License        //
// along with this program; if not, write to the Free Software              //
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA                //
// 02111-1307, USA.                                                         //
//////////////////////////////////////////////////////////////////////////////
#include <qapplication.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qstringlist.h>

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetegroup.h"

#include "gaduprotocol.h"
#include "gaducontact.h"
#include "gaduaddcontactpage.h"
#include "gadupreferences.h"
#include "gadusession.h"
#include "gaducommands.h"

#include <libgadu.h>

K_EXPORT_COMPONENT_FACTORY( kopete_gadu, KGenericFactory<GaduProtocol> );

GaduProtocol* GaduProtocol::protocolStatic_ = 0L;

GaduProtocol::GaduProtocol( QObject* parent, const char* name, const QStringList & )
:   KopeteProtocol( parent, name ),
    gaduStatusOffline_(       KopeteOnlineStatus::Offline, 25, this, 0x0,                       "gg_offline", i18n( "Go O&ffline" ),   i18n( "Online" ) ),
    gaduStatusNotAvail_(      KopeteOnlineStatus::Away,    15, this, GG_STATUS_NOT_AVAIL,       "gg_away",    i18n( "Go A&way" ),      i18n( "Unavailable" ) ),
    gaduStatusNotAvailDescr_( KopeteOnlineStatus::Away,    20, this, GG_STATUS_NOT_AVAIL_DESCR, "gg_away",    i18n( "Go A&way" ),      i18n( "Unavailable" ) ),
    gaduStatusBusy_(          KopeteOnlineStatus::Away,    20, this, GG_STATUS_BUSY,            "gg_busy",    i18n( "Go B&usy" ),      i18n( "Busy" ) ),
    gaduStatusBusyDescr_(     KopeteOnlineStatus::Away,    25, this, GG_STATUS_BUSY_DESCR,      "gg_busy",    i18n( "Go B&usy" ),      i18n( "Busy" ) ),
    gaduStatusInvisible_(     KopeteOnlineStatus::Away,     5, this, GG_STATUS_INVISIBLE,       "gg_invi",    i18n( "Go I&nvisible" ), i18n( "Invisible" ) ),
    gaduStatusInvisibleDescr_(KopeteOnlineStatus::Away,    10, this, GG_STATUS_INVISIBLE_DESCR, "gg_invi",    i18n( "Go I&nvisible" ), i18n( "Invisible" ) ),
    gaduStatusAvail_(         KopeteOnlineStatus::Online,  20, this, GG_STATUS_AVAIL,           "gg_online",  i18n( "Go &Online" ),    i18n( "Online" ) ),
    gaduStatusAvailDescr_(    KopeteOnlineStatus::Online,  25, this, GG_STATUS_AVAIL_DESCR,     "gg_online",  i18n( "Go &Online" ),    i18n( "Online" ) )
{
    if ( protocolStatic_ )
        kdDebug(14100)<<"####"<<"GaduProtocol already initialized"<<endl;
    else
        protocolStatic_ = this;

    prefs_ = new GaduPreferences( "gadu_protocol", this );
    QObject::connect( prefs_, SIGNAL(saved()), this, SLOT(settingsChanged()) );

    setStatusIcon( "gg_connecting" );

    addAddressBookField( "messaging/gadu", KopetePlugin::MakeIndexField );
}

GaduProtocol::~GaduProtocol()
{
    protocolStatic_ = 0L;
}

GaduProtocol*
GaduProtocol::protocol()
{
    return protocolStatic_;
}

void
GaduProtocol::setAway()
{
    slotGoAway();
}

void
GaduProtocol::setAvailable()
{
    slotGoOnline();
}

AddContactPage*
GaduProtocol::createAddContactWidget( QWidget* parent )
{
    return new GaduAddContactPage( this, parent );
}

void
GaduProtocol::settingsChanged()
{
    userUin_ = prefs_->uin();
    password_ = prefs_->password();
}

void
GaduProtocol::deserializeContact( KopeteMetaContact *metaContact,
                                  const QMap<QString, QString> &serializedData,
                                  const QMap<QString, QString> & /* addressBookData */ )
{
    //kdDebug()<<"Adding "<<serializedData[ "contactId" ]<<" || "<< serializedData[ "displayName" ] <<endl;
    addContact( serializedData[ "contactId" ], serializedData[ "displayName" ], metaContact );
}

KopeteOnlineStatus
GaduProtocol::convertStatus( uint status )
{
    switch( status )
    {
    case GG_STATUS_NOT_AVAIL:
        return gaduStatusNotAvail_;
    case GG_STATUS_NOT_AVAIL_DESCR:
        return gaduStatusNotAvailDescr_;
    case GG_STATUS_BUSY:
        return gaduStatusBusy_;
    case GG_STATUS_BUSY_DESCR:
        return gaduStatusBusyDescr_;
    case GG_STATUS_INVISIBLE:
        return gaduStatusInvisible_;
    case GG_STATUS_INVISIBLE_DESCR:
        return gaduStatusInvisibleDescr_;
    case GG_STATUS_AVAIL:
        return gaduStatusAvail_;
    case GG_STATUS_AVAIL_DESCR:
        return gaduStatusAvailDescr_;
    default:
        return gaduStatusOffline_;
    }
}

#include "gaduprotocol.moc"

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
