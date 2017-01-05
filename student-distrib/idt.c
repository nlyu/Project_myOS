#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "keyboard.h"
#include "handler.h"

//Exception: Divide error
void excep_DE(){
	cli();
	printf("Divide error exception!\n");
	while(1);
	sti();
}
//Exception: Debug
void excep_DB(){
	cli();
	printf("Debug exception!\n");
	while(1);
	sti();
}
//Exception: NMI
void excep_NMI(){
	cli();
	printf("Non-maskable interrupt exception!\n");
	while(1);
	sti();
}
//Exception: Breakpoint
void excep_BP(){
	cli();
	printf("Break-point exception!\n");
	while(1);
	sti();
}
//Exception: overflow
void excep_OF(){
	cli();
	printf("Overflow exception!\n");
	while(1);
	sti();
}
//Exception: bound range exceed
void excep_BRE(){
	cli();
	printf("Bound range exceeded exception!\n");
	while(1);
	sti();
}
//Exception: Invalid op
void excep_IO(){
	cli();
	printf("Invalid opcode exception!\n");
	while(1);
	sti();
}
//Exception: Device not available
void excep_DNA(){
	cli();
	printf("Device not available exception!\n");
	while(1);
	sti();
}
//Exception: Double fault
void excep_DF(){
	cli();
	printf("Double fault exception!\n");
	while(1);
	sti();
}
//Exception: coprocessor segment
void excep_CS(){
	cli();
	printf("Coprocessor segment exception!\n");
	while(1);
	sti();
}
//Exception: Invalid TSS
void excep_ITSS(){
	cli();
	printf("Invalid TSS exception!\n");
	while(1);
	sti();
}
//Exception: Segment not present
void excep_SegNP(){
	cli();
	printf("Segment not present exception!\n");
	while(1);
	sti();
}
//Exception: stack fault
void excep_SF(){
	cli();
	printf("Stack fault exception!\n");
	while(1);
	sti();
}
//Exception: general protection
void excep_GP(){
	cli();
	uint32_t error_code;
	uint32_t error_code2;
	asm volatile ("movl 44(%%esp),%%ecx;"
				  "movl %%ecx,%0;"
				 :"=g"(error_code));
		asm volatile ("movl 40(%%esp),%%ecx;"
				  "movl %%ecx,%0;"
				 :"=g"(error_code2));
		
	printf("error_code:%x\n",error_code);
	printf("error_code:%x",error_code2);
	printf("General protection exception!\n");
	while(1);
	sti();
}
//Exception: page fault
void excep_PF(){
	cli();
	printf("Page falut exception!\n");
	while(1);
	sti();
}
//Exception: floating point
void excep_FP(){
	cli();
	printf("FLoating point exception!\n");
	while(1);
	sti();
}
//Exception: alignment check
void excep_AC(){
	cli();
	printf("Alignment check exception!\n");
	while(1);
	sti();
}
//Exception: machine check
void excep_MC(){
	cli();
	printf("Machine check exception!\n");
	while(1);
	sti();
}
//Exception: SIMD floating point
void excep_SFP(){
	cli();
	printf("SIMD floating-point exception!\n");
	while(1);
	sti();
}
//Exception: undefined interrupt
void undef_interrupt(){
	cli();
	printf("Undefined interruption!\n");
	while(1);
	sti();
}



/*
	IDT_init:
		This function intitalize the IDT table by putting exceptions into places dictated
		by Intel manual, and setting correct idt_desc_t bit
	INPUT: none
	OUTPUT: none
	RESULT: correct content is put into idt table
	
*/

void IDT_init(){


	lidt(idt_desc_ptr);			//load IDT base addr and size into IDTR
	int c = 0;

	while( c < NUM_VEC) {		//NUM_VEC = 256
		

		idt[c].reserved0 = 0x0;
		idt[c].reserved1 = 0x1;
		idt[c].reserved2 = 0x1;
		idt[c].reserved3 = 0x1;
		idt[c].reserved4 = 0x0;
		idt[c].seg_selector = KERNEL_CS;  // KERNEL_CS = 0x0010
		idt[c].size = 0x1;
		idt[c].present = 0x1;
		idt[c].dpl = 0x0;
		if (c >= system_defined_idt_entries_number) { 	//system_defined_idt_entries_number = 32,
														// if c is bigger than that, that means we are defining user defined idt entries
			idt[c].reserved3 = 0x0;
			SET_IDT_ENTRY(idt[c], undef_interrupt_helper);
		}
		if(c == SYSTEM_CALL_NUMBER){
			idt[c].dpl = 0x3;
		}
		c = c + 1;
	}


	SET_IDT_ENTRY(idt[0], excep_DE_helper);
	SET_IDT_ENTRY(idt[1], excep_DF_helper);
	SET_IDT_ENTRY(idt[2], excep_NMI_helper);
	SET_IDT_ENTRY(idt[3], excep_BP_helper);
	SET_IDT_ENTRY(idt[4], excep_OF_helper);
	SET_IDT_ENTRY(idt[5], excep_BRE_helper);
	SET_IDT_ENTRY(idt[6], excep_IO_helper);
	SET_IDT_ENTRY(idt[7], excep_DNA_helper);
	SET_IDT_ENTRY(idt[8], excep_DF_helper);
	SET_IDT_ENTRY(idt[9], excep_CS_helper);
	SET_IDT_ENTRY(idt[10], excep_ITSS_helper);
	SET_IDT_ENTRY(idt[11], excep_SegNP_helper);
	SET_IDT_ENTRY(idt[12], excep_SF_helper);
	SET_IDT_ENTRY(idt[13], excep_GP_helper);
	SET_IDT_ENTRY(idt[14], excep_PF_helper);
	
	// idt[15] entry is reserved
	
	SET_IDT_ENTRY(idt[16], excep_FP_helper);
	SET_IDT_ENTRY(idt[17], excep_AC_helper);
	SET_IDT_ENTRY(idt[18], excep_MC_helper);
	SET_IDT_ENTRY(idt[19], excep_SFP_helper);
	SET_IDT_ENTRY(idt[33], keyboard_helper);
	SET_IDT_ENTRY(idt[40], rtc_helper);
	
	SET_IDT_ENTRY(idt[SYSTEM_CALL_NUMBER], asm_linkage);


}
