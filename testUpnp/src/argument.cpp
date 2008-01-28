/*
    argument.cpp -

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

#include "argument.h"


Argument::Argument()
{
}

Argument::~Argument()
{
}

QString Argument::name()
{
	return this->m_name;
}
QString Argument::direction()
{
	return this->m_direction;
}
QString Argument::relatedStateVariable()
{
	return this->m_relatedStateVariable;
}

void Argument::setName(QString name)
{
	this->m_name = name;
}
void Argument::setDirection(QString direction)
{
	this->m_direction = direction;
}
void Argument::setRelatedStateVariable(QString relatedStateVariable)
{
	this->m_relatedStateVariable = relatedStateVariable;
}


void Argument::viewArgument()
{
	printf("Argument name : %s \n",this->name().toLatin1().data());
	printf("Argument direction : %s \n",this->direction().toLatin1().data());
	printf("Argument related state variable : %s \n",this->relatedStateVariable().toLatin1().data());
}

bool Argument::operator==(const Argument &arg)
{
	Argument argument = arg;
	if(this->name() == argument.name() && this->direction() == argument.direction() && this->relatedStateVariable() == argument.relatedStateVariable())
	{
		return true;
	}
	else
	{
		return false;
	}
}
