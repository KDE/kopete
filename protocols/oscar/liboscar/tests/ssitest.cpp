//Licensed under the GNU General Public License

#include "ssitest.h"

#include <qstring.h>

SSITest::SSITest(int argc, char ** argv) : QApplication( argc, argv )
{
	m_manager = new SSIManager(this);

	testIt();

}

SSITest::~SSITest()
{
	delete m_manager;
}

void SSITest::testIt()
{
	QPtrList<TLV> tlvs;
	
	//add three groups
	SSI *ssi = new SSI( "FirstGroup", 1, 1, ROSTER_GROUP, tlvs); 
	m_manager->newGroup(ssi);
	
	ssi = new SSI( "SecondGroup", 2, 2, ROSTER_GROUP, tlvs); 
	m_manager->newGroup(ssi);

	ssi = new SSI( "ThirdGroup", 3, 3, ROSTER_GROUP, tlvs); 
	m_manager->newGroup(ssi);

	//add six contacts
	ssi = new SSI( "FirstContact", 1, 4, ROSTER_CONTACT, tlvs);
    m_manager->newContact(ssi);

    ssi = new SSI( "SecondContact", 1, 5, ROSTER_CONTACT, tlvs);
    m_manager->newContact(ssi);

    ssi = new SSI( "ThirdContact", 1, 6, ROSTER_CONTACT, tlvs);
    m_manager->newContact(ssi);
	
	ssi = new SSI( "FourthContact", 2, 7, ROSTER_CONTACT, tlvs);
    m_manager->newContact(ssi);

    ssi = new SSI( "FifthContact", 2, 8, ROSTER_CONTACT, tlvs);
    m_manager->newContact(ssi);

    ssi = new SSI( "SixthContact", 3, 9, ROSTER_CONTACT, tlvs);
    m_manager->newContact(ssi);

	//try to find a group by name
	ssi = m_manager->findGroup("SecondGroup");
	if ( ssi )
		qDebug( QString("Found group SecondGroup with gid=%1").arg( ssi->gid() ).latin1());
	else
		qDebug( "Oops, group SecondGroup not found" );

	//try to find a group by gid
	ssi = m_manager->findGroup( 3 );
	if ( ssi )
		qDebug( QString("Found group 3 with name=%1").arg( ssi->name() ).latin1() );
	else
		qDebug( "Oops, group 3 not found" );

	//try to find a contact by name
	ssi = m_manager->findContact("ThirdContact");
	if ( ssi )
		qDebug( QString( "Found contact ThirdContact with gid=%1" ).arg( ssi->gid() ).latin1() );
	else
		qDebug( "Oops, contact ThirdContact not found" );

	//try to find a contact using the name and the group name
	ssi = m_manager->findContact("FourthContact","SecondGroup");
	if ( ssi )
		qDebug( QString("Found contact FourthContact with bid=%1").arg( ssi->bid() ).latin1() );
	else
		qDebug( "Oops, contact FourthContact not found" );

	
	//try to find an invalid group
	ssi = m_manager->findGroup("InvalidGroup");
	if ( !ssi )
		qDebug( "Good! It has detected the group is invalid :)" );

	//contacts from a group
	QValueList<SSI*> list = m_manager->contactsFromGroup("FirstGroup");
	QValueList<SSI*>::iterator it;
	qDebug( "Contacts from group FirtsGroup:" );
	for ( it = list.begin(); it != list.end(); ++it)
		qDebug( QString("      name=%1").arg( (*it)->name() ).latin1() );

	//the group list
	QValueList<SSI*> list2 = m_manager->groupList();
	qDebug( "Group list:" );
	for ( it = list2.begin(); it != list2.end(); ++it)
		qDebug( QString("      name=%1").arg( (*it)->name() ).latin1() );
	
	//remove a group - this shouldn't report any debug line
	m_manager->removeGroup( "SecondGroup" );

}

int main(int argc, char ** argv)
{
	SSITest a( argc, argv );
	a.exec();
}

#include "ssitest.moc"
