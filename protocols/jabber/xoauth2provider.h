/*
    xoauth2provider.h - X-OAuth2 provider for QCA

    Copyright (c) 2016 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef XOAUTH2PROVIDER_H
#define XOAUTH2PROVIDER_H

namespace QCA {
class Provider;
}

QCA::Provider *createProviderXOAuth2();

#endif // XOAUTH2PROVIDER_H
