#include "gwfield.h"
#include <stdio.h>

static Field::FieldList fl;

void buildList();
void buildFakeContactList();
void extractFields( Field::FieldList );

int main()
{
	buildFakeContactList();
	// look for a field in the list
/*	if ( fl.find( NM_A_FA_MESSAGE ) != fl.end() )
		printf( "Found a field, where there was supposed to be one :)\n" );
	else
		printf( "Didn't find a field, where there was supposed to be one :(\n" );
	Field::FieldListIterator it;
	if ( (it = fl.find( Field::NM_A_SZ_OBJECT_ID ) ) != fl.end() )
		printf( "Found a field, where there was NOT supposed to be one :(\n" );
	else
		printf( "Didn't find a field, where there wasn't supposed to be one :)\n" );*/
	//printf( "%i\n", static_cast<Field::MultiField*>(*it) );
	// dump the list
	fl.dump( true );
	
	printf( "\nNow testing find routines.\n");
	// find the field containing the contact list
	Field::MultiField * clf = dynamic_cast< Field::MultiField * >( *(fl.find( Field::NM_A_FA_CONTACT_LIST ) ) );
	if ( clf )
	{
		Field::FieldList cl = clf->fields();
		// look for a folder in the list
		Field::FieldListIterator it = cl.find( Field::NM_A_FA_FOLDER );
		if ( it != cl.end() )
			printf( "Found the first folder :)\n");
		else
			printf( "Didn't find the first folder, where did it go? :(\n");
		
		printf( "Looking for a second folder :)\n");
		it = cl.find( ++it, Field::NM_A_FA_FOLDER );
		if ( it == cl.end() )
			printf( "Didn't find a second folder :)\n" );
		else
			printf( "Found a second folder, now did that get there? :(\n");
	}
	else 
		printf( "Didn't find the contact list, where did it go? :(\n");
				
	//extractFields( fl );
	return 0;
}
// test Field subclasses by creating various FieldLists and recovering the data

void buildList()
{
	// STRUCTURE
	//	fl - top list
	//		sf - faust quote
	//		mf - Multifield - participants, containing
	//			nl - nested list
	//				sf - contact list (empty field array)
	//				sf - message body 
	
	Field::SingleField* sf = new Field::SingleField( NM_A_FA_MESSAGE, 0, NMFIELD_TYPE_UTF8, QString::fromLatin1( "Da steh ich nun, ich armer Tor! Und bin so klug als wie zuvor..." ) );
	fl.append( sf );
	sf = new Field::SingleField( NM_A_SZ_TRANSACTION_ID, 0, NMFIELD_TYPE_UTF8, QString::fromLatin1( "maeuschen" ) );
	fl.append( sf );
	// nested list
	Field::FieldList nl;
	sf = new Field::SingleField( Field::NM_A_SZ_STATUS, 0, NMFIELD_TYPE_UDWORD, 123 );
	nl.append( sf );
	sf = new Field::SingleField( Field::NM_A_SZ_MESSAGE_BODY, 0, NMFIELD_TYPE_UTF8, QString::fromLatin1( "bla bla" ) );
	nl.append( sf );
	Field::MultiField* mf = new Field::MultiField( NM_A_FA_PARTICIPANTS, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY );
	mf->setFields( nl );
	fl.append( mf );
	
/*	Field::SingleField * ext = sf;
	printf( "tag: %s  flags: %i type: %i value: %s\n", ext->tag().data(), ext->flags(), ext->type(), ext->value().toString().ascii() );*/
}

void buildFakeContactList()
{
	using namespace Field;
	
	FieldList contactlist;
	// add a few contacts
	{
		const char* names[] = { "apple", "banana", "cherry", "damson", "elderberry", "framboise" };
		for ( int i = 0; i < 6; i ++ )
		{
			FieldList contact;
			Field::SingleField* sf = new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( i ) );
			contact.append( sf );
			sf = new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, names[i] );
			contact.append( sf );
			MultiField* mf = new MultiField( Field::NM_A_FA_CONTACT, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, contact );
			contactlist.append( mf );
		}
	}
	// add a folder
	{
		FieldList folder;
		Field::SingleField* sf = new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( 1 ) );
		folder.append( sf );
		sf = new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, "buddies" );
		folder.append( sf );
		MultiField* mf = new MultiField( Field::NM_A_FA_FOLDER, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, folder );
		contactlist.append( mf );
	}
	// add some more contacts
	{
		const char* names[] = { "aardvark", "boar", "cat" };
		for ( int i = 0; i < 3; i ++ )
		{
			FieldList contact;
			Field::SingleField* sf = new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( i ) );
			contact.append( sf );
			sf = new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, names[i] );
			contact.append( sf );
			MultiField* mf = new MultiField( Field::NM_A_FA_CONTACT, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, contact );
			contactlist.append( mf );
		}
	}
	
	
	MultiField * cl = new MultiField( Field::NM_A_FA_CONTACT_LIST, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, contactlist );
	fl.append( cl );
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
