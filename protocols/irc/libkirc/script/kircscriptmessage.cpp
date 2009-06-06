
#include "kircmessage.h"

static
QScriptValue Message_new(QScriptValue &obj, const KIrc::Message &msg = KIrc::Message())
{
	QScriptEngine *engine = obj.engine();
	obj.setProperty("x", QScriptValue(engine, msg.x));
	obj.setProperty("y", QScriptValue(engine, msg.y));
	return obj;
}

static
QScriptValue Message_toScriptValue(QScriptEngine *engine, const KIrc::Message &msg)
{
	return Message_new(engine->newObject(), msg);
}

static
void Message_fromScriptValue(const QScriptValue &obj, KIrc::Message &msg)
{
	msg.x = obj.property("x").toInt32();
	msg.y = obj.property("y").toInt32();
}

static
QScriptValue Message_line(QScriptContext *ctx, QScriptEngine *eng)
{
	KIrc::Message msg;

	Message_fromScriptValue(ctx->thisObject(), msg);
	return msg.line();
}

// TODO: support copy constructor
static
QScriptValue Message_constructor(QScriptContext *ctx, QScriptEngine *eng)
{
	QScriptValue object;
	if (ctx->isCalledAsConstructor())
	{
		object = ctx->thisObject();
		return Message_new(object);
	}
	else
	{
		object = eng->newObject();
		object.setPrototype(ctx->callee().property("prototype"));
	}
//	object.setProperty("name", ctx->argument(0));
	return object;
 }

void KIrc::Script::scriptRegisterMessage(QScriptEngine &engine, QScriptValue namespaceObject)
{
	if (namespaceObject.isNull())
		namespaceObject = engine.globalObject();

	namespaceObject.setProperty("Message", engine.newFunction(Message_constructor));
	qScriptRegisterMetaType(engine, Message_toScriptValue, Message_fromScriptValue);
}

