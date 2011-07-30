/*
    filetransferhandler.cpp  -  File Transfer Handler

    Copyright (c) 2008 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "filetransferhandler.h"

#include "filetransfertask.h"

FileTransferHandler::FileTransferHandler( FileTransferTask* fileTransferTask )
: QObject( fileTransferTask ), mFileTransferTask( fileTransferTask ), mFileTransferDone( false )
{
	connect( mFileTransferTask, SIGNAL(transferFinished()), this, SLOT(emitTransferFinished()) );
	connect( mFileTransferTask, SIGNAL(transferCancelled()), this, SLOT(emitTransferCancelled()) );
	connect( mFileTransferTask, SIGNAL(transferError(int,QString)), this, SLOT(emitTransferError(int,QString)) );
	connect( mFileTransferTask, SIGNAL(transferProcessed(uint)), this, SIGNAL(transferProcessed(uint)) );

	connect( mFileTransferTask, SIGNAL(nextFile(QString,QString)),
	         this, SIGNAL(transferNextFile(QString,QString)) );
	connect( mFileTransferTask, SIGNAL(nextFile(QString,uint)),
	         this, SIGNAL(transferNextFile(QString,uint)) );
	connect( mFileTransferTask, SIGNAL(fileProcessed(uint,uint)),
	         this, SIGNAL(transferFileProcessed(uint,uint)) );
}

FileTransferHandler::~FileTransferHandler()
{
	if ( !mFileTransferDone )
		emit transferCancelled();
}

void FileTransferHandler::send()
{
	if ( mFileTransferTask )
		mFileTransferTask->go( Task::AutoDelete );
}

QString FileTransferHandler::internalId() const
{
	if ( !mFileTransferTask )
		return QString();

	return mFileTransferTask->internalId();
}

QString FileTransferHandler::contact() const
{
	if ( !mFileTransferTask )
		return QString();

	return mFileTransferTask->contactName();
}

QString FileTransferHandler::fileName() const
{
	if ( !mFileTransferTask )
		return QString();

	return mFileTransferTask->fileName();
}

Oscar::WORD FileTransferHandler::fileCount() const
{
	if ( !mFileTransferTask )
		return 0;

	return mFileTransferTask->fileCount();
}

Oscar::DWORD FileTransferHandler::totalSize() const
{
	if ( !mFileTransferTask )
		return 0;

	return mFileTransferTask->totalSize();
}

QString FileTransferHandler::description() const
{
	if ( !mFileTransferTask )
		return QString();

	return mFileTransferTask->description();
}

void FileTransferHandler::cancel()
{
	if ( mFileTransferTask )
		mFileTransferTask->doCancel();
}

void FileTransferHandler::save( const QString &directory )
{
	if ( mFileTransferTask )
		mFileTransferTask->doAccept( directory );
}

void FileTransferHandler::saveAs( const QStringList &fileNames )
{
	if ( mFileTransferTask )
		mFileTransferTask->doAccept( fileNames );
}

void FileTransferHandler::emitTransferCancelled()
{
	mFileTransferDone = true;
	disconnect( mFileTransferTask, 0, this, 0 );
	emit transferCancelled();
}

void FileTransferHandler::emitTransferError( int errorCode, const QString &error )
{
	mFileTransferDone = true;
	disconnect( mFileTransferTask, 0, this, 0 );
	emit transferError( errorCode, error );
}

void FileTransferHandler::emitTransferFinished()
{
	mFileTransferDone = true;
	disconnect( mFileTransferTask, 0, this, 0 );
	emit transferFinished();
}

#include "filetransferhandler.moc"
