//////////////////////////////////////////////////////////////////////////////
// gaducontact.h                                                            //
//                                                                          //
// Copyright (C)  2002-2003  Zack Rusin <zack@kde.org>                      //
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
#ifndef GADUCONTACT_H
#define GADUCONTACT_H

#include <qstringlist.h>
#include <qstring.h>
#include <qpoint.h>

#include "kopetecontact.h"
#include "kopetemessage.h"
#include "libgadu.h"

class KAction;
class GaduProtocol;
class KopeteMessageManager;

class GaduContact : public KopeteContact
{
    Q_OBJECT
public:
    GaduContact( const QString &protocolId, uin_t uin, const QString& name,
                 KopeteMetaContact *parent );

/*    virtual void          addToGroup( const QString &group );
    virtual void          removeFromGroup( const QString &group );
    virtual void          moveToGroup( const QString &from, const QString &to );
    virtual QStringList   groups() const;
    virtual void          setName( const QString &name );
    virtual QString       name() const;  */
    virtual ContactStatus status() const;
    virtual QString       statusText() const;
    virtual QString       statusIcon() const;
    virtual int           importance() const;
    virtual QString       data() const;
    virtual bool          isReachable();
    virtual KActionCollection *customContextMenuActions();
    virtual QString identityId() const;

    void  setParentIdentity( const QString& );
    void  setGaduStatus( Q_UINT32, const QString& descr = QString::null );
    Q_UINT32  gaduStatus() const;
    QString   description() const;
    uin_t uin() const;
public slots:
//    void showContextMenu(const QPoint& p, const QString& group);

    void slotUserInfo();
    void slotDeleteContact();
    void messageReceived( KopeteMessage& );
    void messageSend( KopeteMessage&, KopeteMessageManager* );

protected:
    virtual KopeteMessageManager* manager( bool canCreate = false );
    void initActions();

private:
    KopeteMessageManager *msgManager_;
    uin_t                 uin_;
//    QString               name_;
    QString               description_;
//    QStringList           groups_;
    QString               parentIdentity_;
    GaduProtocol         *protocol_;
    Q_UINT32              status_;
    KopeteContactPtrList  thisContact_;

    KAction     *actionSendMessage_;
    KAction     *actionInfo_;
    KAction     *actionRemove_;
private slots:
    void slotMessageManagerDestroyed();
};

#endif

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
