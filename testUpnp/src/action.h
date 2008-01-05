#ifndef _ACTION_H_
#define _ACTION_H_

#include <QList>
#include <QString>


typedef struct{
	QString name;
	QString direction;
	QString relatedStateVariable;
}Argument;

class Action
{
	private:
		QString m_name;
		QList<Argument> m_argumentList;
	public:
		Action(QString name);
		void addArgument(QString name, QString direction, QString relatedStateVariable);
		QString name();
		QList<Argument> listArgument();
};

#endif

