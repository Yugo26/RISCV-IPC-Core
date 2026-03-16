#include "os.h"

extern timerCB_t *TimerList;
extern SemCB_t SEMTbl[];
extern taskCB_t TCBTbl[];
extern taskCB_t TCBRdy[];

void showTimerList() {
    timerCB_t *ptimer;
    list_t *pList = (list_t *)TimerList->node.next;
    while(pList != (list_t*)TimerList) {
        ptimer = (timerCB_t *)pList;
        kprintf("Timer ID=%d\n", ptimer->timerID);
        kprintf("TimerCnt =%d\n", ptimer->timerCnt);
        kprintf("Timer status=%d\n", ptimer->timerState);
        kprintf("*******************\n");
        pList = pList->next;
    }
}

void showRdyQ() {
    list_t *pList;
    taskCB_t *readyQ;
    kprintf("*******Rdy Q******\n");
	//get highest priority queue
	for(int i=0;i<PRIO_LEVEL;i++) {
		if (!list_isempty((list_t*)&TCBRdy[i])) {
			readyQ = &TCBRdy[i];
			pList = TCBRdy[i].node.next;
            while(pList != (list_t*)readyQ) {
                kprintf("RdyQ = %d, %s, status=%d\n",i,
                ((taskCB_t*)pList)->name,
                ((taskCB_t*)pList)->state);
                pList = pList->next;
            }
        }       
    }
}

void showSemQ(uint16_t semID) {
    SemCB_t *psemcb = &SEMTbl[semID];
    list_t *pList = (list_t *)psemcb->node.next;
    kprintf("*******SEM Q******\n");
    while(pList != (list_t*)psemcb) {
        kprintf("Q=%d, %s\n",((taskCB_t*)pList)->priority, ((taskCB_t*)pList)->name);
        pList = pList->next;
    }
}

void showMstatus() {
    reg_t status = r_mstatus();
    kprintf("lock mstatus = %x\n",status);
}

void showAllQ(int semID) {
    showTimerList();
    showRdyQ();
    showSemQ(semID);
    showMstatus();
    panic("stop here\n");
}
