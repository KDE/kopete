//
// C++ Interface: gwmessagemanager
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GWMESSAGEMANAGER_H
#define GWMESSAGEMANAGER_H

#include <kopetemessagemanager.h>

/**
@author SUSE AG
*/
class GroupWiseMessageManager : public KopeteMessageManager
{
public:
    GroupWiseMessageManager(const KopeteContact* user, KopeteContactPtrList others, KopeteProtocol* protocol, int id, const char* name);

    ~GroupWiseMessageManager();

};

#endif
