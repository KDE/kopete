#include "action.h"

Action::Action(){;}

Action::Action(QString name)
{
	this->m_name = name;
	this->m_argumentList.erase(this->m_argumentList.begin(),this->m_argumentList.end());;
}

void Action::addArgument(QString name, QString direction, QString relatedStateVariable)
{
	bool find=false;

	Argument arg;
	arg.setName(name);
	arg.setDirection(direction);
	arg.setRelatedStateVariable(relatedStateVariable);

	this->m_argumentList.begin();
	//we check if the argument is not already existing
	for(int i=0;i<this->m_argumentList.size() && !find;i++)
	{
		if(this->m_argumentList.last().name() == name)
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

void Action::viewListArgument()
{
	printf("## Displaying action arguments ##\n");
	for(int i =0; i < this->m_argumentList.size(); i++)
	{
		//Action action = this->m_actionList.at(i);
		//printf("%s \n",action.name().toLatin1().data());
		Argument arg = this->m_argumentList.at(i);
		printf("# %d # \n",i);
		printf("%s \n",arg.name().toLatin1().data());
		printf("%s \n",arg.direction().toLatin1().data());
		printf("%s \n",arg.relatedStateVariable().toLatin1().data());
	}
}

