/*
    kircfunctors.h - IRC Client

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCFUNCTORS_H
#define KIRCFUNCTORS_H

#include "kircmessage.h"

class KIRCMethodFunctorCall
{
public:
	inline operator bool(void)
		{ return isValid(); }

	virtual bool isValid()=0;

	virtual bool checkMsgValidity(const KIRCMessage &msg)=0;

	virtual bool operator()(const KIRCMessage &msg)=0;

	virtual ~KIRCMethodFunctorCall()
		{}
};

class KIRCMethodFunctor_Ignore
	: public KIRCMethodFunctorCall
{
public:
	virtual bool isValid()
		{ return true; }

	bool checkMsgValidity(const KIRCMessage &)
		{ return true; }

	virtual bool operator()(const KIRCMessage &)
		{ return true; }

	KIRCMethodFunctor_Ignore()
		{}
};


template <class TClass> class KIRCMethodFunctorBase
	: public KIRCMethodFunctorCall
{
protected:
	TClass *m_obj;
	bool (TClass::*m_method)(const KIRCMessage &msg);
	int m_argsSize_min, m_argsSize_max;
	QString m_helpMessage;

public:
	virtual bool isValid()
		{ return (m_obj!=0)&&(m_method!=0); }

	bool checkMsgValidity(const KIRCMessage &msg)
		{
			int argsSize = msg.args().size();
			if (m_argsSize_min>=0 && argsSize < m_argsSize_min)
				return false;
			#ifdef _IRC_STRICTNESS_
			if (m_argsSize_max>=0 && argsSize > m_argsSize_max)
				return false;
			#endif // _IRC_STRICTNESS_
			return true;
		}

	QString getHelpMessage()
		{ return m_helpMessage; }

	virtual bool operator()(const KIRCMessage &msg)=0;

		KIRCMethodFunctorBase(TClass *obj,  bool (TClass::*method)(const KIRCMessage &msg),
			 int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: m_obj(obj),m_method(method),
		  m_argsSize_min(argsSize_min), m_argsSize_max(argsSize_max), m_helpMessage(QString::fromLatin1(helpMessage))
		{}

	virtual ~KIRCMethodFunctorBase<TClass>()
		{}
};


template <class TClass> class KIRCMethodFunctor_Forward
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &msg)
		{
			if(this->isValid())
			{
				bool (TClass::*tmp)(const KIRCMessage&) = (bool (TClass::*)(const KIRCMessage&))(this->m_method);
				return (*this->m_obj.*tmp)(msg);
			}
			return false;
		}

	KIRCMethodFunctor_Forward<TClass>(TClass *obj,
			bool (TClass::*method)(const KIRCMessage &msg),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};


template <class TClass> class KIRCMethodFunctor_Empty
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &)
		{
			if(this->isValid())
			{
				void (TClass::*tmp)() = (void (TClass::*)())this->m_method;
				emit (*this->m_obj.*tmp)();
				return true;
			}
			return false;
		}

	KIRCMethodFunctor_Empty<TClass>(TClass *obj, void (TClass::*method)(),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};


template <class TClass, unsigned int argindex> class KIRCMethodFunctor_S
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &msg)
		{
			if(this->isValid())
			{
				void (TClass::*tmp)(const QString&) = (void (TClass::*)(const QString&))this->m_method;
				emit (*this->m_obj.*tmp)(msg.args()[argindex]);
				return true;
			}
			return false;
		}

		KIRCMethodFunctor_S(TClass *obj, void (TClass::*method)(const QString &),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};


template <class TClass> class KIRCMethodFunctor_S_Prefix
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &msg)
		{
			if(this->isValid())
			{
				void (TClass::*tmp)(const QString&) = (void (TClass::*)(const QString&))this->m_method;
				emit (*this->m_obj.*tmp)(msg.prefix());
				return true;
			}
			return false;
		}

	KIRCMethodFunctor_S_Prefix(TClass *obj, void (TClass::*method)(const QString &),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};


template <class TClass> class KIRCMethodFunctor_S_Suffix
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &msg)
		{
			if(this->isValid())
			{
				void (TClass::*tmp)(const QString&) = (void (TClass::*)(const QString&))this->m_method;
				emit (*this->m_obj.*tmp)(msg.suffix());
				return true;
			}
			return false;
		}

	KIRCMethodFunctor_S_Suffix<TClass>(TClass *obj, void (TClass::*method)(const QString &),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};


template <class TClass, unsigned int argindex> class KIRCMethodFunctor_SS
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &msg)
		{
			if(this->isValid())
			{
				void (TClass::*tmp)(const QString&, const QString&) = (void (TClass::*)(const QString&, const QString&))this->m_method;
				emit (*this->m_obj.*tmp)(msg.args()[this->type], msg.args()[this->type+1]);
				return true;
			}
			return false;
		}

		KIRCMethodFunctor_SS(TClass *obj, void (TClass::*method)(const QString&,const QString&),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};


template <class TClass, unsigned int argindex> class KIRCMethodFunctor_SS_Prefix
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &msg)
		{
			if(this->isValid())
			{
				void (TClass::*tmp)(const QString&, const QString&) = (void (TClass::*)(const QString&, const QString&))this->m_method;
				emit (*this->m_obj.*tmp)(msg.prefix(), msg.args()[argindex]);
				return true;
			}
			return false;
		}

		KIRCMethodFunctor_SS_Prefix(TClass *obj, void (TClass::*method)(const QString&,const QString&),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};


template <class TClass, unsigned int argindex> class KIRCMethodFunctor_SS_Suffix
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &msg)
		{
			if(this->isValid())
			{
				void (TClass::*tmp)(const QString&, const QString&) = (void (TClass::*)(const QString&, const QString&))this->m_method;
				emit (*this->m_obj.*tmp)(msg.args()[argindex], msg.suffix());
				return true;
			}
			return false;
		}

		KIRCMethodFunctor_SS_Suffix(TClass *obj, void (TClass::*method)(const QString&,const QString&),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};


template <class TClass> class KIRCMethodFunctor_SS_PrefixSuffix
	: public KIRCMethodFunctorBase<TClass>
{
public:
	virtual bool operator()(const KIRCMessage &msg)
		{
			if(this->isValid())
			{
				void (TClass::*tmp)(const QString&, const QString&) = (void (TClass::*)(const QString&, const QString&))this->m_method;
				emit (*this->m_obj.*tmp)(msg.prefix(), msg.suffix());
				return true;
			}
			return false;
		}

	KIRCMethodFunctor_SS_PrefixSuffix<TClass>(TClass *obj, void (TClass::*method)(const QString&,const QString&),
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0)
		: KIRCMethodFunctorBase<TClass>(obj, (bool (TClass::*)(const KIRCMessage &msg))method,
						argsSize_min, argsSize_max, helpMessage)
		{}
};

#endif // KIRCFUNCTORS_H
