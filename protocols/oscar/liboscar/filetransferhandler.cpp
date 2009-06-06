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
: QObject( fileTransferTask ), mFileTransferTask( fileTransferTask )
{
	connect( mFileTransferTask, SIGNAL(transferFinished()), this, SIGNAL(transferFinished()) );
	connect( mFileTransferTask, SIGNAL(transferCancelled()), this, SIGNAL(transferCancelled()) );
	connect( mFileTransferTask, SIGNAL(transferError(int, const QString&)), this, SIGNAL(transferError(int, const QString&)) );
	connect( mFileTransferTask, SIGNAL(transferProcessed(unsigned int)), this, SIGNAL(transferProcessed(unsigned int)) );

	connect( mFileTransferTask, SIGNAL(nextFile(const QString&, const QString&)),
	         this, SIGNAL(transferNextFile(const QString&, const QString&)) );
	connect( mFileTransferTask, SIGNAL(nextFile(const QString&, unsigned int)),
	         this, SIGNAL(transferNextFile(const QString&, unsigned int)) );
	connect( mFileTransferTask, SIGNAL(fileProcessed(unsigned int, unsigned int)),
	         this, SIGNAL(transferFileProcessed(unsigned int, unsigned int)) );
}

void FileTransferHandler::send()
{
	mFileTransferTask->go( Task::AutoDelete );
}

QString FileTransferHandler::internalId() const
{
	return mFileTransferTask->internalId();
}

QString FileTransferHandler::contact() const
{
	return mFileTransferTask->contactName();
}

QString FileTransferHandler::fileName() const
{
	return mFileTransferTask->fileName();
}

Oscar::WORD FileTransferHandler::fileCount() const
{
	return mFileTransferTask->fileCount();
}

Oscar::DWORD FileTransferHandler::totalSize() const
{
	return mFileTransferTask->totalSize();
}

QString FileTransferHandler::description() const
{
	return mFileTransferTask->description();
}

void FileTransferHandler::cancel()
{
	mFileTransferTask->doCancel();
}

void FileTransferHandler::save( const QString &directory )
{
	mFileTransferTask->doAccept( directory );
}

void FileTransferHandler::saveAs( const QStringList &fileNames )
{
	mFileTransferTask->doAccept( fileNames );
}

#include "filetransferhandler.moc"
