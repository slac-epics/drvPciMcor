#include <drvSup.h>
#include <drvUioPciGen.h>
#include <devLibPCI.h>
#include <devBusMapped.h>
#include <errlog.h>
#include <epicsExport.h>
#include <drvPciMcor.h>
#include <cantProceed.h>
#include <epicsMMIO.h>
#include <iocsh.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <epicsThread.h>
#include <string.h>
#include <unistd.h>

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
#define DEVNAMCATCAT(pre,i,b) DEVNAMCAT(pre,i,b)
#define DEVNAM DEVNAMCATCAT(MCOR_PRE,MCOR_INST,MCOR_BAR_NO)

#define MCOR_DAC_OFF  0
#define MCOR_CH_SHFT  6
#define MCOR_CH_BASE(ch) ((ch)<<MCOR_CH_SHFT)

static volatile void *theBase = 0;

static long mcor_report(int level)
{
const epicsPCIDevice *pci_dev;
DevBusMappedDev dev;

	errlogPrintf(DRVNAM": Pseudo driver for PCIe MCOR\n");
	if ( level ) {
		if ( ! (pci_dev = drvUioPciGenFindDev(DEVNAM)) ) {
			errlogPrintf(DRVNAM" error: MCOR device %s not found.\n", DEVNAM);
		} else {
			errlogPrintf("MCOR PCI Device info:\n");
			devPCIShowDevice(level, pci_dev);
		}
		if ( ! (dev = devBusMappedFind(DEVNAM)) ) {
			errlogPrintf(DRVNAM" error: MCOR (devBusMapped) %s not found.\n", DEVNAM);
		} else {
			errlogPrintf("  Base Addr. mapped @%p\n", dev->baseAddr);
		}
	}
	return 0;
}

extern int
drvPciMcorMgntSetDAC(void *card_p, int dac_channel, epicsInt32 dac_val)
{
	if (   card_p
	    || dac_channel < 0
	    || dac_channel >= MCOR_NUM_CHANNELS
	    || 0 == theBase )
		return -1;

	le_iowrite32(theBase + MCOR_CH_BASE(dac_channel) + MCOR_DAC_OFF , dac_val);
	return 0;
}

static long mcor_init(void)
{
long            rval;
DevBusMappedDev dev;
	rval = drvUioPciGenRegisterDevice(PCI_VENDOR_SLAC, PCI_DEV_SLAC_MCOR, 0x10000, 0x10000, MCOR_INST, (1<<MCOR_BAR_NO), MCOR_PRE);
	if ( rval < 0 ) {
		errlogPrintf(DRVNAM" error: Unable to register MCOR device\n");
	} else if ( 0 == rval ) {
		errlogPrintf(DRVNAM" error: No MCOR devices found\n");
	} else {
		if ( ! (dev = devBusMappedFind( DEVNAM )) ) {
			rval = -1;
			cantProceed(DRVNAM" INTERNAL ERROR: MCOR device registered but not found!\n");
			/* never get here */
		} else {
			theBase = dev->baseAddr;
		}
	}
	return rval;
}

static drvet drvPciMcor = {
	number: 2,
	report: mcor_report,
	init:   mcor_init
};

epicsExportAddress( drvet, drvPciMcor );


#define MCOR_FILENAME "/dev/emcor"
#define TIMEOUT_MSEC 200

static struct IrqFdStruct {
	int fd;
	void *handler;
} irqFdStruct;

static void irq_handler(int status) {
	errlogPrintf("Status: 0x%08X\n", status);

	// enable back interrupts
	int wVal = 0xFFFFFFFF;
	int retval = write(irqFdStruct.fd, &wVal, 4);
	if (retval != 4) {
		errlogPrintf("Write failed, return value: %d\n", retval);
	}
}

static int mcor_irq_thread(void *eiFdStruct) {
	struct IrqFdStruct *eiFdS = eiFdStruct;
	int val, retP, retR;
	struct pollfd fds[1];
	fds[0].fd = eiFdS->fd;
	fds[0].events = POLLIN;
	void (*irqHandler)(int) = eiFdS->handler;

	while (1) {
		retP = poll(fds, 1, TIMEOUT_MSEC);
		if (retP > 0) {
			retR = read(eiFdS->fd, &val, 4);
			if (retR != 4) {
				errlogPrintf("Read failed, return value: %d\n", retR);
				return -1;
			}
			if (irqHandler) {
				irqHandler(val);
			}
		} else if (retP < 0) {
			errlogPrintf("Poll failed: %s\n", strerror(errno));
			return -1;
		}
	}
	return 0;
}

static void drvPciMcorIRQInit(void) {
	int fd = open(MCOR_FILENAME, O_RDWR);
	if (fd < 0) {
		errlogPrintf("Failed to open FD: %s\n", strerror(errno));
		return;
	}

	irqFdStruct.fd = fd;
	irqFdStruct.handler = irq_handler;

	epicsThreadMustCreate("mcor_irq_thread", epicsThreadPriorityHigh + 5,
			epicsThreadGetStackSize(epicsThreadStackMedium),
			(EPICSTHREADFUNC) mcor_irq_thread, &irqFdStruct);

	// enable interrupts
	int wVal = 0xFFFFFFFF;
	int retval = write(fd, &wVal, 4);
	if (retval != 4) {
		errlogPrintf("Write failed, return value: %d\n", retval);
	}
}

static const iocshFuncDef drvPciMcorInitFuncDef = { "drvPciMcorIRQInit", 0, NULL };
static void drvPciMcorInitCallFunc(const iocshArgBuf *args) {
	drvPciMcorIRQInit();
}
static void drvPciMcorRegister(void) {
	iocshRegister(&drvPciMcorInitFuncDef, drvPciMcorInitCallFunc);
}
epicsExportRegistrar(drvPciMcorRegister);
