/***************************************************************************
                          texteffectplugin.cpp  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
 

#include <kdebug.h>
#include <kgenericfactory.h>

#include "kopetemessagemanagerfactory.h"

#include "texteffectplugin.h"
#include "texteffectpreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_texteffect, KGenericFactory<TextEffectPlugin> );

TextEffectPlugin::TextEffectPlugin( QObject *parent, const char *name, const QStringList &/*args*/ ) :  KopetePlugin( parent, name ) 
{
	if( !pluginStatic_ )
		pluginStatic_=this;
	
	m_prefs = new TextEffectPreferences ( "color", this );

	connect( KopeteMessageManagerFactory::factory(),
		SIGNAL( aboutToSend( KopeteMessage & ) ),
		SLOT( slotOutgoingMessage( KopeteMessage & ) ) );

	 last_color=0;
}

TextEffectPlugin::~TextEffectPlugin()
{
	pluginStatic_ = 0L;
}

TextEffectPlugin* TextEffectPlugin::plugin()
{
	return pluginStatic_ ;
}

TextEffectPlugin* TextEffectPlugin::pluginStatic_ = 0L;


void TextEffectPlugin::slotOutgoingMessage( KopeteMessage& msg )
{
	if(msg.direction() != KopeteMessage::Outbound)
		return;

	QStringList colors=m_prefs->colors();

	if(m_prefs->color_char() || m_prefs->color_words() || m_prefs->lamer() || m_prefs->waves() )
	{
		QString original=msg.plainBody();
		QString resultat;

		unsigned int c=0;
		bool wavein=false;

		for(unsigned int f=0;f<original.length();f++)
		{
			char x=original[f].latin1();
			if(f==0 || m_prefs->color_char() || (m_prefs->color_words() && x==' ' ))
			{
				if(f!=0)
					resultat+="</font>";
				resultat+="<font color=\"";
				resultat+=colors[c];
				if(m_prefs->color_random())
					c=rand()%colors.count();
				else
				{
					c++;
					if(c >= colors.count())
						c=0;
				}
				resultat+="\">";
			}
			switch (x)
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
					if(m_prefs->lamer())
					{
						resultat+="4";
						break;
					}
				case 'e':
				case 'E':
					if(m_prefs->lamer())
					{
						resultat+="3";
						break;
					}
				case 'i':
				case 'I':
					if(m_prefs->lamer())
					{
						resultat+="1";
						break;
					}
				case 'l':
				case 'L':
					if(m_prefs->lamer())
					{
						resultat+="|";
						break;
					}
				case 't':
				case 'T':
					if(m_prefs->lamer())
					{
						resultat+="7";
						break;
					}
				case 's':
				case 'S':
					if(m_prefs->lamer())
					{
						resultat+="5";
						break;
					}
				case 'o':
				case 'O':
					if(m_prefs->lamer())
					{
						resultat+="0";
						break;
					}
				default:
					if(m_prefs->waves())
					{
						QString q=QString(QChar(x));
						resultat+= wavein ? q.lower() : q.upper();
						wavein=!wavein;
					}
					else
						resultat+=x;
					break;
			}
		}
		if( m_prefs->color_char() || m_prefs->color_words() )
			resultat+="</font>";
		msg.setBody(resultat,KopeteMessage::RichText);
	}
	
	if(m_prefs->color_lines())
	{
		if(m_prefs->color_random())
		{
			last_color=rand()%colors.count();
		}
		else
		{
			last_color++;
			if(last_color >= colors.count())
				last_color=0;
		}
	
		msg.setFg(QColor (colors[last_color]));
	}
}


#include "texteffectplugin.moc"

