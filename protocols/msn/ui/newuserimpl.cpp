/***************************************************************************
                          newuserimpl.cpp  -  description
                             -------------------
    begin                : Fri Jan 5 2001
    copyright            : (C) 2001 by Olaf Lueg
    email                : olaf.lueg@t-online.de
 ***************************************************************************/

 /***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by  the Free Software Foundation; either version 2 of the   *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/
#include "newuserimpl.h"
#include <qcheckbox.h>
#include <qlabel.h>
NewUserImpl::NewUserImpl(QWidget *parent, const char *name ) : NewUser(parent,name) {

}
NewUserImpl::~NewUserImpl(){
}
/**  */
void NewUserImpl::slotClose(){
	emit addUser(userHandle,publicName);
	if(Block->isChecked()){
		emit blockUser(userHandle);
	}
	delete this;
}
/**  */
void NewUserImpl::setHandle(const QString & _handle ,const QString & _public){
	if(_public.isNull())
	{
		handle->setText(_handle);
		publicName = _handle;
	}
	else
	{
		handle->setText(_public+"\n("+_handle+")");
		publicName = _public;
	}
		
	userHandle = _handle;
}
#include "newuserimpl.moc"

// vim: set noet ts=4 sts=4 sw=4:

