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
		// Method witch show an argument
		void viewArgument();
};


#endif
