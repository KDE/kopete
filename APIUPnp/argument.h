/*
    argument.h -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>

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

#ifndef _ARGUMENT_H_
#define _ARGUMENT_H_

#include <QString>

class Argument
{
	private:
		// Argument name
		QString m_name;
		// Argument type (in, out)
		QString m_direction;
		// State variable linked to the argument
		QString m_relatedStateVariable;
	public:
		// Construtor
		Argument();
		// Destructor
		~Argument();
		// Getters
		QString name();
		QString direction();
		QString relatedStateVariable();
		// Setters
		void setName(QString name);
		void setDirection(QString direction);
		void setRelatedStateVariable(QString relatedStateVariable);
		// Method which show an argument
		void viewArgument();
		// Method which test the equality of two arguments
		bool operator==(const Argument &arg);
};


#endif
