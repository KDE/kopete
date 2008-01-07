#include "action.h"

Action::Action(QString name)
{
	this->m_name = name;
	this->m_argumentList.erase(this->m_argumentList.begin(),this->m_argumentList.end());;
}

void Action::addArgument(QString name, QString direction, QString relatedStateVariable)
{
	bool find=false;

	Argument arg;
	arg.name=name;
	arg.direction=direction;
	arg.relatedStateVariable=relatedStateVariable;

	this->m_argumentList.begin();
	//we check if the argument is not already existing
	for(int i=0;i<this->m_argumentList.size() && !find;i++)
	{
		if(this->m_argumentList.last().name == name)
		{
			find=true;
		}
	}
	if (find==false)
	{
		this->m_argumentList.append(arg);
	}
	else
	{
		printf("The argument exist\n");
	}
}

QString Action::name()
{
	return this->m_name;
}


QList<Argument> Action::listArgument()
{
	return this->m_argumentList;
}


