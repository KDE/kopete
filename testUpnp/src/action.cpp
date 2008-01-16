#include "action.h"

// Constructor without parameter
Action::Action()
{
}

Action::Action(QString name)
{
	// Modify the action name
	this->setName(name);
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
	this->listArgument()->begin();
	// We check if the argument is not already existing
	for(int i=0;i<this->listArgument()->size() && !find;i++)
	{
		if(this->listArgument()->last().name() == name)
		{
			find=true;
		}
	}
	if (find==false)
	{
		// Adding the argument to the list
		this->listArgument()->append(arg);
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


QList<Argument>* Action::listArgument()
{
	return &(this->m_argumentList);
}

void Action::setName(QString name)
{
	this->m_name = name;
}

void Action::viewListArgument()
{
	printf("## Displaying action arguments ##\n");
	for(int i =0; i < this->listArgument()->size(); i++)
	{
		Argument arg = this->listArgument()->at(i);
		printf("# %d # \n",i);
		// Show argument characteristics
		arg.viewArgument();
	}
}

bool Action::operator==(const Action &act)
{
	bool equals = true;
	Action action = act;
	if(this->name() != action.name())
	{
		equals = false;
	}
	else
	{
		if(this->listArgument()->size() == action.listArgument()->size())
		{
			for(int i =0; i < this->listArgument()->size(); i++)
			{
				Argument argument = this->listArgument()->at(i);
				if((argument == action.listArgument()->at(i))==false)
				{
					equals = false;
				}
			}	
		}
		else
		{
			equals = false;
		}
	}
	return equals;
}


