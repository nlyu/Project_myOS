##globally declared, called by idt 
.global excep_DE_helper
.global excep_DF_helper
.global excep_NMI_helper
.global excep_BP_helper
.global excep_OF_helper
.global excep_BRE_helper
.global excep_IO_helper
.global excep_DNA_helper
.global excep_CS_helper
.global excep_ITSS_helper
.global excep_SegNP_helper
.global excep_SF_helper
.global excep_GP_helper
.global excep_PF_helper
.global excep_FP_helper
.global excep_AC_helper
.global excep_MC_helper
.global excep_SFP_helper
.global undef_interrupt_helper
.global keyboard_helper
.global rtc_helper
.global asm_linkage

##
## asm_linkage
## Description: Jump to the correct system call from user space to kernel space
##
## Inputs: %eax systemcall number 
## 		   %ebx, %ecx, %edx arguments
## Outputs: none
## Side Effects: iret to the right system call 
##
asm_linkage:
	#check validity of system call number
	cmpl $1,%eax
	jl INVALID_NUMBER
	cmpl $10,%eax
	jg INVALID_NUMBER
	#save registers
	pushl %ebp
	pushl %edi 
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	
	#push arguments onto stack 
	pushl %edx 
	pushl %ecx
	pushl %ebx
	call *systemcall_jump_table(,%eax,4)
	addl $12,%esp
	
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	iret 
INVALID_NUMBER:
	movl $-1,%eax
	iret
	
systemcall_jump_table:
	.long 0x0,halt,execute,read,write,open,close,getargs,vidmap,set_handler,sigreturn
	
##Exception and interupt hanlder. Since they are different from normal functions we use iret to return to
##the correct place
excep_DE_helper:
call excep_DE
iret

excep_DF_helper:
call excep_DF
iret

excep_NMI_helper:
call excep_NMI
iret

excep_BP_helper:
call excep_BP
iret

excep_OF_helper:
call excep_OF
iret 

excep_BRE_helper:
call excep_BRE
iret 

excep_IO_helper:
call excep_IO
iret

excep_DNA_helper:
call excep_DNA
iret

excep_CS_helper:
call excep_CS
iret 

excep_ITSS_helper:
call excep_ITSS
iret

excep_SegNP_helper:
call excep_SegNP
iret 

excep_SF_helper:
call excep_SF
iret 

excep_GP_helper:
call excep_GP
iret 

excep_PF_helper:
call excep_PF
iret 

excep_FP_helper:
call excep_FP
iret 

excep_AC_helper:
call excep_AC
iret 

excep_MC_helper:
call excep_MC
iret 

excep_SFP_helper:
call excep_SFP
iret 

undef_interrupt_helper:
call undef_interrupt
iret 

keyboard_helper:
call keyboard_interrupt_handler
iret 

rtc_helper:
call rtc_handler
iret 

