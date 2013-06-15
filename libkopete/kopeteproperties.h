/*
    kopeteproperties.h - Kopete Properties

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPROPERTIES_H
#define KOPETEPROPERTIES_H

#include <QMultiHash>
#include <QVariant>

#include <typeinfo>

class QString;
class QDomElement;

namespace Kopete
{

/**
 * Contains the classes forming Kopete's Properties system.
 *
 * @todo Explain more, give examples.
 *
 * @author Richard Smith <kde@metafoo.co.uk>
 */
namespace Properties
{

//BEGIN core functionality

/**
 * @brief Property-type-independent base class for properties
 *
 * The base class for all properties of any type which can be set or got for @p Parent
 * objects. It is rare to need to use this class directly. Usually you will want to use
 * the @ref Property derived class, or dynamic_cast the PropertyBase object to another interface.
 *
 * @see Property UserVisible XMLSerializable StringSerializable
 *
 * @author Richard Smith <kde@metafoo.co.uk>
 */
template<class Parent>
class PropertyBase
{
public:
	/**
	 * Returns the name of the property. This name should uniquely identify this property
	 * within the type Parent, and will be used for persistently identifying this property.
	 *
	 * For core properties, the chosen name should not contain any slash characters. For
	 * properties defined in plugins kept in Kopete's CVS, the name should be of the form
	 * pluginName/propertyName. For third-party plugins, please use a URL with a host which
	 * you own, such as "http://my-host.com/kopete/properties/groupId".
	 *
	 * @return the name of this property.
	 */
	virtual const QByteArray &name() const = 0;
};

/**
 * @brief Property-type-dependent base class for properties
 *
 * This class represents a property of type @p Type applicable to @p Parent objects. Usage
 * of this class is usually as simple as:
 *
 * \code
 * SomeParent *propertyContainer = ...
 * Property<SomeParent,QString> &myProperty = ...
 * QString value = propertyContainer->property(myProperty);
 * propertyContainer->setProperty(myProperty, "hello");
 * \endcode
 *
 * You should never need to call functions in this class directly.
 */
template<class Parent, typename Type>
class Property : public PropertyBase<Parent>
{
public:
	/**
	 * Returns the value of this property in the object @p parent.
	 */
	virtual Type get( const Parent *parent ) const = 0;
	/**
	 * Sets the value of this property in the object @p parent.
	 */
	virtual void set( Parent *, const Type & ) const = 0;
};

/**
 * @brief Base class for property data objects
 *
 * Some property objects want to store property-specific data in their parent objects.
 * To support that, subclasses of this class are permitted to be stored. Once passed
 * to the @ref PropertyStorage object via @ref PropertyStorage::setCustomPropertyData,
 * the @ref PropertyStorage object owns the PropertyData, and will delete it when it
 * is no longer needed.
 */
struct PropertyData
{
	virtual ~PropertyData() {}
};

/**
 * @brief Storage object for PropertyData objects
 *
 * This class is responsible for storing PropertyData-derived data objects for properties.
 * This is the non-templated part of the @ref WithProperties class, split out into its own
 * class to eliminate the template bloat.
 */
class PropertyStorage
{
	typedef QMultiHash<QByteArray, PropertyData*> PropertyDict;
	// setCustomPropertyData can be called on a const object, allowing the
	// guarantee that DataProperty::data() never returns 0.
	mutable PropertyDict _storage;

public:
	PropertyStorage() {}
	~PropertyStorage()
	{
		qDeleteAll(_storage);
	}

	/**
	 * Sets the stored property data with name @p name to be @p data.
	 *
	 * @note The @p name argument should usually be the name of the property which the data
	 * is being stored for. However, if properties wish to share data, they may choose to
	 * name their custom data differently. Names are bound by the same rules as are laid out
	 * for naming properties in PropertyBase<Parent>::name.
	 */
	void setCustomPropertyData( const QByteArray &name, PropertyData *data ) const { _storage.replace( name, data ); }

	/**
	 * Gets the stored property data with name @p name. Returns a null
	 * pointer if no data has been stored for that property.
	 */
	PropertyData *getCustomPropertyData( const QByteArray &name ) const { return _storage.value(name); }
};

/**
 * @brief Base class for classes to which properties can be applied
 *
 * This class provides support for properties to another class. If you want your class
 * to support properties, derive from this passing your class as the Parent parameter:
 *
 * \code
 * class YourClass : public WithProperties<YourClass> { ... };
 * \endcode
 *
 * You will also need to explicitly specify the propertyCreated() member function to
 * load property data upon creation of a new property object.
 */
template<class Parent>
class WithProperties : public PropertyStorage
{
public:
	/**
	 * Get the value of property @p prop in this object.
	 * @param prop the Property object representing the property to get
	 */
	template<typename T>
	T property( Property<Parent,T> const &prop ) { return prop.get( static_cast<Parent*>(this) ); }
	/**
	 * Set the value of property @p prop in this object.
	 * @param prop the Property object representing the property to get
	 * @param value the value to set the property to
	 */
	template<typename T>
	void setProperty( Property<Parent,T> const &prop, const T &value ) { prop.set( static_cast<Parent*>(this), value ); }

	/**
	 * Called when a property is created; loads the Parent object's data into the property.
	 *
	 * @note Derived classes must explicitly specialize this to load the property's data into
	 *       every object of this type.
	 */
	static void propertyCreated( const PropertyBase<Parent> &property );
};

//END core functionality

//BEGIN interfaces

/**
 * @brief An interface for user-visible properties
 * @todo document
 */
template<class Parent>
struct UserVisible
{
	virtual QString userText( Parent * ) = 0;
	virtual QString label() = 0;
	virtual QString icon() = 0;
};

/**
 * @brief An interface for properties which can be serialized as XML
 * @todo document
 */
template<class Parent>
struct XMLSerializable
{
	virtual void fromXML( Parent *, const QDomElement & ) = 0;
	virtual void toXML( const Parent *, QDomElement & ) = 0;
};

/**
 * @brief An interface for properties which can be serialized as strings
 * @todo document
 */
template<class Parent>
struct StringSerializable
{
	virtual void fromString( Parent *, const QString & ) = 0;
	virtual QString toString( const Parent * ) = 0;
};

//END interfaces

//BEGIN convenience classes

/**
 * @internal Display a warning message when the wrong type of property data is found
 */
void customPropertyDataIncorrectType( const char *name, const std::type_info &found, const std::type_info &expected );

/**
 * @brief Convenience implementation of a Property that stores PropertyData
 *
 * A property for objects of type @p Parent, that stores data in the class @p Data.
 * @p Data must be derived from @ref PropertyBase, or your code will not compile.
 */
template<class Parent, typename Type, class Data>
class DataProperty : public Property<Parent,Type>
{
public:
	Data *data( const Parent *c ) const
	{
		PropertyData *pd = c->getCustomPropertyData( this->name() );
		Data *data = dynamic_cast<Data*>(pd);
		if ( !data )
		{
			if ( pd )
				customPropertyDataIncorrectType( this->name(), typeid(*pd), typeid(Data) );
			data = new Data;
			c->setCustomPropertyData( this->name(), data );
		}
		return data;
	}
};

/**
 * @brief Convenience implementation of a PropertyData subclass which stores a single datum
 *
 * If a @ref Property needs to store only a single value in an object, using this
 * class is simpler than deriving from @ref PropertyData yourself. The value will
 * be default-constructed (which means for numeric types and pointers it will be
 * set to 0).
 */
template<typename T>
struct SimplePropertyData : public PropertyData
{
	SimplePropertyData() : value() {}
	T value;
};

/**
 * @brief Convenience implementation of a Property which stores a single datum as PropertyData
 *
 * This convenience class implements the @ref Property interface by simply storing and
 * retrieving the datum from PropertyData. This class does not provide any serialization
 * of the data.
 *
 * @note You will need to derive from this class to use it; the @ref name function is
 * still pure virtual.
 */
template<class Parent, typename Type>
class SimpleDataProperty : public DataProperty<Parent,Type,SimplePropertyData<Type> >
{
public:
	Type get( const Parent *p ) const { return data(p)->value; }
	void set( Parent *p, const Type &v ) const { data(p)->value = v; }
};

/**
 * Move somewhere else
 * @{
 */

/**
 * Explicitly specialized for all types QVariant supports
 */
template<class T> T variantTo(QVariant);

QVariant variantFromXML(const QDomElement&);
void variantToXML(QVariant v, QDomElement &);

/**
 * @}
 */

/**
 * @brief Convenience implementation of XMLSerializable in terms of QVariants
 *
 * This class provides XML serialization for data that can be stored in a QVariant. You
 * will need to multiply-inherit from this class and (usually indirectly) from @ref Property.
 *
 * You can combine this class with other convenience classes such as SimpleDataProperty
 * like this:
 *
 * \code
 * class ContactNickNameProperty
 *     : public SimpleDataProperty<Contact,QString>
 *     , XMLProperty<ContactNickNameProperty,Contact,QString>
 * {
 * public:
 *   const char *name() const { return "nickName"; }
 * };
 * \endcode
 */
template<class Derived, class Parent, typename Type>
class XMLProperty : public XMLSerializable<Parent>
{
public:
	void fromXML( Parent *t, const QDomElement &e )
	{
		static_cast<Derived*>(this)->set(t, variantTo<Type>(variantFromXML(e)));
	}
	void toXML( const Parent *t, QDomElement &e )
	{
		variantToXML(QVariant(static_cast<Derived*>(this)->get(t)),e);
	}
};

//END convenience classes

} // namespace Properties

} // namespace Kopete

#endif
