
#ifndef KIRC_SCRIPT_H
#define KIRC_SCRIPT_H

namespace KIrc
{

class Script
{
public:

	static void scriptRegisterMessage(QScriptEngine &engine, QScriptValue &namespaceObject = QScriptValue());
};

};

#endif
