//
// C++ Implementation: createprivacyitemtask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "privacyitemtask.h"

PrivacyItemTask::PrivacyItemTask( Task* parent) : RequestTask( parent )
{
}

PrivacyItemTask::~PrivacyItemTask()
{
}

QString PrivacyItemTask::dn() const
{
	return m_dn;
}

bool PrivacyItemTask::defaultDeny() const
{
	return m_default;
}

void PrivacyItemTask::allow( const QString & dn )
{
	m_dn = dn;
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_BLOCKING_ALLOW_ITEM, NMFIELD_METHOD_ADD, 0, NMFIELD_TYPE_UTF8, dn ) );
	createTransfer( "createblock", lst );
}

void PrivacyItemTask::deny( const QString & dn )
{
	m_dn = dn;
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_BLOCKING_DENY_ITEM, NMFIELD_METHOD_ADD, 0, NMFIELD_TYPE_UTF8, dn ) );
	createTransfer( "createblock", lst );
}

void PrivacyItemTask::removeAllow( const QString & dn )
{
	m_dn = dn;
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_BLOCKING_ALLOW_LIST, NMFIELD_METHOD_DELETE, 0, NMFIELD_TYPE_UTF8, dn ) );
	createTransfer( "updateblocks", lst );

}

void PrivacyItemTask::removeDeny( const QString & dn )
{
	m_dn = dn;
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_BLOCKING_DENY_LIST, NMFIELD_METHOD_DELETE, 0, NMFIELD_TYPE_UTF8, dn ) );
	createTransfer( "updateblocks", lst );
}
	
void PrivacyItemTask::defaultPolicy( bool defaultDeny )
{
	m_default = defaultDeny;
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_BLOCKING, NMFIELD_METHOD_UPDATE, 0, NMFIELD_TYPE_UTF8, ( defaultDeny ? "1" :"0" ) ) );
	createTransfer( "updateblocks", lst );
}

#include "privacyitemtask.moc"
