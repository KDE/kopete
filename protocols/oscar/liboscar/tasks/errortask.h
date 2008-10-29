/*
   Kopete Oscar Protocol
   errortask.h - handle OSCAR protocol errors

   Copyright (c) 2004 Matt Rogers <mattr@kde.org>

   Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef ERRORTASK_H
#define ERRORTASK_H

#include <task.h>

/**
Handles OSCAR protocol errors received from the server on snac subtype 0x01
@author Matt Rogers
*/
class ErrorTask : public Task
{
	Q_OBJECT
public:
    ErrorTask( Task* parent );
    ~ErrorTask();
    bool take( Transfer* transfer );

signals:
	void messageError( const QString& contact, uint messageId );

protected:
    bool forMe( const Transfer* transfer ) const;

};

#endif
