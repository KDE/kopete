namespace Papillon
{

class InboxTask::Private
{
public:
	Private()
	{}
	QString currentTransactionId;
};

InboxTask::InboxTask(Task *parent)
 : Papillon::Task(parent), d(new Private)
{
}


InboxTask::~InboxTask()
{
	delete d;
}

void InboxTask::sendInboxCommand()
{
	qDebug() << PAPILLON_FUNCINFO << "Sending URL INBOX command.";
	Transfer *inboxTransfer = new Transfer(Transfer::TransactionTransfer);

	inboxTransfer->setCommand( QLatin1String("URL") );
	d->currentTransactionId = QString::number(connection()->transactionId()) ;
	inboxTransfer->setTransactionId( d->currentTransactionId );
	inboxTransfer->setArguments( QString("INBOX") );

	send(inboxTransfer);
}

InboxTask::onGo()
{
	sendInboxCommand();
}

bool InboxTask::take(Transfer *transfer)
{
	if( forMe(transfer) )
	{
		/*TODO emit signal of URL
		 * receive format is:  URL 9 /cgi-bin/HoTMaiL https://login.live.com/ppsecure/md5auth.srf?lc=2052 2
		 */
		return true;
	}

	return false;
}


bool InboxTask::forMe(Transfer *transfer) const
{
	if( transfer->command() == QLatin1String("URL") )
	{
		return true;
	}

	return false;	
}

}

#include "notifymessagetask.moc"
