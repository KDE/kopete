#include "client.h"
#include "task.h"

int main()
{
	Client c;
	Task rootTask( &c, true );
	
	return 0;
}
