namespace Papillon
{

class SwitchBoard::Private
{
public:
}

SwitchBoard::SwitchBoard()
{

}

SwitchBoard::~SwitchBoard()
{

}

SwitchBoard::connect()
{
	if(!d->switchboardConnection){
		d->switchboardConnection = createConnection();
		connect(d->switchboardConnection, SIGNAL(connected()), this, SLOT(switchboardConnected()));
	}
}

}
