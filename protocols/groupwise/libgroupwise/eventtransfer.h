//
// C++ Interface: eventtransfer
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EVENTTRANSFER_H
#define EVENTTRANSFER_H

#include <transfer.h>

/**
@author Kopete Developers
*/
class EventTransfer : public Transfer
{
public:
	EventTransfer();
	TransferType type() { return Transfer::EventTransfer; }
	~EventTransfer();

};

#endif
