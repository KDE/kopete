/**
 * Copyright (C)  2006  Jakub Stachowski <qbast@go2.pl>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "datatypes.h"

QDataStream &operator<<(QDataStream &out, const ContactDef &a)
{
	out << a.id << a.display;
	return out;
}

QDataStream &operator>>(QDataStream &in, ContactDef &a)
{
	in >> a.id >> a.display;
	return in;
}

QDataStream &operator<<(QDataStream &out, const ServiceDef &a)
{
	out << a.name  << a.type << a.contact << a.hostName << a.port << a.txt;
	return out;
}

QDataStream &operator>>(QDataStream &in, ServiceDef &a)
{
	in >> a.name >> a.type >> a.contact >> a.hostName >> a.port >> a.txt;
	return in;
}

bool operator==(const ServiceDef& a, const ServiceDef& b)
{
	return a.name==b.name && a.type==b.type && a.hostName==b.hostName && a.port==b.port && a.txt == b.txt;
}

