#include "gwfield.h"
#include <stdio.h>

static Field::FieldList fl;

void buildList();
void extractFields( Field::FieldList );

int main()
{
	buildList();
	// look for a field in the list
	if ( fl.find( NM_A_FA_MESSAGE ) != fl.end() )
		printf( "Found a field, where there was supposed to be one :)" );
	else
		printf( "Didn't find a field, where there was supposed to be one :(" );
		
	if ( fl.find( NM_A_SZ_OBJECT_ID ) != fl.end() )
		printf( "Found a field, where there was NOT supposed to be one :(" );
	else
		printf( "Didn't find a field, where there wasn't supposed to be one :)" );
				
	extractFields( fl );
	return 0;
}
// test Field subclasses by creating various FieldLists and recovering the data

void buildList()
{
	Field::SingleField* sf = new Field::SingleField( NM_A_FA_MESSAGE, 0, NMFIELD_TYPE_UTF8, QString::fromLatin1( "Da steh ich nun, ich armer Tor! Und bin so klug als wie zuvor..." ) );
	fl.append( sf );
	sf = new Field::SingleField( NM_A_SZ_TRANSACTION_ID, 0, NMFIELD_TYPE_UTF8, QString::fromLatin1( "maeuschen" ) );
	fl.append( sf );
	// nested list
	Field::FieldList nl;
	sf = new Field::SingleField( NM_A_FA_CONTACT_LIST, 0, NMFIELD_TYPE_UDWORD, 123 );
	nl.append( sf );
	sf = new Field::SingleField( NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, QString::fromLatin1( "bla bla" ) );
	nl.append( sf );
	Field::MultiField* mf = new Field::MultiField( NM_A_FA_PARTICIPANTS, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY );
	mf->setFields( nl );
	fl.append( mf );
	
/*	Field::SingleField * ext = sf;
	printf( "tag: %s  flags: %i type: %i value: %s\n", ext->tag().data(), ext->flags(), ext->type(), ext->value().toString().ascii() );*/
}

void extractFields( Field::FieldList l )
{
	Field::FieldListIterator it;
	printf ("iterating over %i fields\n", l.count() );
	for ( it = l.begin(); it != l.end() ; ++it )
	{
		printf ("field\n");
		Field::SingleField * ext = dynamic_cast<Field::SingleField *>( *it );
		if ( ext )
			printf( "tag: %s  flags: %i type: %i value: %s\n", ext->tag().data(), ext->flags(), ext->type(), ext->value().toString().ascii() );
		else
		{
			Field::MultiField* mf = dynamic_cast<Field::MultiField *>( *it );
			if ( mf )
			{
			 	printf( "found a multi value field\n" );
				extractFields( mf->fields() );
			}
		}
	}

	printf ("done\n");
}
