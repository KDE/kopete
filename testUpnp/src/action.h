#ifndef _ACTION_H_
#define _ACTION_H_

#include <QList>


typedef struct{
	char * name;
	char * direction;
	char * relatedStateVariable;
}Argument;

class Action
{
	private:
		char * name;
		QList<Argument> argumentList;
	public:
		Action(char *name);
		void addArgument(char *name, char* direction, char* relatedStateVariable);
		char* getName();
		QList<Argument> getListArgument();
};

#endif

