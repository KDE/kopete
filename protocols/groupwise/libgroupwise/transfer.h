//
// C++ Interface: transfer
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TRANSFER_H
#define TRANSFER_H

/**
@author Kopete Developers
*/
class Transfer{
public:
	enum TransferType { EventTransfer, RequestTransfer, ResponseTransfer };
	Transfer();
	virtual ~Transfer();
	
	virtual TransferType type() = 0;

};

#endif
