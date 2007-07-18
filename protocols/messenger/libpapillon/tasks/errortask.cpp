namespace Papillon
{
class ErrorMessageTask::Private
{
public:
	Private()
	{}
};

ErrorMessageTask::ErrorMessageTask(Task *parent)
 : Papillon::Task(parent), d(new Private)
{
}


ErrorMessageTask::~ErrorMessageTask()
{
	delete d;
}

/* 
 * handler error
 * See http://www.hypothetic.org/docs/msn/basics.php for a description of all error codes.
 */
void ErrorMessageTask::handlerError()
{
		kDebug(14140) << k_funcinfo << endl;
		QString msg;

		switch ( code )
		{
#if 0
			// We cant show message for error we don't know what they are or not related to the correct socket
			//  Theses following messages are not so instructive
			case 205:
				msg = i18n ( "An invalid username has been specified.\nPlease correct it, and try to reconnect.\n" );
				break;
			case 201:
				msg = i18n ( "Fully Qualified domain name missing.\n" );
				break;
			case 207:
				msg = i18n ( "You are already logged in.\n" );
				break;
			case 208:
				msg = i18n ( "You specified an invalid username.\nPlease correct it, and try to reconnect.\n");
				break;
			case 209:
				msg = i18n ( "Your nickname is invalid. Please check it, correct it,\nand try to reconnect.\n" );
				break;
			case 210:
				msg = i18n ( "Your list has reached its maximum capacity.\nNo more contacts can be added, unless you remove some first.\n" );
				break;
			case 216:
				msg = i18n ( "This user is not in your contact list.\n " );
				break;
			case 300:
				msg = i18n ( "Some required fields are missing. Please fill them in and try again.\n" );
				break;
			case 302:
				msg = i18n ( "You are not logged in.\n" );
				break;
#endif
			case 500:
				msg = i18n ( "An internal server error occurred. Please try again later." );
				break;
			case 502:
				msg = i18n ( "It is no longer possible to perform this operation. The Messenger server does not allow it anymore." );
				break;
			case 600:
			case 910:
			case 912:
			case 922:
				msg = i18n ( "The Messenger server is busy. Please try again later." );
				break;
			case 601:
			case 604:
			case 605:
			case 914:
			case 915:
			case 916:
			case 917:
				msg = i18n ( "The server is not available at the moment. Please try again later." );
				break;
			default:
				// FIXME: if the error causes a disconnect, it will crash, but we can't disconnect every time
				msg = i18n( "Unhandled Messenger error code %1 \n"
						"Please fill a bug report with a detailed description and if possible the last console debug output.", code );
				break;
		}

		if ( !msg.isEmpty() )
			//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, msg, i18n( "Messenger Plugin" ) );
			emit errorMessage( ErrorNormal, msg );

}

bool ErrorMessageTask::take(Transfer *transfer)
{
	bool isError;
	uint errorCode =  transfer->command().toUInt(&isError);
	uint id = transfer->transactionId();

	if( isError)
	{
		handlerError(errorCode,id)
		return true;
	}

	return false;	
}

}

