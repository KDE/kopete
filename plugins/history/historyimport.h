/*
    historylogger.cpp

    Copyright (c) 2008 by Timo Schluessler       <timo@schluessler.org>

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

#ifndef HISTORYIMPORT_H
#define HISTORYIMPORT_H

#include <QtGui/QTextCursor>
#include <QHash>

#include <kdialog.h>

class QDir;
class QStandardItem;
class QTreeView;
class QTextEdit;
class QCheckBox;
class QModelIndex;
class QFile;

namespace Kopete { class Contact; }

/**
 * @author Timo Schluessler <timo@schluessler.org>
 */
class HistoryImport : public KDialog
{
Q_OBJECT
public:

	HistoryImport(QWidget *parent);
	~HistoryImport();

private:

	/**
	 * Used for internal storage of a message.
	 */
	struct Message {
		bool incoming;
		QString text;
		QDateTime timestamp;
	};
		
	/**
	 * Holds the messages sent and received between two specified contacts.
	 */
	struct Log {
		Kopete::Contact *me;
		Kopete::Contact *other;
		QList<Message> messages;
		/**
		 * Comparison between the Message lists is not necessary because we need this operator
		 * only in displayLog() to get the index of this log in logs.
		 */
		bool operator==(const struct Log & cmp) {
			if (cmp.me == me && cmp.other == other/* && other.messages == messages*/)
				return true;
			else
				return false;
		}
	};
		
	/**
	 * Parses @param file and appends the found messages to @param log.
	 */
	void parsePidginXml(QFile &file, struct Log *log, QDate date);

	/**
	 * Parses @param file and appends the found messages to @param log.
	 */
	void parsePidginTxt(QFile &file, struct Log *log, QDate date);


	/**
	 * Inserts @param log into treeView and prepares to display it when clicking on it.
	 */
	void displayLog(struct Log *log);

	/**
	 * Looks up if an item with @param text exists already as child of @param parent.
	 * If not, it creates one.
	 */
	QStandardItem * findItem(const QString &text, QStandardItem *parent);

	/**
	 * @return the number of files found in @param depth th subdir of @param dir.
	 */
	int countLogs(QDir dir, int depth);

	/**
	 * Tries to extract the time from @param string using formats specified in timeFormats.
	 * @param ref is used when @param string doesn't contain a date or to adjust a found date.
	 * @param ref is taken from the filename of the log.
	 */
	QDateTime extractTime(const QString &string, QDate ref);

	QStringList timeFormats;

	QTreeView *treeView;
	QTextEdit *display;

	/**
	 * Used for adding details to the details widget.
	 */
	QTextCursor detailsCursor;

	/**
	 * Enables/disables auto search for log dirs.
	 */
	QCheckBox *selectByHand;

	/**
	 * Contains all already parsed logs.
	 */
	QList<Log> logs;

	/**
	 * Used for mapping nickname to account. See isNickIncoming().
	 */
	QHash<QString, bool> knownNicks;

	/**
	 * To warn the user when importing logs twice
	 */
	bool pidginImported;

	int amount;
	bool cancel;

private slots:
	/**
	 * Starts parsing history from pidgin.
	 * Default path is ~/.purple/logs.
	 */
	void importPidgin(void);

	void save(void);
	void itemClicked(const QModelIndex & index);
};

#endif
