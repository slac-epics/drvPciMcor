#ifndef DRV_PCI_MCOR_H
#define DRV_PCI_MCOR_H

#ifdef cplusplus_
extern "C" {
#endif

/* Valid channel numbers are 0..15 */
#define MCOR_NUM_CHANNELS 16

extern int
drvPciMcorMgntSetDAC(void *card_p, int dac_channel, epicsInt32 dac_valu);


#ifdef cplusplus_
}
#endif

#endif
