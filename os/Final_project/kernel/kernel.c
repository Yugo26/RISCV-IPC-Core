#include "os.h"

/*
 * Following functions SHOULD be called ONLY ONE time here,
 * so just declared here ONCE and NOT included in file os.h.
 */
extern void uart_init(void);
extern void page_init(void);


void start_kernel(void)
{
	//disableINT();	
	page_init();

	InitTCBList();
	readyQ_init();
	deviceList_init();
	plic_init();
	trap_init();
	drivers_init();

	timer_init();
	softTimer_init(); //08
	idleTask_init(); //08
	loadTasks();
	sched_init();	
	syscall_init();
	
	INFO("OS is scheduling....\n");
	schedule(); //idle task
	INFO("Would not go here!\n");
	while (1) {}; // stop here!	
}

