//
// C++ Implementation: getdetailstask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "client.h"
#include "request.h"
#include "requestfactory.h"
#include "response.h"

#include "getdetailstask.h"

GetDetailsTask::GetDetailsTask( Task * parent )
 : RequestTask( parent )
{
}


GetDetailsTask::~GetDetailsTask()
{
}

void GetDetailsTask::userDNs( const QStringList & userDNs )
{
	// set up Transfer
	Request * getDetailsRequest = client()->requestFactory()->request( "getdetails" );
	Field::FieldList lst;
	
	for ( QStringList::ConstIterator it = userDNs.begin(); it != userDNs.end(); ++it )
	{
		lst.append( new Field::SingleField( NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, *it ) );
	}
	getDetailsRequest->setFields( lst );
	setTransfer( getDetailsRequest );
}

bool GetDetailsTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	
	Field::FieldList detailsFields = response->fields();
	// parse received details and signal like billio
	Field::MultiField * container = 0;
	Field::FieldListIterator end = detailsFields.end();
	for ( Field::FieldListIterator it = detailsFields.find( NM_A_FA_RESULTS );
		  it != end;
		  it = detailsFields.find( ++it, NM_A_FA_RESULTS ) )
	{
		container = static_cast<Field::MultiField *>( *it );
		ContactDetails cd = extractUserDetails( container );
		emit gotContactUserDetails( cd );
	}
	
	return true;
}

ContactDetails GetDetailsTask::extractUserDetails(Field::MultiField * details )
{
	ContactDetails cd;
	cd.status = GroupWise::Invalid;
	Field::FieldList fields = details->fields();
	// TODO: not sure what this means, ask Mike
	Field::SingleField * sf;
	if ( ( sf = fields.findSingleField ( NM_A_SZ_AUTH_ATTRIBUTE ) ) )
		cd.authAttribute = sf->value().toString();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_DN ) ) )
		cd.dn =sf->value().toString().lower(); // HACK: lowercased DN
	if ( ( sf = fields.findSingleField ( "CN" ) ) )
		cd.cn = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Given Name" ) ) )
		cd.givenName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Surname" ) ) )
		cd.surname = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Full Name" ) ) )
		cd.fullName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_STATUS ) ) )
		cd.status = sf->value().toInt();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_MESSAGE_BODY ) ) )
		cd.awayMessage = sf->value().toString();
	return cd;
}
#include "getdetailstask.moc"
