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
	printf("%s \n",this->name().toLatin1().data());
	printf("%s \n",this->direction().toLatin1().data());
	printf("%s \n",this->relatedStateVariable().toLatin1().data());
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
