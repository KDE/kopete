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
#ifndef UPDATEFOLDERTASK_H
#define UPDATEFOLDERTASK_H

#include <updateitemtask.h>

/**
Renames a folder on the server

@author Kopete Developers
*/
class UpdateFolderTask : public UpdateItemTask
{
public:
    UpdateFolderTask(Task* parent);

    ~UpdateFolderTask();

};

#endif
