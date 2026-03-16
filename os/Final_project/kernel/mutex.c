#include "os.h"

/*!< Table use to save SEM              */
MutexCB_t      MUTTbl[MAX_SEM];
static uint32_t     MutMap[MAX_SEM/MAP_SIZE];

/***
 interface 
* @brief Take Mutex
* @param semID stands for Mutex's ID 
* @details Recursive Lock, Safe Timeout, Ghost Wakeup / Lock Stealing Prevention
*/
err_t createMut(uint16_t initCnt,uint16_t maxCnt,uint8_t sortType)
{
    reg_t lock_status;
    lock_status = baseLock();
    for (int i=0; i<MAX_SEM;i++) {
        int mapIndex = i / MAP_SIZE;
        int mapOffset = i % MAP_SIZE;
        if ((MutMap[mapIndex] & (1<< mapOffset)) == 0)
        {
            MutMap[mapIndex] |= (1<< mapOffset);
            MUTTbl[i].id = i;
            MUTTbl[i].semCounter = initCnt;
            MUTTbl[i].initialCounter = maxCnt;
            MUTTbl[i].sortType = sortType;
            MUTTbl[i].owner = NULL;
            list_init((list_t*)&MUTTbl[i].node);
            baseUnLock(lock_status);
            return i;
        }
    }
    baseUnLock(lock_status);
    return E_CREATE_FAIL;
}

 
void delMut(uint16_t semID)
{   
    /* free semaphore control block */
    int mapIndex = semID / MAP_SIZE;
    int mapOffset = semID % MAP_SIZE;
    reg_t lock_status = baseLock();
    MutMap[mapIndex] &=~(1<<mapOffset);   
    /* wakeup all suspended threads */
    MutexCB_t *psemcb = &MUTTbl[semID];
    if (AllWaitTaskToRdy((list_t*)psemcb))//return 1: task_yield
    {
        baseUnLock(lock_status);  
        task_yield();                             
    }else
        baseUnLock(lock_status);  
}

/*
timeout = 0, try, -1: for ever
*/
err_t mut_take(uint16_t semID, int timeout){
    MutexCB_t  *pmutcb = &MUTTbl[semID];
    taskCB_t   *ptcb;
    reg_t lock_status=baseLock();

    if (pmutcb->semCounter > 0) {
        /* mutex is available */
        pmutcb->semCounter --;
        pmutcb->owner = getCurrentTask();
        pmutcb->lockCount = 1;
        baseUnLock(lock_status);
    } else {
        ptcb = getCurrentTask();
        if(pmutcb->owner==ptcb){
            /* recursive locking */
            pmutcb->lockCount++;
            baseUnLock(lock_status);
            return E_OK;
        }
        /* no waiting, return with timeout */
        if (timeout == 0) {
            baseUnLock(lock_status);
            return E_TIMEOUT;
        }
        //return OK
        
        ptcb->returnMsg = E_OK;
        DEBUG("[MutTake] Task %s Enter Wait List (Time=%d)\n", ptcb->name, timeout);
        /* adding the current task into waiting list*/
        TaskToWait((list_t*)pmutcb, pmutcb->sortType, ptcb);
        /* has waiting time, start thread timer */      
        if (timeout > 0) {
            //waiting in delayList
            setCurTimerCnt(ptcb->timer->timerID,timeout, timeout);
            startTimer(ptcb->timer->timerID);
        }  
        baseUnLock(lock_status);
        task_yield();

        //wake up here if has a value or timeout
        if (ptcb->returnMsg != E_OK) { //mean timeout, task remove from sem queue when taskTimeout
            DEBUG("[MutTake] Task %s Timeout Wakeup! Msg=%d\n", ptcb->name, ptcb->returnMsg);
            reg_t lock_status_2 = baseLock();
            list_remove(&ptcb->node); 
            baseUnLock(lock_status_2);
            DEBUG("[MutTake] Task %s Removed from List\n", ptcb->name);
            return ptcb->returnMsg;
        } else { //wake up from sem queue
            //prevent ghost wakeup and lock stealing by rechecking the owner
            DEBUG("[MutTake] Task %s Got Lock Wakeup!\n", ptcb->name);
            stopTimer(ptcb->timer->timerID); //remove timer from delayList
            if (pmutcb->owner != NULL && pmutcb->owner != ptcb) {
                reg_t lock_status_2 = baseLock();
                list_remove(&ptcb->node);
                baseUnLock(lock_status_2);
                return E_TIMEOUT;
            }
            
            reg_t lock_status_2=baseLock();
            pmutcb->owner = ptcb;
            pmutcb->lockCount = 1;
            baseUnLock(lock_status_2);
            return E_OK;
        }
    }
    return E_OK;
}

err_t mut_trytake(uint16_t semID) {
    return mut_take(semID, 0);
}

/**
 * @brief   Release Mutex
 * @param   semID stands for Mutex's ID 
 * @details Ownership Verification, Recursive Unlock, Preemptive Context Switch
 */
err_t mut_release(uint16_t semID)
{
    MutexCB_t *psemcb=&MUTTbl[semID];
    reg_t  need_schedule_sem;
    taskCB_t* ptcb;
    
    need_schedule_sem = 0;
    reg_t lock_status = baseLock();
    ptcb = getCurrentTask();
    DEBUG("[MutRel] Task %s releasing. Owner is %s\n", 
          ptcb->name, 
          psemcb->owner ? psemcb->owner->name : "NULL");
    // Ownership Verification. The main feature sets mutex apart from semaphore 
    if(ptcb!=psemcb->owner){
        baseUnLock(lock_status);
        return -1;
    }
    // Recursive Unlock. if the resource is not released completely, process will be returned directly.
    psemcb->lockCount--;
    if(psemcb->lockCount>0){  
        baseUnLock(lock_status);
        return E_OK;
    }
    psemcb->owner = NULL;
    if (!list_isempty((list_t*)psemcb)) {
        taskCB_t *nextTask = (taskCB_t *)psemcb->node.next;
        DEBUG("[MutRel] Waking up Task %s\n", nextTask->name);
        /* resume the suspended task */
        WaitTaskToRdy((list_t*)psemcb);
        need_schedule_sem = 1;
    } else {
        DEBUG("[MutRel] List is Empty. Counter++\n");
        //showAllQ(semID);
        if (psemcb->semCounter < MAX_SEM_VALUE){
            psemcb->semCounter++;
        }
        else {
            baseUnLock(lock_status);
            return E_FULL;
        }
    }
    baseUnLock(lock_status);
    if (need_schedule_sem)
        task_yield();

    return E_OK;

}    
