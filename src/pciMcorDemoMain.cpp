#include <iocsh.h>
#include <epicsThread.h>
#include <epicsExit.h>

int
main(int argc, char **argv)
{
	if ( argc >= 2 ) {
		iocsh(argv[1]);
		epicsThreadSleep(.2);
	}
	iocsh(NULL);
	epicsExit(0);
}
