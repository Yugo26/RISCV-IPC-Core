//for IPC queue, sem, mbox, messageQ, event etc.
#include "os.h"


void TaskToWait(list_t *plist, uint8_t sortType, taskCB_t *ptcb)
{
    task_suspend(ptcb);

    switch (sortType)
    {
    case FIFO:
        list_insert_before(plist, (list_t*)ptcb);
        break;

    case PRIO:
        {//header is empty data
            list_t *pCntNode = plist->next;
            while(pCntNode != (list_t *)plist) {//double link is empty check
                if ( ((taskCB_t*)pCntNode)->priority > ptcb->priority) 
                    break;
                pCntNode = pCntNode->next;
            }
            list_insert_before(pCntNode, (list_t *)ptcb);
        }
        break;

    default:
        break;
    }
}

void WaitTaskToRdy(list_t *plist)
{
   
    if (list_isempty(plist))
        return;
    
    taskCB_t *ptcb = (taskCB_t *)plist->next;
    if(ptcb == getCurrentTask()){
	    ptcb->state = TASK_RUNNING;
	} else {
        task_resume(ptcb);
    }
}

err_t AllWaitTaskToRdy(list_t *plist)
{
    taskCB_t *ptcb;
    reg_t lock_status;
    uint8_t need_schedule_here = 0;
    
    while(!list_isempty(plist)) {
        lock_status = baseLock();
        //get next task
        ptcb = (taskCB_t*) plist->next;
        list_remove((list_t*)ptcb);
        //set error for return task???
        ptcb->returnMsg = E_TIMEOUT;
        task_resume(ptcb);
        baseUnLock(lock_status);
        need_schedule_here=1;
    }
    return need_schedule_here;
}
