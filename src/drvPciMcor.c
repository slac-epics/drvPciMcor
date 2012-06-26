#include <drvSup.h>
#include <drvUioPciGen.h>
#include <devLibPCI.h>
#include <devBusMapped.h>
#include <errlog.h>
#include <epicsExport.h>

#define DRVNAM "drvPciMcor"

#ifndef PCI_VENDOR_SLAC
#define PCI_VENDOR_SLAC 0x1a4a
#endif

#ifndef PCI_DEV_SLAC_MCOR
#define PCI_DEV_SLAC_MCOR 0x1000
#endif

#define MCOR_INST   0
#define MCOR_BAR_NO 0

#define MCOR_PRE    "mcor"
#define DEVNAMCAT(pre,i,b) pre#i"."#b
#define DEVNAM DEVNAMCAT(MCOR_PRE,MCOR_INST,MCOR_BAR_NO)

static long mcor_report(int level)
{
epicsPCIDevice *pci_dev;
volatile void  *base;

	errlogPrintf(DRVNAM": Pseudo driver for PCIe MCOR\n");
	if ( level ) {
		if ( ! (pci_dev = drvUioPciGenFindDev(DEVNAM)) ) {
			errlogPrintf(DRVNAM" error: MCOR device %s not found.\n", DEVNAM);
		} else {
			errlogPrintf("MCOR PCI Device info:\n");
			devPCIShowDevice(level, pci_dev);
		}
		if ( ! (base = devBusMappedFind(DEVNAM)) ) {
			errlogPrintf(DRVNAM" error: MCOR (devBusMapped) %s not found.\n", DEVNAM);
		} else {
			errlogPrintf("  Base Addr. mapped @%p\n", base);
		}
	}
	return 0;
}

static long mcor_init(void)
{
long rval;
	rval = drvUioPciGenRegisterDevice(PCI_VENDOR_SLAC, PCI_DEV_SLAC_MCOR, 0x10000, 0x10000, MCOR_INST, (1<<MCOR_BAR_NO), "mcor");
	if ( rval < 0 ) {
		errlogPrintf(DRVNAM" error: Unable to register MCOR device\n");
	} else if ( 0 == rval ) {
		errlogPrintf(DRVNAM" error: No MCOR devices found\n");
	}
	return rval;
}

static drvet drvPciMcor = {
	number: 2,
	report: mcor_report,
	init:   mcor_init
};

epicsExportAddress( drvet, drvPciMcor );
