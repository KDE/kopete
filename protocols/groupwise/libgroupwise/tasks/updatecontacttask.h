//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UPDATECONTACTTASK_H
#define UPDATECONTACTTASK_H

#include <updateitemtask.h>

/**
Renames a contact on the server

@author Kopete Developers
*/
class UpdateContactTask : public UpdateItemTask
{
public:
    UpdateContactTask(Task* parent);

    ~UpdateContactTask();

};

#endif
