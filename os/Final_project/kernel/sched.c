#include "os.h"

/* defined in entry.S */
extern void switch_to(ctx_t *next);
extern taskCB_t TCBRdy[];
extern taskCB_t * TCBRunning; 
extern ctx_t kernel_context;

void sched_init()
{
	w_mscratch(0);
	/* enable machine-mode software interrupts. */
	w_mie(r_mie() | MIE_MSIE);
}

void schedule()
{
	taskCB_t *nextTask;
	ctx_t *next;
	taskCB_t *readyQ=NULL;
	reg_t lock_status;

	//get highest priority queue
	for(int i=0;i<PRIO_LEVEL;i++) {
		if (!list_isempty((list_t*)&TCBRdy[i])) {
			readyQ = &TCBRdy[i];
			break;
		}
	}
	if (readyQ==NULL) return;

	//get next task
	nextTask= (taskCB_t*)readyQ->node.next;
	next = &nextTask->ctx;
	lock_status = baseLock();
	list_remove((list_t*)nextTask);


	//current task into ready queue
	if (TCBRunning != NULL){//kernel
		taskCB_t *currentTask = TCBRunning;
        
		//	權限比 TCBRunning低 且 不是在 waiting 就不用切換
		//	如果currentTask 要 waiting，就必須要切換
		
	    if (currentTask->state == TASK_RUNNING) {  
			if (currentTask->priority < nextTask->priority) {
				//no schedule so insert nextTask to Queue
				list_insert_after((list_t*)&TCBRdy[nextTask->priority],(list_t*)nextTask);
				baseUnLock(lock_status);
				return;
			}
			currentTask->state = TASK_READY;
			list_insert_before((list_t*)&TCBRdy[currentTask->priority],(list_t*)currentTask);
		}
	} 
	TCBRunning = nextTask;
	nextTask->state = TASK_RUNNING;
	baseUnLock(lock_status);
	/*
	if (r_mscratch()==0) {
		enableINT();
		switch_first(next);
	}
	else
	*/
	switch_to(next);
}



