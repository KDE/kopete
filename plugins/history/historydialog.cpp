/*
    kopetehistorydialog.cpp - Kopete History Dialog

    Copyright (c) 2002 by  Richard Stellingwerff <remenic@linuxfromscratch.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "historydialog.h"

#include "historylogger.h"
#include "kopetemetacontact.h"

#include <sys/time.h>


#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <ktextbrowser.h>

#define CBUFLENGTH 512 // buffer length for fgets()

HistoryDialog::HistoryDialog( KopeteContact *mContact, bool showclose, int count, QWidget* parent, const char* name )
	: KDialogBase( KDialogBase::Plain, i18n("History for %1").arg( mContact->displayName() ), KDialogBase::Close, KDialogBase::Close , parent, name, false)
{
	kdDebug(14010) << k_funcinfo << "called." << endl;
	setWFlags(Qt::WDestructiveClose);	// send SIGNAL(closing()) on quit

	showButton(KDialogBase::Close, showclose); // hide Close button if showClose is false

	m_logger= new HistoryLogger(mContact,this);
	m_logger->setReversed(true);
	connect(m_logger, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));


	buildWidget(count);


	// show the dialog before people get impatient
	show();

	init();
}

HistoryDialog::HistoryDialog( KopeteMetaContact *mContact, bool showClose, int count, QWidget* parent, const char* name )
	: KDialogBase( Plain, i18n("History for %1").arg( mContact->displayName() ), Close, Close, parent, name, false)
{
	kdDebug(14010) << k_funcinfo << "called." << endl;
	setWFlags(Qt::WDestructiveClose);	// send SIGNAL(closing()) on quit

	showButton(KDialogBase::Close, showClose); // hide Close button if showClose is false

	m_logger= new HistoryLogger(mContact->contacts().first(),this);
	m_logger->setReversed(true);
	connect(m_logger, SIGNAL( addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ) , this , SLOT (addMessage( KopeteMessage::MessageDirection , QString , QString , QString  ) ));


	buildWidget(count);


	// show the dialog before people get impatient
	show();

	init();
}




void HistoryDialog::buildWidget(int count)
{
	msgStart = 0; // always display newest message first
	msgCount = count; // 50 by default
	mUser = "";
	mSuperBuffer = "";

 	QHBoxLayout *alayout = new QHBoxLayout( plainPage() );
	QWidget *mHistoryWidget = new QWidget( plainPage(), "mHistoryWidget" );
	alayout->addWidget( mHistoryWidget );
	mHistoryWidget->setMinimumHeight(400);
	setMainWidget(mHistoryWidget);

	layout = new QGridLayout( mHistoryWidget, 1, 1, 6, 6 );
	mHistoryView = new KTextBrowser(mHistoryWidget, "mHistoryView");

	layout->addMultiCellWidget( mHistoryView, 0, 0, 0, 5);

	optionsBox = new QGroupBox(mHistoryWidget, "optionsBox");
	optionsBox->setTitle( i18n("Options") );
	optionsBox->setColumnLayout(0, Qt::Vertical);
	optionsBox->layout()->setSpacing( 6 );
	optionsBox->layout()->setMargin( 11 );

	optionsLayout = new QGridLayout( optionsBox->layout() );
	optionsLayout->setAlignment( Qt::AlignTop );

	optionsCBLayout = new QHBoxLayout(0, 0, 6, "optionsCBLayout");

	mSearchLabel = new QLabel(optionsBox, "mSearchLabel");
	mSearchLabel->setText( i18n("Search:") );
	optionsCBLayout->addWidget(mSearchLabel);

	mSearchInput = new QLineEdit(optionsBox, "mSearchInput");
	optionsCBLayout->addWidget(mSearchInput);
	mSearchInput->setFocus();

	mSearchButton = new QPushButton( optionsBox, "mSearchButton" );
	mSearchButton->setText( i18n("&Search") );
	mSearchButton->setDefault( true );

	optionsCBLayout->addWidget( mSearchButton );

	optionsLayout->addMultiCellLayout( optionsCBLayout, 0, 0, 0, 1 );

	mReverse = new QCheckBox( optionsBox, "mReverse" );
	mReverse->setText( i18n("Show &oldest message first") );

	optionsLayout->addWidget( mReverse, 1, 0 );

	mIncoming = new QCheckBox( optionsBox, "mIncoming" );
	mIncoming->setText( i18n("Only show &incoming messages") );

	optionsLayout->addWidget( mIncoming, 1, 1 );

	layout->addMultiCellWidget( optionsBox, 1, 1, 0, 5 );

	mBack = new QPushButton( mHistoryWidget, "mBack" );
	mBack->setPixmap( SmallIcon( QString::fromLatin1( "2leftarrow" ) ) );

	layout->addWidget( mBack, 2, 0 );

	mPrevious = new QPushButton(mHistoryWidget, "mPrevious");
	mPrevious->setPixmap( SmallIcon( QString::fromLatin1( "1leftarrow" ) ) );

	layout->addWidget( mPrevious, 2, 1 );

	mNext = new QPushButton(mHistoryWidget, "mNext");
	mNext->setPixmap( SmallIcon( QString::fromLatin1( "1rightarrow" ) ) );

	layout->addWidget( mNext, 2, 2 );

	mForward = new QPushButton(mHistoryWidget, "mForward");
	mForward->setPixmap( SmallIcon( QString::fromLatin1( "2rightarrow" ) ) );

	layout->addWidget( mForward, 2, 3 );

	mProgress = new QProgressBar(50, mHistoryWidget, "progress");
	mProgress->setCenterIndicator( true );

	layout->addMultiCellWidget(mProgress, 2, 2, 4, 5);

	// all buttons disabled by default
/*	mNext->setEnabled( false );
	mPrevious->setEnabled( false );
	mBack->setEnabled( false );
	mForward->setEnabled( false );
	optionsBox->setEnabled( false );*/

	connect( mNext, SIGNAL(clicked()), this, SLOT(slotNextClicked()));
	connect( mPrevious, SIGNAL(clicked()), this, SLOT(slotPrevClicked()));
	connect( mForward, SIGNAL(clicked()), this, SLOT(slotForwardClicked()));
	connect( mBack, SIGNAL(clicked()), this, SLOT(slotBackClicked()));

	connect( mReverse, SIGNAL(toggled(bool)), this, SLOT(slotReversedToggled(bool)));
	connect( mIncoming, SIGNAL(toggled(bool)), this, SLOT(slotIncomingToggled(bool)));
	connect( mSearchButton, SIGNAL(clicked()), this, SLOT(slotSearchClicked()));

	refreshEnabled();
}

void HistoryDialog::init()
{
	mSuperBuffer=QString::null;
	optionsBox->setEnabled( false );
	m_logger->readLog(0 , msgCount);
	mHistoryView->setText(mSuperBuffer);
	refreshEnabled();
}

void HistoryDialog::addMessage(KopeteMessage::MessageDirection dir, QString nick, QString date, QString body)
{
	kdDebug(14010) << k_funcinfo << "received message: " << body << endl;
	QString message = QString::fromLatin1( "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">" );

	if( dir == KopeteMessage::Inbound )
	{
		message += QString::fromLatin1( "<tr><td><font color=\"#0360B1\"><b>" ) +
			i18n( "Message from %1 at %2:" ).arg( nick ).arg( date );
	}
	else
	{
		message += QString::fromLatin1( "<tr><td><font color=\"#E11919\"><b>" ) +
			i18n( "Message to %1 at %2:" ).arg( nick ).arg( date );
	}

	message += QString::fromLatin1(
		"</b></font></td></tr></table>\n"
		"<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\"><tr><td>" ) +
		body.stripWhiteSpace() + QString::fromLatin1( "</tr></td></table><br><br>" );

	if( mSuperBuffer.isEmpty() )
	{
		mSuperBuffer = message + QString::fromLatin1( "\n" );
	}
	else
	{
		if( mReverse->isOn() )
			mSuperBuffer += message + '\n';
		else
			mSuperBuffer.prepend( message + '\n' );
	}
}

void HistoryDialog::slotPrevClicked()
{
	msgStart -= msgCount;
	mSuperBuffer = "";
	refreshEnabled();
	m_logger->readLog(msgStart , msgCount);
	mHistoryView->setText(mSuperBuffer);
}

void HistoryDialog::slotNextClicked()
{
	msgStart += msgCount;
	mSuperBuffer = "";
	refreshEnabled();
	m_logger->readLog(msgStart, msgCount);
	mHistoryView->setText(mSuperBuffer);
}

void HistoryDialog::slotBackClicked()
{
	msgStart = 0;
	mSuperBuffer = "";
	refreshEnabled();
	m_logger->readLog(msgStart, msgCount);
	mHistoryView->setText(mSuperBuffer);
}

void HistoryDialog::slotForwardClicked()
{
	msgStart = m_logger->totalMessages()-msgCount;
	mSuperBuffer = "";

	refreshEnabled();
	m_logger->readLog(msgStart, msgCount);
	mHistoryView->setText(mSuperBuffer);
}

void HistoryDialog::slotSearchClicked()
{
/*	kdDebug(14010) << "[HistoryWidget] slotSearchClicked()" << endl;

	if (mSearchInput->text().isEmpty())
		return;

	mPrevious->setEnabled( false );
	mNext->setEnabled( false );

	mSuperBuffer = "";

	QDomElement msgelement;
	QDomNode node;

	KopeteMessage::MessageDirection dir;

	int currentPos=0;
	int steps = messages / 20; // split the scrollbar into 20 parts.

	if ( steps == 0 )
		steps = 1; // avoid division by zero

	QString body, date, nick;
	QString buffer, msgBlock;

	char cbuf[CBUFLENGTH]; // buffer for the log file

	mProgress->setTotalSteps(20); // split the scrollbar into 20 parts.

	//Loop through the logfiles in the list, process each one
	for ( QStringList::Iterator it = logFileNames.begin(); it != logFileNames.end(); ++it )
	{
		QString logFileName = *it;

		// open the file
		FILE *f = fopen(QFile::encodeName(logFileName), "r");

		// create a new <message> block
		while ( ! feof( f ) )
		{
			fgets(cbuf, CBUFLENGTH, f);
			buffer = QString::fromUtf8(cbuf);

			while ( strchr(cbuf, '\n') == NULL && !feof(f) )
			{
				fgets( cbuf, CBUFLENGTH, f );
				buffer += QString::fromUtf8(cbuf);
			}

			if( buffer.startsWith( QString::fromLatin1( "<message " ) ) )
			{
				msgBlock = buffer;

				// find the end of the message block
				while ( ! feof( f ) &&  strcmp("</message>\n", cbuf ) )  // buffer != "</message>\n"
				{
					fgets(cbuf, CBUFLENGTH, f);
					buffer = QString::fromUtf8(cbuf);

					while ( strchr(cbuf, '\n') == NULL && !feof(f) )
					{
						fgets( cbuf, CBUFLENGTH, f );
						buffer += QString::fromUtf8(cbuf);
					}

					msgBlock.append(buffer);
				}

				// now let's work on this new block
				xmllist->setContent(msgBlock, false);
				msgelement = xmllist->documentElement();
				node = msgelement.firstChild();

				if( msgelement.attribute( QString::fromLatin1( "direction" ) ) == QString::fromLatin1( "inbound" ) )
					dir = KopeteMessage::Inbound;
				else
				{
					dir = KopeteMessage::Outbound;

					// skip outbound messages if only incoming messages want to be seen
					if (mIncoming->isOn())
						continue;
				}

				nick = date = body = "";

				// Read all the elements.
				while ( ! node.isNull() )
				{
					QString tagname;
					QDomElement element;

					if ( node.isElement() )
					{
						element = node.toElement();
						tagname = element.tagName();

						if( dir == KopeteMessage::Inbound && tagname == QString::fromLatin1( "srcnick" ) )
							nick = element.text();
						else if( dir == KopeteMessage::Outbound && tagname == QString::fromLatin1( "destnick" ) )
							nick = element.text();
						else if( tagname == QString::fromLatin1( "date" ) )
							date = element.text();
						else if( tagname == QString::fromLatin1( "body" ) )
						{
							if (!element.text().contains(mSearchInput->text()))
								break;

							body = element.text();
						}
					}

					node = node.nextSibling();
				}

				// add the message to the history view

				if (!body.isEmpty())
					addMessage(dir, nick, date, body);

				currentPos++;

				mProgress->setProgress (currentPos / steps);
				qApp->processEvents();
			}
		}

		fclose( f );
	}

	mProgress->reset();

	if ( ! mSuperBuffer.isEmpty() )
		mHistoryView->setText(mSuperBuffer);
	else
		mHistoryView->setText(i18n("No matches found."));
*/
}

void HistoryDialog::slotReversedToggled( bool  b  )
{
	// FIXME: Honour the bool!
	mSuperBuffer = "";
	msgStart = 0;
	m_logger->setReversed( !b );
	m_logger->readLog(msgStart,msgCount);
	mHistoryView->setText(mSuperBuffer);
	refreshEnabled();
}

void HistoryDialog::slotIncomingToggled( bool  b  )
{
	mSuperBuffer = "";
	msgStart = 0;
	m_logger->setHideOutgoing( b );
	m_logger->readLog(msgStart,msgCount);
	mHistoryView->setText(mSuperBuffer);
	refreshEnabled();
}


void HistoryDialog::refreshEnabled( )
{
	if(msgStart < 0)
		msgStart=0;

	if(msgStart+msgCount > m_logger->totalMessages())
		msgStart=m_logger->totalMessages()-msgCount;

	if (msgStart <= 0)
	{
		mPrevious->setEnabled(false);
		mBack->setEnabled(false);
	}
	else
	{
		mPrevious->setEnabled(true);
		mBack->setEnabled(true);
	}
	if (msgStart+msgCount >= m_logger->totalMessages())
	{
		mNext->setEnabled( false );
		mForward->setEnabled( false );
	}
	else
	{
		mForward->setEnabled( true );
		mNext->setEnabled(true);
	}

	if ( m_logger->totalMessages() == 0 )
	{
		//There are no messages for this contact
		mHistoryView->setText(i18n("No history for user %1.").arg(mUser));
		optionsBox->setEnabled( false );
	}
	else
	{
		// enable the options GroupBox
		optionsBox->setEnabled( true );
	}

}


#include "historydialog.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

