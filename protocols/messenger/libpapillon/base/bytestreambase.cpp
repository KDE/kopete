//
// ByteStream
//
// Authors:
//   Gregg Edghill (Gregg.Edghill@gmail.com)
//
// Copyright (C) 2007, Kopete (http://kopete.kde.org)
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of this software.
//
// THIS LIBRARY IS FREE SOFTWARE; YOU CAN REDISTRIBUTE IT AND/OR
// MODIFY IT UNDER THE TERMS OF THE GNU LESSER GENERAL PUBLIC
// LICENSE AS PUBLISHED BY THE FREE SOFTWARE FOUNDATION; EITHER
// VERSION 2 OF THE LICENSE, OR (AT YOUR OPTION) ANY LATER VERSION.
//

#include "Papillon/Base/ByteStreamBase"

namespace Papillon
{

ByteStreamBase::ByteStreamBase(QObject *parent) : QObject(parent)
{
}

ByteStreamBase::~ByteStreamBase()
{
}

qint64 ByteStreamBase::bytesAvailable() const
{
	return 0l;
}

qint64 ByteStreamBase::bytesToWrite() const
{
	return 0l;
}

void ByteStreamBase::close()
{}

}

#include "bytestreambase.moc"
