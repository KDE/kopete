#include "bindingobject.h"
#include "bindingobject.moc"

BindingObject::BindingObject( QObject *parent, const char *name )
    : QObject( parent, name )
{
}

BindingObject::~BindingObject()
{
}

