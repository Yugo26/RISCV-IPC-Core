#include "os.h"


extern timerCB_t     *TimerList;
uint32_t _tick = 0;

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(int interval)
{
	/* each CPU has a separate source of timer interrupts. */
	int id = r_mhartid();
	
	*(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;
}

void timer_init()
{
	/*
	 * On reset, mtime is cleared to zero, but the mtimecmp registers 
	 * are not reset. So we have to init the mtimecmp manually.
	 */
	timer_load(SYSTEM_TICK);

	/* enable machine-mode timer interrupts. */
	w_mie(r_mie() | MIE_MTIE);

	/* enable machine-mode global interrupts. */
	//w_mstatus(r_mstatus() | MSTATUS_MIE);
}

static void tick_dec() {
	taskCB_t *ptcb=getCurrentTask();
	if (ptcb!=NULL) {
		ptcb->remain_ticks--;
		if (ptcb->remain_ticks<=0) {
			ptcb->remain_ticks = ptcb->init_ticks;
			schedule();
		}
	} else {
		schedule();
	}
}

/* 08 */
static void timer_check()
{
	
	timerCB_t *pTimer = TimerList;
	
	if (!list_isempty((list_t*)pTimer)) {
		pTimer = (timerCB_t*)TimerList->node.next;
        pTimer->timerCnt--;
		if (pTimer->timerCnt<=0)		
			timerDispose();
	}
	
}

static void showTick() 
{
	#ifdef DEBUGMSG
	char erase[32]={[0 ... 30]=8,0};
	if (_tick%10==0) {
		lockfree_printf("tick: %d", _tick);
		lockfree_printf("%s",erase);
	}
	#endif
}
void timer_handler() 
{
	_tick++;
	showTick();
	timer_load(SYSTEM_TICK);
	tick_dec();
	timer_check(); //08
}

uint64_t getTicks()
{
	return _tick;
}