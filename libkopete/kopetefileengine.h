/*
*	kopetefileengine.h - Kopete file engine
*
*	Copyright (c) 2007      by Guillermo A. Amaral B <me@guillermoamaral.com>
*	Kopete    (c) 2007      by the Kopete developers <kopete-devel@kde.org>
*
*	Based on Kopete Mime Source Factory
*	Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
*
*************************************************************************
*                                                                       *
* This library is free software; you can redistribute it and/or         *
* modify it under the terms of the GNU Lesser General Public            *
* License as published by the Free Software Foundation; either          *
* version 2 of the License, or (at your option) any later version.      *
*                                                                       *
*************************************************************************
*/

#ifndef KOPETEFILEENGINE_H
#define KOPETEFILEENGINE_H

#include <qabstractfileengine.h>
#include <qstring.h>
#include <QBuffer>

#include "kopete_export.h"

namespace Kopete
{
	class KOPETE_EXPORT FileEngineHandler : public QAbstractFileEngineHandler
	{
		public:
			QAbstractFileEngine *create(const QString &fileName) const;
	};

	class KOPETE_EXPORT FileEngine : public QAbstractFileEngine
	{
		public:
			FileEngine();
			~FileEngine();
			explicit FileEngine(const QString&);

			bool open(QIODevice::OpenMode openMode);
			bool close();
			qint64 size() const;
			qint64 pos() const;
			bool seek(qint64);
			bool isSequential() const;
			bool remove();
			bool rename(const QString &newName);
			bool mkdir(const QString &dirName, bool createParentDirectories) const;
			bool rmdir(const QString &dirName, bool recurseParentDirectories) const;
			bool setSize(qint64 size);
			bool caseSensitive() const;
			bool isRelativePath() const;
			QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;
			FileFlags fileFlags(FileFlags type) const;
			bool setPermissions(uint perms);
			QString fileName(FileName file=DefaultName) const;
			uint ownerId(FileOwner) const;
			QString owner(FileOwner) const;
			QDateTime fileTime(FileTime time) const;
			void setFileName(const QString &file);
			bool atEnd() const;

			qint64 read(char *data, qint64 maxlen);
			qint64 readLine(char *data, qint64 maxlen);
			qint64 write(const char *data, qint64 len);
		private:
			QString m_fileName;
			QByteArray m_data;
			QBuffer m_buffer;
	};
} // Kopete

#endif
