/*
    gwclientstream.h - Kopete Groupwise Protocol
  
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
    
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

#ifndef SAFEDELETE_H
#define SAFEDELETE_H

#include<qobject.h>

class SafeDelete;
class SafeDeleteLock
{
public:
	SafeDeleteLock(SafeDelete *sd);
	~SafeDeleteLock();

private:
	SafeDelete *_sd;
	bool own;
	friend class SafeDelete;
	void dying();
};

class SafeDelete
{
public:
	SafeDelete();
	~SafeDelete();

	void deleteLater(QObject *o);

	// same as QObject::deleteLater()
	static void deleteSingle(QObject *o);

private:
	QObjectList list;
	void deleteAll();

	friend class SafeDeleteLock;
	SafeDeleteLock *lock;
	void unlock();
};

class SafeDeleteLater : public QObject
{
	Q_OBJECT
public:
	static SafeDeleteLater *ensureExists();
	void deleteItLater(QObject *o);

private slots:
	void explode();

private:
	SafeDeleteLater();
	~SafeDeleteLater();

	QObjectList list;
	friend class SafeDelete;
	static SafeDeleteLater *self;
};

#endif
