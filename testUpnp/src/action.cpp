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

void Action::addArgument(QString name, QString direction, QString relatedStateVariable)
{
	// A boolean to know if the argument was found in the list
	bool find=false;
	// Argument creation
	Argument arg;
	arg.setName(name);
	arg.setDirection(direction);
	arg.setRelatedStateVariable(relatedStateVariable);
	// We check if the argument is not already existing
	for(int i=0;i<this->listArgument()->size() && !find;i++)
	{
		if(*(this->getArgumentAt(i)) == arg)
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
	Argument* arg = new Argument();
	printf("## Displaying action arguments ##\n");
	for(int i =0; i < this->listArgument()->size(); i++)
	{
		arg = this->getArgumentAt(i);
		printf("##### Argument %d ##### \n",i);
		// Show argument characteristics
		(*arg).viewArgument();
		printf("####################### \n");
	}
	delete(arg);
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
			Argument* argument = new Argument();
			for(int i =0; i < this->listArgument()->size(); i++)
			{
				argument = this->getArgumentAt(i);
				if(((*argument) == (*(action.getArgumentAt(i))))==false)
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

Argument* Action::getArgumentAt(int i)
{
	return &((*this->listArgument())[i]);
}

void Action::viewAction()
{
	printf("Action name : %s \n",this->name().toLatin1().data());
	this->viewListArgument();
}
