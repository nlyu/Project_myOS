/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask=0xff; /* IRQs 0-7 */
uint8_t slave_mask=0xff; /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void
i8259_init(void)
{
	//send four ICWs to master
	outb(ICW1,MASTER_8259_PORT);
	outb(ICW2_MASTER,MASTER_8259_PORT+1);
	outb(ICW3_MASTER,MASTER_8259_PORT+1);
	outb(ICW4,MASTER_8259_PORT+1);
	
	//send four ICWs to slave
	outb(ICW1,SLAVE_8259_PORT);
	outb(ICW2_SLAVE,SLAVE_8259_PORT+1);
	outb(ICW3_SLAVE,SLAVE_8259_PORT+1);
	outb(ICW4,SLAVE_8259_PORT+1);
}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
	if((irq_num<0)||(irq_num>15))
		return;
	switch(irq_num)
	{
		case 0:
		//11111110
		master_mask=0xfe & master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 1:
		//11111101
		master_mask=0xfd & master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 2:
		//11111011
		master_mask=0xfb & master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 3:
		//11110111
		master_mask=0xf7 & master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 4:
		//11101111
		master_mask=0xef & master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 5:
		//11011111
		master_mask=0xdf & master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 6:
		//10111111
		master_mask=0xbf & master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 7:
		//01111111
		master_mask=0x7f & master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		
		//below are used to unmask the slave
		case 8:
		//11111110
		slave_mask=0xfe & slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 9:
		//11111101
		slave_mask=0xfd & slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 10:
		//11111011
		slave_mask=0xfb & slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 11:
		//11110111
		slave_mask=0xf7 & slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 12:
		//11101111
		slave_mask=0xef & slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 13:
		//11011111
		slave_mask=0xdf & slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 14:
		//10111111
		slave_mask=0xbf & slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 15:
		//01111111
		slave_mask=0x7f & slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
	}
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
	if((irq_num<0)||(irq_num>15))
		return;
	switch(irq_num)
	{
		case 0:
		//00000001
		master_mask=0x01 | master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 1:
		//00000010
		master_mask=0x02 | master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 2:
		//00000100
		master_mask=0x04 | master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 3:
		//00001000
		master_mask=0x08 | master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 4:
		//00010000
		master_mask=0x10 | master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 5:
		//00100000
		master_mask=0x20 | master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 6:
		//01000000
		master_mask=0x40 | master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		case 7:
		//10000000
		master_mask=0x80 | master_mask;
		outb(master_mask,MASTER_8259_PORT+1);
		return;
		
		//below are used to mask the slave
		case 8:
		//00000001
		slave_mask=0x01 | slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 9:
		//00000010
		slave_mask=0x02 | slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 10:
		//00000100
		slave_mask=0x04 | slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 11:
		//00001000
		slave_mask=0x08 | slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 12:
		//00010000
		slave_mask=0x10 | slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 13:
		//00100000
		slave_mask=0x20 | slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 14:
		//01000000
		slave_mask=0x40 | slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
		case 15:
		//10000000
		slave_mask=0x80 | slave_mask;
		outb(slave_mask,SLAVE_8259_PORT+1);
		return;
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
	if((irq_num<0)||(irq_num>15))
		return;
	if((irq_num>=0)&&(irq_num<8))
		outb( EOI|irq_num, MASTER_8259_PORT );
	else
	{
		outb( EOI|(irq_num-8), SLAVE_8259_PORT);
		outb( EOI|SLAVE_IRQ,MASTER_8259_PORT );
	}
}

