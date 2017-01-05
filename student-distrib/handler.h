#ifndef HANDLER_H
#define HANDLER_H

extern void keyboard_helper();
extern void rtc_helper();
extern void excep_DE_helper();
extern void excep_DF_helper();
extern void excep_NMI_helper();
extern void excep_BP_helper();
extern void excep_OF_helper();
extern void excep_BRE_helper();
extern void excep_IO_helper();
extern void excep_DNA_helper();
extern void excep_CS_helper();
extern void excep_ITSS_helper();
extern void excep_SegNP_helper();
extern void excep_SF_helper();
extern void excep_GP_helper();
extern void excep_PF_helper();
extern void excep_FP_helper();
extern void excep_AC_helper();
extern void excep_MC_helper();
extern void excep_SFP_helper();
extern void undef_interrupt_helper();
extern void asm_linkage();


#endif
