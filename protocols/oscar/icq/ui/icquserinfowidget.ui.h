/*
  icquserinfowidget.ui.h  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Will Stephenson <lists@stevello.free-online.co.uk>

  Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#include <qstringlist.h>

void ICQUserInfoWidget::init()
{
	// set up the combo box contents on the Interests page
	QStringList interests;
	interests.append( i18n("Not Specified") );
	interests.append( i18n("Art") );
	interests.append( i18n("Cars") );
	interests.append( i18n("Celebrity Fans") );
	interests.append( i18n("Collections") );
	interests.append( i18n("Computers") );
	interests.append( i18n("Culture") );
	interests.append( i18n("Fitness") );
	interests.append( i18n("Games") );
	interests.append( i18n("Hobbies") );
	interests.append( i18n("ICQ - Help") );
	interests.append( i18n("Internet") );
	interests.append( i18n("Lifestyle") );
	interests.append( i18n("Movies and TV") );
	interests.append( i18n("Music") );
	interests.append( i18n("Outdoors") );
	interests.append( i18n("Parenting") );
	interests.append( i18n("Pets and Animals") );
	interests.append( i18n("Religion") );
	interests.append( i18n("Science") );
	interests.append( i18n("Skills") );
	interests.append( i18n("Sports") );
	interests.append( i18n("Web Design") );
	interests.append( i18n("Ecology") );
	interests.append( i18n("News and Media") );
	interests.append( i18n("Government") );
	interests.append( i18n("Business") );
	interests.append( i18n("Mystics") );
	interests.append( i18n("Travel") );
	interests.append( i18n("Astronomy") );
	interests.append( i18n("Space") );
	interests.append( i18n("Clothing") );
	interests.append( i18n("Parties") );
	interests.append( i18n("Women") );
	interests.append( i18n("Social science") );
	interests.append( i18n("60's") );
	interests.append( i18n("70's") );
	interests.append( i18n("40's") );
	interests.append( i18n("50's") );
	interests.append( i18n("Finance and Corporate") );
	interests.append( i18n("Entertainment") );
	interests.append( i18n("Consumer Electronics") );
	interests.append( i18n("Retail Stores") );
	interests.append( i18n("Health and Beauty") );
	interests.append( i18n("Media") );
	interests.append( i18n("Household Products") );
	interests.append( i18n("Mail Order Catalog") );
	interests.append( i18n("Business Services") );
	interests.append( i18n("Audio and Visual") );
	interests.append( i18n("Sporting and Athletic") );
	interests.append( i18n("Publishing") );
	interests.append( i18n("Home Automation") );
	
	intrCategoryCombo1->insertStringList( interests );
	intrCategoryCombo2->insertStringList( interests );
	intrCategoryCombo3->insertStringList( interests );
	intrCategoryCombo4->insertStringList( interests );
	
	//set up the Background combo boxes
	QStringList organisations;
	organisations.append( i18n( "Not Specified" ) );
	organisations.append( i18n( "Alumni Org." ) );
	organisations.append( i18n( "Charity Org." ) );
	organisations.append( i18n( "Club/Social Org." ) );
	organisations.append( i18n( "Community Org." ) );
	organisations.append( i18n( "Cultural Org." ) );
	organisations.append( i18n( "Fan Clubs" ) );
	organisations.append( i18n( "Fraternity/Sorority" ) );
	organisations.append( i18n( "Hobbyists Org." ) );
	organisations.append( i18n( "International Org." ) );
	organisations.append( i18n( "Nature and Environment Org." ) );
	organisations.append( i18n( "Professional Org." ) );
	organisations.append( i18n( "Scientific/Technical Org." ) );
	organisations.append( i18n( "Self Improvement Group" ) );
	organisations.append( i18n( "Spiritual/Religious Org." ) );
	organisations.append( i18n( "Sports Org." ) );
	organisations.append( i18n( "Support Org." ) );
	organisations.append( i18n( "Trade and Business Org." ) );
	organisations.append( i18n( "Union" ) );
	organisations.append( i18n( "Voluntary Org." ) );
	organisations.append( i18n( "Other" ) );
	
	bgrdCurrOrgCombo1->insertStringList( organisations );
	bgrdCurrOrgCombo2->insertStringList( organisations );
	bgrdCurrOrgCombo3->insertStringList( organisations );
	
	QStringList affiliations;
	affiliations.append ( i18n( "Not Specified" ));
	affiliations.append ( i18n( "Elementary School" ));
	affiliations.append ( i18n( "High School" ) );
	affiliations.append ( i18n( "College" ) );
	affiliations.append ( i18n( "University" ) );
	affiliations.append ( i18n( "Military" ) );
	affiliations.append ( i18n( "Past Work Place" ) );
	affiliations.append ( i18n( "Past Organization" ) );
	affiliations.append ( i18n( "Other" ) );
	
	bgrdPastOrgCombo1->insertStringList( affiliations );
	bgrdPastOrgCombo2->insertStringList( affiliations );
	bgrdPastOrgCombo3->insertStringList( affiliations );
		
}


void ICQUserInfoWidget::slotCategory1Changed( int i )
{
	if ( i == 0 )
		intrDescText1->setEnabled( false );
	else
		intrDescText1->setEnabled( true );
}


void ICQUserInfoWidget::slotCategory2Changed( int i )
{
	if ( i == 0 )
		intrDescText2->setEnabled( false );
	else
		intrDescText2->setEnabled( true );
}


void ICQUserInfoWidget::slotCategory3Changed( int i )
{
	if ( i == 0 )
		intrDescText3->setEnabled( false );
	else
		intrDescText3->setEnabled( true );
}


void ICQUserInfoWidget::slotCategory4Changed( int i )
{
	if ( i == 0 )
		intrDescText4->setEnabled( false );
	else
		intrDescText4->setEnabled( true );
}


void ICQUserInfoWidget::slotOrganisation1Changed( int i )
{
	if ( i == 0 )
		bgrdCurrOrgText1->setEnabled( false );
	else
		bgrdCurrOrgText1->setEnabled( true );
}


void ICQUserInfoWidget::slotOrganisation2Changed( int i )
{
	if ( i == 0 )
		bgrdCurrOrgText2->setEnabled( false );
	else
		bgrdCurrOrgText2->setEnabled( true );
}


void ICQUserInfoWidget::slotOrganisation3Changed( int i )
{
	if ( i == 0 )
		bgrdCurrOrgText3->setEnabled( false );
	else
		bgrdCurrOrgText3->setEnabled( true );
}


void ICQUserInfoWidget::slotAffiliation1Changed( int i )
{
	if ( i == 0 )
		bgrdPastOrgText1->setEnabled( false );
	else
		bgrdPastOrgText1->setEnabled( true );
}


void ICQUserInfoWidget::slotAffiliation2Changed( int i )
{
	if ( i == 0 )
		bgrdPastOrgText2->setEnabled( false );
	else
		bgrdPastOrgText2->setEnabled( true );
}


void ICQUserInfoWidget::slotAffiliation3Changed( int i )
{
	if ( i == 0 )
		bgrdPastOrgText3->setEnabled( false );
	else
		bgrdPastOrgText3->setEnabled( true );
}
