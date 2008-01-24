/*
    action.cpp -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>
    Copyright (c) 2007-2008 by Julien Hubatzeck   <reineur31@gmail.com>
    Copyright (c) 2007-2008 by Michel Saliba      <msalibaba@gmail.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

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


