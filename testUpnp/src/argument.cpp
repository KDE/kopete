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

Argument & Argument::operator=(const Argument &arg)
{
	this->m_name = arg.m_name;
	this->m_direction = arg.m_direction;
	this->m_relatedStateVariable = arg.m_relatedStateVariable;
	return(*this);
}

bool Argument::operator==(const Argument &arg)
{
	bool value = true;
	if(this->m_name != arg.m_name){value = false;}
	if(this->m_direction != arg.m_direction){value = false;}
	if(this->m_relatedStateVariable != arg.m_relatedStateVariable){value = false;}
	return value;
}
