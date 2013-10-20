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

#include "kopetefileengine.h"

#include "kopeteaccountmanager.h"
#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetepicture.h"

#include <kdebug.h>
#include <kiconloader.h>

#include <QPixmap>
#include <qdiriterator.h>
#include <qstring.h>
#include <qstringlist.h>

namespace Kopete
{
	QAbstractFileEngine *FileEngineHandler::create(const QString &fileName) const
	{
		bool handle = false;
		handle |= fileName.startsWith("kopete-account-icon", Qt::CaseInsensitive);
		handle |= fileName.startsWith("kopete-contact-icon", Qt::CaseInsensitive);
		handle |= fileName.startsWith("kopete-metacontact-icon", Qt::CaseInsensitive);
		handle |= fileName.startsWith("kopete-metacontact-photo", Qt::CaseInsensitive);
		handle |= fileName.startsWith("kopete-onlinestatus-icon", Qt::CaseInsensitive);
		return handle ? new FileEngine(fileName) : 0;
	}

	FileEngine::FileEngine()
	: m_buffer(&m_data)
	{
		kDebug(14010) ;
	}

	FileEngine::FileEngine(const QString& fileName)
	: m_fileName(fileName), m_buffer(&m_data)
	{
		kDebug(14010) << fileName;
	}

	FileEngine::~FileEngine()
	{
		kDebug(14010) << m_fileName;
	}

	bool FileEngine::open(QIODevice::OpenMode openMode)
	{
		kDebug(14010) << m_fileName << " " << openMode;

		// flag used to signal something went wrong when creating a mimesource
		bool completed = false;

		QPixmap tmpPixmap;
		QImage tmpImage;

		// extract and decode arguments
		QStringList parts = m_fileName.split(QChar(':'), QString::SkipEmptyParts);
		QStringList::iterator partsEnd = parts.end();
		for (QStringList::iterator it = parts.begin(); it != partsEnd; ++it)
			*it = QUrl::fromPercentEncoding((*it).toUtf8());

		if (parts[0] == QString::fromLatin1("kopete-contact-icon"))
		{
			if (parts.size() >= 4)
			{
				Account *account = AccountManager::self()->findAccount(parts[1], parts[2]);

				if (account)
				{
					Contact *contact = account->contacts().value( parts[3] );

					if (contact)
					{
						tmpPixmap = QPixmap(contact->onlineStatus().iconFor(contact).pixmap(16));
						completed = true;
					}
					else
					{
						kDebug(14010) << "kopete-contact-icon: contact not found";
					}
				}
				else
				{
					kDebug(14010) << "kopete-contact-icon: account not found";
				}
			}
			else
			{
				kDebug(14010) << "kopete-contact-icon: insufficient information in m_fileName: " << parts;
			}
		}

		if (parts[0] == QString::fromLatin1("kopete-account-icon"))
		{
			if (parts.size() >= 3)
			{
				Account *account = AccountManager::self()->findAccount(parts[1], parts[2]);

				if (account)
				{
					tmpPixmap = QPixmap(account->myself()->onlineStatus().iconFor(account->myself()).pixmap(16));
					completed = true;
				}
				else
				{
					kDebug(14010) << "kopete-account-icon: account not found";
				}
			}
			else
			{
				kDebug(14010) << "kopete-account-icon: insufficient information in m_fileName: " << parts;
			}
		}

		if (parts[0] == QString::fromLatin1("kopete-metacontact-icon"))
		{
			if (parts.size() >= 2)
			{
				MetaContact *mc = ContactList::self()->metaContact(parts[1]);

				if (mc)
				{
					tmpPixmap = QPixmap(SmallIcon(mc->statusIcon()));
					completed = true;
				}
			}
			else
			{
				kDebug(14010) << "kopete-metacontact-icon: insufficient information in m_fileName: " << parts;
			}
		}

		if (parts[0] == QString::fromLatin1("kopete-metacontact-photo"))
		{
			if (parts.size() >= 2)
			{
				MetaContact *mc = ContactList::self()->metaContact(parts[1]);

				if (mc)
				{
					tmpImage = QImage(mc->picture().image());
					completed = true;
				}
			}
			else
			{
				kDebug(14010) << "kopete-metacontact-photo: insufficient information in m_fileName: " << parts;
			}
		}

		if (parts[0] == QString::fromLatin1("kopete-onlinestatus-icon"))
		{
			if (parts.size() >= 2)
			{
				/*
				* We are using a dirty trick here: this mime source is supposed to return the
				* icon for an arbitrary KOS instance. To do this, the caller needs to ask
				* the KOS for the mime source key first, which also ensures the icon is
				* currently in the cache. The cache is global, so we just need to find any
				* existing KOS instance to return us the rendered icon from the cache.
				* To find a valid KOS, we ask Kopete's account manager to locate an existing
				* account. We'll use the myself() instance of that account to reference its
				* current KOS object, which in turn has access to the global KOS icon cache.
				* Note that if the cache has been invalidated in the meantime, we'll just
				* get an empty pixmap back.
				*/

				Account *account = AccountManager::self()->accounts().first();

				if (account)
				{
					tmpPixmap = QPixmap(account->myself()->onlineStatus().iconFor(parts[1]));
					completed = true;
				}
				else
				{
					kDebug(14010) << "kopete-onlinestatus-icon: no active account found";
				}
			}
			else
			{
				kDebug(14010) << "kopete-onlinestatus-icon: insufficient information in m_fileName: " << parts;
			}
		}

		close();

		if (completed)
		{
			m_buffer.open(QIODevice::WriteOnly);

			if (!tmpImage.isNull())
			{
				tmpImage.save(&m_buffer, "JPEG");
			}
			else
			{
				if (!tmpPixmap.isNull())
				{
					tmpPixmap.save(&m_buffer, "PNG");
				}
				else
				{
					completed = false;
				}
			}

			m_buffer.close();
			m_buffer.open(openMode);
			m_buffer.seek(0);
		}

		kDebug(14010) << "return: " << completed;

		return completed;
	}

	bool FileEngine::close()
	{
		kDebug(14010) ;

		if(m_buffer.isOpen())
		{
			m_buffer.close();
		}

		m_data.clear();

		return true;
	}

	qint64 FileEngine::size() const
	{
		return m_buffer.size();
	}

	qint64 FileEngine::pos() const
	{
		return m_buffer.pos();
	}

	bool FileEngine::seek(qint64 newPos)
	{
		return m_buffer.seek(newPos);
	}

	bool FileEngine::isSequential() const
	{
		return false;
	}

	bool FileEngine::remove()
	{
		return false;
	}

	bool FileEngine::rename(const QString &newName)
	{
		Q_UNUSED(newName);
		return false;
	}

	bool FileEngine::mkdir(const QString &dirName, bool createParentDirectories) const
	{
		Q_UNUSED(dirName);
		Q_UNUSED(createParentDirectories);
		return false;
	}

	bool FileEngine::rmdir(const QString &dirName, bool recurseParentDirectories) const
	{
		Q_UNUSED(dirName);
		Q_UNUSED(recurseParentDirectories);
		return false;
	}

	bool FileEngine::setSize(qint64 size)
	{
		Q_UNUSED(size);
		return false;
	}

	bool FileEngine::caseSensitive() const
	{
		return false;
	}

	bool FileEngine::isRelativePath() const
	{
		return false;
	}

	QStringList FileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
	{
		Q_UNUSED(filters);
		Q_UNUSED(filterNames);
		return QStringList();
	}

	FileEngine::FileFlags FileEngine::fileFlags(FileFlags type) const
	{
		Q_UNUSED(type);
		return 0;
	}

	bool FileEngine::setPermissions(uint perms)
	{
		Q_UNUSED(perms);
		return false;
	}

	QString FileEngine::fileName(FileName file) const
	{
		Q_UNUSED(file);
		return m_fileName;
	}

	uint FileEngine::ownerId(FileOwner owner) const
	{
		Q_UNUSED(owner);
		return 0;
	}

	QString FileEngine::owner(FileOwner owner) const
	{
		Q_UNUSED(owner);
		return QString();
	}

	QDateTime FileEngine::fileTime(FileTime time) const
	{
		Q_UNUSED(time);
		return QDateTime();
	}

	void FileEngine::setFileName(const QString &file)
	{
		m_fileName = file;
	}

	qint64 FileEngine::read(char *data, qint64 maxlen)
	{
		return m_buffer.read(data, maxlen);;
	}

	qint64 FileEngine::readLine(char *data, qint64 maxlen)
	{
		return m_buffer.readLine(data, maxlen);
	}

	qint64 FileEngine::write(const char *data, qint64 len)
	{
		Q_UNUSED(data);
		Q_UNUSED(len);
		return -1;
	}

	bool FileEngine::atEnd() const
	{
		return m_buffer.atEnd();
	}
} // END namespace Kopete
