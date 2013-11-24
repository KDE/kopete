/*
    texteffectplugin.cpp  -  description
    
    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "texteffectplugin.h"

#include <stdlib.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <QColor>

#include "kopetechatsessionmanager.h"

#include "texteffectconfig.h"

K_PLUGIN_FACTORY(TextEffectPluginFactory, registerPlugin<TextEffectPlugin>();)
K_EXPORT_PLUGIN(TextEffectPluginFactory( "kopete_texteffect" ))

TextEffectPlugin::TextEffectPlugin( QObject *parent, const QVariantList &/*args*/ )
: Kopete::Plugin( TextEffectPluginFactory::componentData(), parent )
{
	if( !pluginStatic_ )
		pluginStatic_=this;

	m_config = new TextEffectConfig;
	m_config->load();

	connect ( this , SIGNAL(settingsChanged()) , this , SLOT(slotSettingsChanged()) );

	connect( Kopete::ChatSessionManager::self(),
		SIGNAL(aboutToSend(Kopete::Message&)),
		SLOT(slotOutgoingMessage(Kopete::Message&)) );

	 last_color=0;
}

TextEffectPlugin::~TextEffectPlugin()
{
	delete m_config;
	pluginStatic_ = 0L;
}

TextEffectPlugin* TextEffectPlugin::plugin()
{
	return pluginStatic_ ;
}

TextEffectPlugin* TextEffectPlugin::pluginStatic_ = 0L;


void TextEffectPlugin::slotOutgoingMessage( Kopete::Message& msg )
{
	if(msg.direction() != Kopete::Message::Outbound)
		return;

	QStringList colors=m_config->colors();

	if(m_config->colorChar() || m_config->colorWords() || m_config->lamer() || m_config->waves() )
	{
		QString original=msg.plainBody();
		QString resultat;

		int c=0;
		bool wavein=false;

		for(int f=0;f<original.length();f++)
		{
			QChar x=original[f];
			if(f==0 || m_config->colorChar() || (m_config->colorWords() && x==' ' ))
			{
				if(f!=0)
					resultat+="</font>";
				resultat+="<font color=\"";
				resultat+=colors[c];
				if(m_config->colorRandom())
					c=rand()%colors.count();
				else
				{
					c++;
					if(c >= colors.count())
						c=0;
				}
				resultat+="\">";
			}
			switch (x.toLatin1())
			{
				case '>':
					resultat+="&gt;";
					break;
				case '<':
					resultat+="&lt;";
					break;
				case '&':
					resultat+="&amp;";
					break;
				case '\n':
					resultat+="<br>";
				case 'a':
				case 'A':
					if(m_config->lamer())
					{
						resultat+='4';
						break;
					} //else, go to the default,  all other case have this check
				case 'e':
				case 'E':
					if(m_config->lamer())
					{
						resultat+='3';
						break;
					}//else, go to the default,  all other case have this check
				case 'i':
				case 'I':
					if(m_config->lamer())
					{
						resultat+='1';
						break;
					}//else, go to the default,  all other case have this check
				case 'l':
				case 'L':
					if(m_config->lamer())
					{
						resultat+='|';
						break;
					}//else, go to the default,  all other case have this check
				case 't':
				case 'T':
					if(m_config->lamer())
					{
						resultat+='7';
						break;
					}//else, go to the default,  all other case have this check
				case 's':
				case 'S':
					if(m_config->lamer())
					{
						resultat+='5';
						break;
					}//else, go to the default,  all other case have this check
				case 'o':
				case 'O':
					if(m_config->lamer())
					{
						resultat+='0';
						break;
					}//else, go to the default,  all other case have this check
				default:
					if(m_config->waves())
					{
						resultat+= wavein ? x.toLower() : x.toUpper();
						wavein=!wavein;
					}
					else
						resultat+=x;
					break;
			}
		}
		if( m_config->colorChar() || m_config->colorWords() )
			resultat+="</font>";
		msg.setHtmlBody(resultat);
	}

	if(m_config->colorLines())
	{
		if(m_config->colorRandom())
		{
			last_color=rand()%colors.count();
		}
		else
		{
			last_color++;
			if(last_color >= colors.count())
				last_color=0;
		}

		msg.setForegroundColor(QColor (colors[last_color]));
	}
}

void TextEffectPlugin::slotSettingsChanged()
{
	m_config->load();
}


#include "texteffectplugin.moc"

