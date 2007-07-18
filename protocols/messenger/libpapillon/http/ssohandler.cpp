/*
   ssohandler.cpp - Windows Live Messenger SSO authentication

   Copyright (c) 2007 by Zhang PanYong <pyzhang@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

namespace Papillon 
{
class SSOHandler::Private
{
public:
	Private()
	 : stream(0), success(false)
	{}

	~Private()
	{
		delete stream;
	}

	SecureStream *stream;
	bool success;

};

SSOHandler::SSOHandler(SecureStream *stream)
 : QObject(0), d(new Private)
{

}

SSOHandler::~SSOHandler()
{
	delete d;
}

}
#include "ssohandler.moc"
