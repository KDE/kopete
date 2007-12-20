#include "action.h"

Action::Action(char * name)
{
	this->name = name;
	this->argumentList.erase(this->argumentList.begin(),this->argumentList.end());;
}

void Action::addArgument(char *name, char* direction, char* relatedStateVariable)
{
	bool find=false;

	Argument arg;
	arg.name=name;
	arg.direction=direction;
	arg.relatedStateVariable=relatedStateVariable;

	this->argumentList.begin();
	//on verifie que l'argument existe pas deja
	for(int i=0;i<this->argumentList.size() && !find;i++)
	{
		if(strcmp(this->argumentList.last().name,name)==0)
		{
			find=true;
		}
	}
	if (find==false)
	{
		this->argumentList.append(arg);
	}
	else
	{
		printf("The argument exist\n");
	}
}

char* Action::getName()
{
	return this->name;
}


QList<Argument> Action::getListArgument()
{
	return this->argumentList;
}


