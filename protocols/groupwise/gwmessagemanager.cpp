//
// C++ Implementation: gwmessagemanager
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "gwmessagemanager.h"

GroupWiseMessageManager::GroupWiseMessageManager(const KopeteContact* user, KopeteContactPtrList others, KopeteProtocol* protocol, int id, const char* name): KopeteMessageManager(user, others, protocol, id, name)
{
}


GroupWiseMessageManager::~GroupWiseMessageManager()
{
}


