#include <iocsh.h>
#include <epicsThread.h>
#include <epicsExit.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_CEXP
#include <cexp.h>
#endif

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define PREF ""

static void dumpchars(unsigned char **pstr, unsigned char *end)
{
    printf("  ");
    while ( *pstr < end ) {
        fputc(isprint(**pstr) ? **pstr : '.', stdout);
        (*pstr)++;
    }
}

static void dumpval(uintptr_t address, int size)
{
    switch(abs(size)) {
        case 4: printf(PREF"%08x", *(unsigned*)address);        break;
        case 2: printf(PREF"%04x", *(unsigned short*)address);  break;
        case 1: printf(PREF"%02x", *(unsigned char *)address);  break;
        default:
        break;
    }
}


extern "C" int
md(uintptr_t address, int count, int size)
{
int i = 0;
unsigned char *oadd = (unsigned char*)address;
	switch (size) {
		default:	size=4;
		case 1: case 2: case 4:
		break;
	}
	while ( i < count) {
		if ( i%16 == 0 ) {
			if (i) {
				dumpchars(&oadd, (unsigned char*)address);
			}
			printf("\n0x%08" PRIxPTR ":", address);
		}
		printf("  ");
		dumpval(address,size);
		i += size;
		address += size;
	}

	if (count > 0) {
		printf("%*s",(((i+15)/16)*16 - i)/size * (2 + (int)strlen(PREF) + 2*size),"");
		dumpchars(&oadd,(unsigned char*)address);
	}
	fputc('\n',stdout);
	return 0;
}


int
main(int argc, char **argv)
{
#ifdef HAVE_CEXP
	cexp_main(argc, argv);
#else
	if ( argc >= 2 ) {
		iocsh(argv[1]);
		epicsThreadSleep(.2);
	}
	iocsh(NULL);
#endif
	epicsExit(0);
}
