/*
    historyimport.cpp

    Copyright (c) 2010 by Timo Schluessler

    Kopete    (c) 2010 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "historyimport.h"

#include <QtCore/QStack>
#include <QtCore/QDir>
#include <QtGui/QTextEdit>
#include <QtGui/QTreeView>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QStandardItemModel>
#include <QtGui/QProgressDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>
#include <QtXml/QXmlStreamReader>

#include <kdebug.h>
#include <klocale.h>

#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopeteprotocol.h>
#include <kopeteaccount.h>
#include <kopetecontact.h>
#include <kopetemessage.h>

#include "historylogger.h"

HistoryImport::HistoryImport(QWidget *parent)
	: KDialog(parent)
{

	// set dialog settings
	setButtons(KDialog::Ok | KDialog::Details | KDialog::Cancel);
	setWindowTitle(KDialog::makeStandardCaption(i18n("Import History")));
	setButtonText(KDialog::Ok, i18n("Import Listed Logs"));

	// create widgets
	QWidget *w = new QWidget(this);
	QGridLayout *l = new QGridLayout(w);

	display = new QTextEdit(w);
	display->setReadOnly(true);
	treeView = new QTreeView(w);

	QPushButton *fromPidgin = new QPushButton(i18n("Get History From &Pidgin..."), w);
	
	l->addWidget(treeView, 0, 0, 1, 3);
	l->addWidget(display, 0, 4, 1, 10);
	l->addWidget(fromPidgin, 1, 0);

	setMainWidget(w);


	// create details widget
	QWidget *details = new QWidget(w);
	QVBoxLayout *dL = new QVBoxLayout(details);

	QTextEdit *detailsEdit = new QTextEdit(details);
	detailsEdit->setReadOnly(true);
	selectByHand = new QCheckBox(i18n("Select log directory by hand"), details);
	
	dL->addWidget(selectByHand);
	dL->addWidget(detailsEdit);

	setDetailsWidget(details);
	detailsCursor = QTextCursor(detailsEdit->document());

	// create model for treeView
	QStandardItemModel *model = new QStandardItemModel(treeView);
	treeView->setModel(model);
	model->setHorizontalHeaderLabels(QStringList(i18n("Parsed History")));

	// connect everything
	connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
	connect(fromPidgin, SIGNAL(clicked()), this, SLOT(importPidgin()));
	connect(this, SIGNAL(okClicked()), this, SLOT(save()));

	// define variables
	amount = 0;
	cancel = false;
	pidginImported = false;

	timeFormats << "(MM/dd/yyyy hh:mm:ss)" << "(MM/dd/yyyy hh:mm:ss AP)" << "(MM/dd/yy hh:mm:ss)" << "(MM/dd/yy hh:mm:ss AP)" << "(dd.MM.yyyy hh:mm:ss)" << "(dd.MM.yyyy hh:mm:ss AP)" << "(dd.MM.yy hh:mm:ss)" << "(dd.MM.yyyy hh:mm:ss AP)" << "(dd/MM/yyyy hh:mm:ss)" << "(dd/MM/yyyy hh:mm:ss AP)" << "(dd/MM/yy hh:mm:ss)" << "(dd/MM/yy hh:mm:ss AP)";

	show();
}

HistoryImport::~HistoryImport(void)
{
}

void HistoryImport::save(void)
{
	QProgressDialog progress(i18n("Saving logs to disk ..."), i18n("Abort Saving"), 0, amount, this);
	progress.setWindowTitle(i18n("Saving"));

	Log log;

	foreach (log, logs) {
		HistoryLogger logger(log.other, this);
		Message message;
		foreach (message, log.messages) {
			Kopete::Message kMessage;
			if (message.incoming) {
				kMessage = Kopete::Message(log.other, log.me);
				kMessage.setDirection(Kopete::Message::Inbound);
			} else {
				kMessage = Kopete::Message(log.me, log.other);
				kMessage.setDirection(Kopete::Message::Outbound);
			}
			kMessage.setPlainBody(message.text);
			kMessage.setTimestamp(message.timestamp);
			logger.appendMessage(kMessage, log.other);
			
			progress.setValue(progress.value()+1);
			qApp->processEvents();
			if (progress.wasCanceled()) {
				cancel = true;
				break;
			}
		}
		if (cancel)
			break;
	}
}

void HistoryImport::displayLog(struct Log *log)
{
	Message message;

	QList<QStandardItem*> items;
	QStringList strings;

	items << static_cast<QStandardItemModel*>(treeView->model())->invisibleRootItem();
	items << NULL << NULL << NULL;
	strings << "" << "" << "";

	foreach(message, log->messages) {
		amount++; // for QProgressDialog in save()

		strings[0] = log->other->protocol()->pluginId() + " (" + log->other->account()->accountId() + ')';
		strings[1] = log->other->displayName();
		strings[2] = message.timestamp.toString("yyyy-MM-dd");

		bool update = false;
		int i;
		for (i=1; i<4; i++) {
			if (update || !items.at(i) || items.at(i)->data(Qt::DisplayRole) != strings.at(i-1)) {
				items[i] = findItem(strings.at(i-1), items.at(i-1));
				update = true;
			} //else
				//kDebug(14310) << "using cached item";
		}

		if (!items.at(3)->data(Qt::UserRole).isValid())
			items[3]->setData((int)logs.indexOf(*log), Qt::UserRole);
	}

}

QStandardItem * HistoryImport::findItem(const QString &text, QStandardItem *parent)
{
	int i;
	bool found = false;
	QStandardItem *child = 0L;

	for (i=0; i < parent->rowCount(); i++) {
		child = parent->child(i, 0);
		if (child->data(Qt::DisplayRole) == text) {
			found = true;
			break;
		}
	}
	if (!found) {
		child = new QStandardItem(text);
		parent->appendRow(child);
	}

	return child;
}
		
void HistoryImport::itemClicked(const QModelIndex &index)
{
	QVariant id = index.data(Qt::UserRole);

	if (id.canConvert<int>()) {
		Log log = logs.at(id.toInt());
		display->document()->clear();
		QTextCursor cursor(display->document());

		Message message;
		QDate date = QDate::fromString(index.data(Qt::DisplayRole).toString(), "yyyy-MM-dd");
		foreach (message, log.messages) {
			if (date != message.timestamp.date())
				continue;
			cursor.insertText(message.timestamp.toString("hh:mm:ss "));
			if (message.incoming)
				cursor.insertText(log.other->displayName().append(": "));
			else
				cursor.insertText(log.me->displayName().append(": "));
			cursor.insertText(message.text);
			cursor.insertBlock();
		}
	}
}

int HistoryImport::countLogs(QDir dir, int depth)
{
	int res = 0;
	QStack<int> pos;
	QStringList files;
	pos.push(0);

	depth++;

	forever {
		files = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

		if (pos.size() == depth) {
			res += dir.entryList(QDir::Files).size();
		}
		if (files.isEmpty() || files.size() <= pos.top() || pos.size() == depth) {
			dir.cdUp();
			pos.pop();
			if (pos.isEmpty())
				break;
			pos.top()++;
		}
		else if (pos.size() != depth) {
			dir.cd(files.at(pos.top()));
			pos.push(0);
		}
	}

	return res;
}

void HistoryImport::importPidgin()
{
	if (pidginImported) {
		if (QMessageBox::question(this,
		 i18n("Are You Sure?"),
		 i18n("You already imported logs from pidgin. If you do it twice, each message is imported twice.\nAre you sure you want to continue?"),
		 QMessageBox::Yes | QMessageBox::No,
		 QMessageBox::No) != QMessageBox::Yes)
			return;
	}
	pidginImported = true;

	QDir logDir = QDir::homePath();
	if (selectByHand->isChecked() || !logDir.cd(".purple/logs"))
		logDir = QFileDialog::getExistingDirectory(mainWidget(), i18n("Select Log Directory"), QDir::homePath());

	int total = countLogs(logDir, 3);
	QProgressDialog progress(i18n("Parsing history from pidgin ..."), i18n("Abort parsing"), 0, total, mainWidget());
	progress.setWindowTitle(i18n("Parsing history"));
	progress.show();
	cancel = false;

	// protocolMap maps pidgin account-names to kopete protocol names (as in Kopete::Contact::protocol()->pluginId())
	QHash<QString, QString> protocolMap;
	protocolMap.insert("msn", "WlmProtocol");
	protocolMap.insert("icq", "ICQProtocol");
	protocolMap.insert("aim", "AIMProtocol");
	protocolMap.insert("jabber", "JabberProtocol");
	protocolMap.insert("yahoo", "YahooProtocol");
	protocolMap.insert("qq", "QQProtocol");
	protocolMap.insert("irc", "IRCProtocol");
	protocolMap.insert("gadu-gadu", "GaduProtocol");
	protocolMap.insert("bonjour", "BonjourProtocol");
	protocolMap.insert("meanwhile", "MeanwhileProtocol");

	QString protocolFolder;
	foreach (protocolFolder, logDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		logDir.cd(protocolFolder);

		QString accountFolder;
		foreach (accountFolder, logDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
			logDir.cd(accountFolder);

			// check if we can map the protocol
			if (!protocolMap.contains(protocolFolder)) {
				detailsCursor.insertText(i18n("WARNING: There is no equivalent for protocol %1 in kopete.\n", protocolFolder));
				logDir.cdUp();
				continue;
			}
			const QString & protocol = protocolMap.value(protocolFolder);

			// TODO use findContact?
			Kopete::ContactList * cList = Kopete::ContactList::self();
			QList<Kopete::Contact *> meList = cList->myself()->contacts();
			Kopete::Contact *me;
			bool found = false;
			foreach (me, meList) {
				if (me->protocol()->pluginId() == protocol && me->account()->accountId().contains(accountFolder, Qt::CaseInsensitive)) {
					found = true;
					break;
				}
			}
			if (!found) {
				detailsCursor.insertText(i18n("WARNING: Cannot find matching account for %1 (%2).\n", accountFolder, protocolFolder));
				logDir.cdUp();
				continue;
			}

			QString chatPartner;
			foreach (chatPartner, logDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
				logDir.cd(chatPartner);

				Kopete::Contact *other = cList->findContact(me->protocol()->pluginId(), me->account()->accountId(), chatPartner);
				struct Log log;
				if (!other) {
					detailsCursor.insertText(i18n("WARNING: Cannot find %1 (%2) in your contact list. Found logs will not be imported.\n", chatPartner, protocolFolder));
					logDir.cdUp();
					continue;
				}
				else {
					log.me = me;
					log.other = other;
				}

				QString logFile;
				QStringList filter;
				filter << "*.html" << "*.txt";
				foreach(logFile, logDir.entryList(filter, QDir::Files)) {
					QFile file(logDir.filePath(logFile));
					if (!file.open(QIODevice::ReadOnly)) {
						detailsCursor.insertText(i18n("WARNING: Cannot open file %1. Skipping.\n", logDir.filePath(logFile)));
						continue;
					}

					if (logFile.endsWith(".html"))
						parsePidginXml(file, &log, QDate::fromString(logFile.left(10), "yyyy-MM-dd"));
					else if (logFile.endsWith(".txt"))
						parsePidginTxt(file, &log, QDate::fromString(logFile.left(10), "yyyy-MM-dd"));

					file.close();

					progress.setValue(progress.value()+1);
					qApp->processEvents();
					if (cancel || progress.wasCanceled()) {
						cancel = true;
						break;
					}
				}

				logs.append(log);
				displayLog(&log);

				if (cancel)
					break;
				logDir.cdUp();
			}
			if (cancel)
				break;
			logDir.cdUp();
		}	
		if (cancel)
			break;
		logDir.cdUp();
	}

}

QDateTime HistoryImport::extractTime(const QString &string, QDate ref)
{
	QDateTime dateTime;
	QTime time;

	// try some formats used by pidgin
	if      ((time = QTime::fromString(string, "(hh:mm:ss)"))    .isValid());
	else if ((time = QTime::fromString(string, "(hh:mm:ss AP)")) .isValid());
	else {
		QString format;
		foreach (format, timeFormats) {
			if ((dateTime = QDateTime::fromString(string, format)).isValid())
				break;
		}
	}

	// check if the century in dateTime is equal to that of our date reference
	if (dateTime.isValid()) {
		int diff = ref.year() - dateTime.date().year();
		dateTime = dateTime.addYears(diff - (diff % 100));
	}

	// if string contains only a time we use ref as date
	if (time.isValid())
		dateTime = QDateTime(ref, time);

	// inform the user about the date problems
	if (!dateTime.isValid())
		detailsCursor.insertText(i18n("WARNING: Cannot parse date \"%1\". You may want to edit the file containing this date manually. (Example recognized date strings: \"%2\".)\n", string, dateTime.toString("yyyy-MM-dd hh:mm:ss")));


	return dateTime;
}

void HistoryImport::parsePidginTxt(QFile &file, struct Log *log, QDate date)
{
	QString line;
	QString nick;
	struct Message message;

	// this is to collect unknown nicknames (the list stores the index in log->messages of the messages that used the nickname)
	// the bool says if that nickname is incoming (only used when the list is empty)
	QHash<QString, QPair<bool, QList<int> > > nicknames;

	QTextStream str(&file);
	// utf-8 seems to be default for pidgins-txt logs
	str.setCodec("UTF-8");

	while (!str.atEnd()) {
		line = str.readLine();

		if (line[0] == '(') {
			if (!message.text.isEmpty()) {
				/*// message.text contains an unwished newline at the end
				if (message.text.endsWith('\n'))
					message.text.chop(1); */
				log->messages.append(message);
				message.text.clear();
			}

			int endTime = line.indexOf(')')+1;
			message.timestamp = extractTime(line.left(endTime), date);

			int nickEnd = QRegExp("\\s").indexIn(line, endTime + 1);
			// TODO what if a nickname consists of two words? is this possible?
			// the following while can't be used because in status logs there is no : after the nickname :(
			//while (line[nickEnd-1] != ':')
			//	nickEnd = QRegExp("\\").indexIn(line, nickEnd);
			if (line[nickEnd -1] != ':') // this line is a status message
				continue;

			nick = line.mid(endTime+1, nickEnd - endTime - 2); // -2 to delete the colon

			// detect if the message is in- or outbound
			if (nick == log->me->displayName())
				message.incoming = false;
			else if (nick == log->other->displayName())
				message.incoming = true;
			else if (knownNicks.contains(nick))
				message.incoming = knownNicks.value(nick);
			else {
				// store this nick for later decision
				nicknames[nick].second.append(log->messages.size());
			}
			nicknames[nick].first = message.incoming;

			if (cancel)
				return;

			message.text = line.mid(nickEnd + 1);
		}
		else if (line[0] == ' ') {
			// an already started message is continued in this line
			int start = QRegExp("\\S").indexIn(line);
			message.text.append('\n' + line.mid(start));
		}
	}
	if (!message.text.isEmpty())
		log->messages.append(message);

	// check if we can guess which nickname belongs to us
	QHash<QString, QPair<bool, QList<int> > >::iterator itr;
	QHash<QString, QPair<bool, QList<int> > >::iterator itr2;
	for (itr = nicknames.begin(); itr != nicknames.end(); ++itr) {
		if (itr->second.isEmpty()) // no work for this one
			continue;
		bool haveAnother = false, lastIncoming = false;
		// check against all other nicknames
		for (itr2 = nicknames.begin(); itr2 != nicknames.end(); ++itr2) {
			if (itr2 == itr) // skip ourselves
				continue;

			// if there is another unknown nickname, we have no chance to guess which is our
			if (!itr2->second.isEmpty())
				break;
			if (!haveAnother) {
				lastIncoming = itr2->first;
				haveAnother = true;
			} else {
				// when there are more than one known nicknames, but with different incoming-values, we also can't guess which is ours
				if (lastIncoming != itr2->first)
					break;
			}
		}
		// we now can guess the incoming value of itr, namely !lastIncoming
		if (haveAnother && itr2 == nicknames.end()) {
			// inform the user
			if (lastIncoming)
				detailsCursor.insertText(i18n("INFORMATION: Guessed %1 to be one of your nicks.\n", itr.key()));
			else
				detailsCursor.insertText(i18n("INFORMATION: Guessed %1 to be one of your buddys nicks.\n", itr.key()));

			knownNicks.insert(itr.key(), !lastIncoming);
			int i;
			for (i = 0; i < itr->second.size(); i++)
				log->messages[itr->second.at(i)].incoming = !lastIncoming;
			itr->second.clear(); // we are finished with these indexes
		}
	}

	// iterate once again over the nicknames to detect which nicks are still not known. simply ask the user!
	for (itr = nicknames.begin(); itr != nicknames.end(); ++itr) {
		if (itr->second.isEmpty()) // no word for this one
			continue;

		bool incoming;
		int r = QMessageBox::question(NULL,
			i18n("Cannot map Nickname to Account"),
			i18n("Did you ever use \"%1\" as nickname in your history?", itr.key()),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);

		if (r == QMessageBox::Yes) {
			knownNicks.insert(itr.key(), false);
			incoming = true;
		}
		else if (r == QMessageBox::No) {
			knownNicks.insert(itr.key(), true);
			incoming = false;
		}
		else {
			cancel = true;
			return;
		}

		// set the queried incoming value to our already stored Messages
		int i;
		for (i = 0; i < itr->second.size(); i++)
			log->messages[itr->second.at(i)].incoming = incoming;
	}
}
			

void HistoryImport::parsePidginXml(QFile &file, struct Log * log, QDate date)
{
	bool inMessage = false, textComes = false;
	int lineNumber = -1;
	struct Message msg;

	// unfortunately pidgin doesn't write <... /> for the <meta> tag
	QByteArray data = file.readAll();
	if (data.contains("<meta")) {
		int metaEnd = data.indexOf(">", data.indexOf("<meta"));
		if (data.at(metaEnd-1) != '/')
			data.insert(metaEnd, '/');
	}

	QXmlStreamReader reader(data);

	while (!reader.atEnd()) {
		reader.readNext();

		// pidgin writes one chat-message per line. so if we come to the next line, we can finish and save the current message
		if (inMessage && reader.lineNumber() != lineNumber) {
			if (!msg.text.isEmpty()) {
				msg.text = msg.text.trimmed(); // trimm especially unwished newlines and spaces
				log->messages.append(msg); // save messge for later import via HistoryLogger (see HistoryImport::save())
			}
			textComes = false;
			inMessage = false;
		}
		// when there is only the color attribute for the font-tag, this must be the beginning of a new message
		if (!inMessage && reader.isStartElement() && reader.name() == "font" && reader.attributes().size() == 1 && reader.attributes().first().name() == "color") {
			if (reader.attributes().value("color") == "#A82F2F")
				msg.incoming = true;
			else
				msg.incoming = false;

			while (reader.readNext() != QXmlStreamReader::Characters) { }; // skip tags
			msg.timestamp = extractTime(reader.text().toString(), date);
			msg.text.clear();
			lineNumber = reader.lineNumber();
			inMessage = true;
		}
		else if (inMessage && !textComes && reader.isStartElement() && reader.name() == "b") {
			reader.readNext(); // this is the nickname, which is followed by the messageText
			textComes = true;
		}
		else if (textComes && reader.isCharacters())
			msg.text += reader.text().toString(); // append text
		else if (textComes && reader.isStartElement() && reader.name() == "br")
			msg.text += '\n'; // append newline
	}

	if (reader.hasError()) {
		// we ignore error 4: premature end of document
		if (reader.error() != 4) {
			int i, pos = 0;
			for (i=1;i<reader.lineNumber();i++)
				pos = data.indexOf('\n', pos) + 1;
			detailsCursor.insertText(i18n("WARNING: XML parser error in %1 at line %2, character %3: %4",
				file.fileName(), reader.lineNumber(), reader.columnNumber(), reader.errorString()));
			detailsCursor.insertBlock();
			detailsCursor.insertText(i18n("\t%1", QString(data.mid(pos, data.indexOf('\n', pos) - pos))));
			detailsCursor.insertBlock();
		}
	} else if (inMessage) { // an unsaved message is still pending (this doesn't happen at least for my pidgin-logs - handle it anyway)
		msg.text = msg.text.trimmed(); // trimm especially unwished newlines and spaces
		log->messages.append(msg); // save messge for later import via HistoryLogger (see HistoryImport::save())
	}
}

#include "historyimport.moc"
