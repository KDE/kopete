
#ifndef KOPETE_ERROR_HANDLER_H
#define KOPETE_ERROR_HANDLER_H

#include "qobject.h"
#include "qstring.h"
#include "qpixmap.h"
#include "kopete_export.h"

namespace Kopete
{
namespace ErrorNotifier
{

/*
 * Severity of the error, this allows for
 * non being intrusive with non important errors
 */
enum ErrorType
{
	/*
	 * Is better to the user to be aware, but if
	 * the user doesn't read it it should not matter.
	 */
	TypeInfo = 0,
	/*
	 * Is better to the user to be aware, like network
	 * experimenting problems, but should not be instrusive
	 */
	TypeWarning = 1,
	/*
	 * The user did something is not possible by the program
	 */
	TypeSorry = 2,
	/*
	 * The error should be read by the user
	 */
	TypeError = 3,
	/*
	 * The error is fatal and irrecoverable
	 */
	TypeFatal = 4
};

typedef struct
{
	QString caption;
	QString message;
	QString explaination;
	QString debugInfo;
	QPixmap picture;
} ErrorNotification;

/**
	 * Notifies an error to the user. Avoiding the case of plugin messing with popups dialogs.
	 * @param sev Severity of the error. See @ref Severity for more information about the Severities wich
	 * will determine how the error is displayed.
	 * @param caption brief subject line, used where possible if the presentation allows it.
	 * @param message A short description of the error.
	 * @param explaination A long description on how the error occured and what the user can do about it.
	 * @param debugInfo Debug info that can be sent to the developers or to the network service owners.
	 */
void KOPETE_EXPORT notify( ErrorType sev, const QString &caption, const QString &message, const QString explaination = QString::null, const QString debugInfo = QString::null);

class KNotification;

class ErrorHandler : public QObject
{
Q_OBJECT
public:
	static ErrorHandler* self();
	void registerNotification(KNotification* event, ErrorNotification error);
public slots:
	//void slotEventActivated(unsigned int action);
private:
	ErrorHandler();
	~ErrorHandler();
	class Private;
	Private *d;
	static ErrorHandler *s_self;
};

} // end ns ErrorNotifier
} // end ns Kopete

#endif