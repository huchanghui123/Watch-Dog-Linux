#include<stdio.h>
#include<sys/io.h>
#include<stdlib.h>
#include<unistd.h>


/* IO Ports */
#define REG		0x2e
#define VAL		0x2f
/* Logical device Numbers LDN */
#define LDNREG		0x07
#define GPIOLND		0x07


static int superio_enter(void)
{
	outb(0x87, REG);
	outb(0x01, REG);
	outb(0x55, REG);
	outb(0x55, REG);
	return 0;
}

static int superio_inb(int reg)
{
	int val;
	outb(reg, REG);
	val = inb(VAL);
	return val;
}

static int superio_inw(int reg, int type)
{
	int val;
	if(type)
	{
		outb(reg++, REG);
	}else{
		outb(reg--, REG);
	}
	val = inb(VAL) << 8;
	outb(reg, REG);
	val |= inb(VAL);
	return val;
}

static void exit_superio()
{
	outb(0x02, REG);
	outb(0x02, VAL);
}

static void feed_dog(int val)
{
	outb(0x73, REG);
	outb(val, VAL);
	if(val>255)
	{
		outb(0x74, REG);
		outb(val>>8, VAL);
	}
	
}

int main()
{
	unsigned int chip_type;
	int ret = iopl(3);
	if(ret == -1)
	{
		printf("iopl error.\n");
		return -1;
	}

	superio_enter();
	chip_type = superio_inw(0x20, 1);
	if(chip_type)
	{
		printf("Chip found, Chip %04x.\n", chip_type);
	}
	exit_superio();

	superio_enter();
	//select logic device
	outb(LDNREG, REG);
	outb(GPIOLND, VAL);
	
	//Q500G6 support
	//8.10.21 SMI# Control Register 2 (Index=F1h, Default=00h)
	//bit 2 This bit is to enable the generation of an SMI# due to WDT’s IRQ (EN_WDT).
	//outb(0xf1, REG);
	//outb(0x44, VAL);

	printf("Watch dog Control Register:%02x\n", superio_inb(0x71));

	printf("Watch dog defalut Configuration Register:%02x\n", superio_inb(0x72));
	//set configuartion
	outb(0x72, REG);
	outb(0x90, VAL);//1001 0000
	int cfg = superio_inb(0x72);
	printf("Watch dog Configuration Register:%02x\n", cfg);


	printf("Watch dog TIme-out default Value Register:%d\n", superio_inw(0x74, 0));
	outb(0x73, REG);
	outb(0x1e, VAL);//12c:300s 1e:30s  3c:60s
	outb(0x74, REG);
	outb(0x00, VAL);
	exit_superio();

	usleep(20*1000000);

	superio_enter();
	printf("Watch dog TIme-out2 Value Register:%d\n", superio_inw(0x74, 0));

	feed_dog(50);//feed dog，reset time-out to 50s
	printf("Watch dog TIme-out3 Value Register:%d\n", superio_inw(0x74, 0));

	//Q300 disable watchdog
	//outb(0x72, REG);
	//outb(0x80, VAL);
	
	//Q500 disable watchdog
	//outb(0xf1, REG);
	//outb(0x40, VAL);

	exit_superio();
	
	iopl(0);
	return 0;
}
