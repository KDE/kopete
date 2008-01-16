#include "action.h"

// Constructor without parameter
Action::Action()
{
	// Initialise an empty list
	this->m_argumentList.erase(this->m_argumentList.begin(),this->m_argumentList.end());
}

Action::Action(QString name)
{
	// Modify the action name
	this->setName(name);
	// Initialise an empty list
	this->m_argumentList.erase(this->m_argumentList.begin(),this->m_argumentList.end());
}

void Action::addArgument(QString name, QString direction, QString relatedStateVariable)
{
	// A boolean to know if the argument was found in the list
	bool find=false;
	// Argument creation
	Argument arg;
	arg.setName(name);
	arg.setDirection(direction);
	arg.setRelatedStateVariable(relatedStateVariable);
	
	// To go a the list beginning
	this->m_argumentList.begin();
	// We check if the argument is not already existing
	for(int i=0;i<this->m_argumentList.size() && !find;i++)
	{
		if(this->m_argumentList.last().name() == name)
		{
			find=true;
		}
	}
	if (find==false)
	{
		// Adding the argument to the list
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

void Action::setName(QString name)
{
	this->m_name = name;
}

void Action::viewListArgument()
{
	printf("## Displaying action arguments ##\n");
	for(int i =0; i < this->m_argumentList.size(); i++)
	{
		Argument arg = this->m_argumentList.at(i);
		printf("# %d # \n",i);
		// Show argument characteristics
		arg.viewArgument();
	}
}

