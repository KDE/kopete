namespace Papillon 
{

class LogoutTask::Private
{
public:
	Private()
}

LogoutTask::LogoutTask(Task *parent)
 : Task(parent), d(new Private)
{
}

LogoutTask::~LogoutTask()
{
	delete d;
}

void LogoutTask::onGo()
{
	qDebug() << PAPILLON_FUNCINFO << "send out OUT command...";
	sendLogoutCommand();
}

void LogoutTask::sendLogoutCommand()
{
	qDebug() << PAPILLON_FUNCINFO << "Sending OUT command.";
	Transfer *logoutTransfer = new Transfer(Transfer::TransactionTransfer);
	logoutTransfer->setCommand( QLatin1String("OUT") );
	logoutTransfer->setTransactionId( QString() );
	send(logoutTransfer);
}

bool LogoutTask::take(Transfer *transfer)
{
	if( forMe(transfer) )
	{
		//Out Error
		if(transfer->arguments()[0] == QLatin1String("OUH"))
		{

		}
		//Server Down
		if(transfer->arguments()[0] == QLatin1String("SSD"))
		{

		}
		return true;
	}
	return false
}

bool logoutTask::forMe(Transfer *transfer) const
{
	if( transfer->command() == QLatin1String("OUT") )
	{
		return true;
	}
	return false;	
}

}
