namespace Papillon
{
class NotifyErrorMessageTask::Private
{
public:
	Private()
	{}
};

NotifyErrorMessageTask::NotifyErrorMessageTask(Task *parent)
 : Papillon::ErrorMessageTask(parent), d(new Private)
{
}


NotifyErrorMessageTask::~NotifyErrorMessageTask()
{
	delete d;
}

/* 
 * handler error
 * See http://www.hypothetic.org/docs/msn/basics.php for a description of all error codes.
 */
void NotifyErrorMessageTask::handlerError()
{
	kDebug(14140) << k_funcinfo << endl;

	QString handle;
	if(m_tmpHandles.contains(id))
		handle=m_tmpHandles[id];

	// TODO: Add support for all of these!
	switch( code )
	{
		case 201:
		case 205:
		case 208:
			{
				QString msg = i18n( "<qt>The Messenger user '%1' does not exist.<br>Please check the Messenger ID.</qt>", handle );
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorSorry, msg );
				break;
			}
		case 207:
		case 218:
		case 540:
			{
				QString msg =i18n( "<qt>An internal error occurred in the Messenger plugin.<br>"
						"Messenger Error: %1<br>"
						"please send us a detailed bug report "
						"at kopete-devel@kde.org containing the raw debug output on the "
						"console (in gzipped format, as it is probably a lot of output.)" , code);
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information, msg , i18n( "Messenger Internal Error" ) );
				emit errorMessage( MessengerSocket::ErrorInternal, msg );
				break;

			}
		case 209:
			{
				if(handle==m_account->accountId())
				{
					QString msg = i18n( "Unable to change your display name.\n"
							"Please ensure your display is not too long and does not contains censored words." );
					//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) );
					emit errorMessage( MessengerSocket::ErrorSorry, msg );
				}
				/*else
				  {
				  QString msg = i18n( "You are trying to change the display name of a user who has not "
				  "confirmed his or her email address;\n"
				  "the contact was not renamed on the server." );
				  KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, msg, i18n( "Messenger Plugin" ) );
				  }*/
				break;
			}
		case 210:
			{
				QString msg = i18n("Your contact list is full; you cannot add any new contacts.");
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Contact List Full" ) );
				emit errorMessage( MessengerSocket::ErrorSorry, msg );
				break;
			}
		case 215:
			{
				QString msg = i18n( "<qt>The user '%1' already exists in this group on the Messenger server;<br>"
						"if Kopete does not show the user, please send us a detailed bug report "
						"at kopete-devel@kde.org containing the raw debug output on the "
						"console (in gzipped format, as it is probably a lot of output.)</qt>" , handle);
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information, msg, i18n( "Messenger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorInformation, msg );
				break;
			}
		case 216:
			{
				//This might happen is you rename an user if he is not in the contact list
				//currently, we just iniore;
				//TODO: try to don't rename user not in the list
				//actualy, the bug is in MessengerChatSession::slotUserJoined()
				break;
			}
		case 219:
			{
				QString msg = i18n( "The user '%1' seems to already be blocked or allowed on the server." , handle);
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorSorry, msg );
				break;
			}
		case 223:
			{
				QString msg = i18n( "You have reached the maximum number of groups:\n"
						"Messenger does not support more than 30 groups." );
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorSorry, msg );
				break;
			}
		case 224:
		case 225:
		case 230:
			{
				QString msg = i18n("Kopete is trying to perform an operation on a group or a contact that does not exists on the server.\n"
						"This might happen if the Kopete contact list and the Messenger-server contact list are not correctly synchronized; if this is the case, you probably should send a bug report.");
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information,msg, i18n( "Messernger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorInformation, msg );
				break;
			}

		case 229:
			{
				QString msg = i18n("The group name is too long; it has not been changed on the Messenger server.");
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Invalid Group Name - Messenger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorSorry, msg );
				break;
			}
		case 710:
			{
				QString msg = i18n( "You cannot open a Hotmail inbox because you do not have an Messenger account with a valid "
						"Hotmail or Messenger mailbox." );
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorSorry, msg );
				break;
			}
		case 715:
			{
				/*
				//if(handlev==m_account->accountId())
				QString msg = i18n( "Your email address has not been verified with the Messenger server.\n"
				"You should have received a mail with a link to confirm your email address.\n"
				"Some functions will be restricted if you do not confirm your email address." );
				KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) ); //TODO do not show again
				*/
				break;
			}
		case 800:
			{
				//This happen when too much commends are sent to the server.
				//the command will not be executed, too bad.
				// ignore it for now, as we don't really know what command it was.
				/*		QString msg = i18#n( "You are trying to change your status, or your display name too rapidly.\n"
						"This might happen if you added yourself to your own contact list." );
						KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) );
				//FIXME: try to fix this problem*/
				break;
			}
		case 911:
			m_disconnectReason=Kopete::Account::BadPassword;
			disconnect();
			break;
		case 913:
			{
				QString msg = i18n( "You can not send messages when you are offline or when you are invisible." );
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorSorry, msg );
				break;
			}
		case 923:
			{
				QString msg = i18n( "You are trying to perform an action you are not allowed to perform in 'kid mode'." );
				//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Messenger Plugin" ) );
				emit errorMessage( MessengerSocket::ErrorSorry, msg );
				break;
			}

		default:
			ErrorMessageTask::handleError( code, id );
			break;
	}

}

}
#include "notifyerrortask.moc"
