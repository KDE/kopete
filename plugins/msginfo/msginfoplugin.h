////////////////////////////////////////////////////////////////////////////////
// msginfoplugin.h                                                            //
//                                                                            //
// Copyright (C)  2002  Zack Rusin <zack@kde.org>                             //
//                                                                            //
// This library is free software; you can redistribute it and/or              //
// modify it under the terms of the GNU Lesser General Public                 //
// License as published by the Free Software Foundation; either               //
// version 2.1 of the License, or (at your option) any later version.         //
//                                                                            //
// This library is distributed in the hope that it will be useful,            //
// but WITHOUT ANY WARRANTY; without even the implied warranty of             //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          //
// Lesser General Public License for more details.                            //
//                                                                            //
// You should have received a copy of the GNU Lesser General Public           //
// License along with this library; if not, write to the Free Software        //
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA                   //
// 02111-1307  USA                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef AUTOAWAYPLUGIN_H
#define AUTOAWAYPLUGIN_H

#include <qobject.h>
#include <qmap.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
namespace Kopete { class Message; }
namespace Kopete { class MetaContact; }

class MsgInfoPlugin : public Kopete::Plugin
{
	Q_OBJECT
public:
	MsgInfoPlugin( QObject *parent, const char *name, const QStringList &args );
	~MsgInfoPlugin();

	bool serialize( Kopete::MetaContact *metaContact,
			QStringList &strList) const;
	void deserialize( Kopete::MetaContact *metaContact, const QStringList& data );

public slots:
	void slotProcessDisplay( Kopete::Message& msg );
	void slotProcessSend( Kopete::Message& msg );

protected:
	void changeMessage( Kopete::Message& msg );
private:
	QMap<const Kopete::MetaContact*, Q_UINT32> mMsgCountMap;
};

#endif


