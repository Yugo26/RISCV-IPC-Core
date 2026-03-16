#include "os.h"

/*!< Table use to save SEM              */
SemCB_t      SEMTbl[MAX_SEM];
static uint32_t     SemMap[MAX_SEM/MAP_SIZE];

/***
 interface 
*/
err_t createSem(uint16_t initCnt,uint16_t maxCnt,uint8_t sortType)
{
    reg_t lock_status;
    lock_status = baseLock();
    for (int i=0; i<MAX_SEM;i++) {
        int mapIndex = i / MAP_SIZE;
        int mapOffset = i % MAP_SIZE;
        if ((SemMap[mapIndex] & (1<< mapOffset)) == 0)
        {
            SemMap[mapIndex] |= (1<< mapOffset);
            SEMTbl[i].id = i;
            SEMTbl[i].semCounter = initCnt;
            SEMTbl[i].initialCounter = maxCnt;
            SEMTbl[i].sortType = sortType;
            list_init((list_t*)&SEMTbl[i].node);
            baseUnLock(lock_status);
            return i;
        }
    }
    baseUnLock(lock_status);
    return E_CREATE_FAIL;
}

 
void delSem(uint16_t semID)
{   
    /* free semaphore control block */
    int mapIndex = semID / MAP_SIZE;
    int mapOffset = semID % MAP_SIZE;
    reg_t lock_status = baseLock();
    SemMap[mapIndex] &=~(1<<mapOffset);   
    /* wakeup all suspended threads */
    SemCB_t *psemcb = &SEMTbl[semID];
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
err_t sem_take(uint16_t semID, int timeout){
    SemCB_t *psemcb = &SEMTbl[semID];
    taskCB_t *ptcb;
    reg_t lock_status=baseLock();

    if (psemcb->semCounter > 0) {
        /* semaphore is available */
        psemcb->semCounter --;
        baseUnLock(lock_status);
    } else {
        /* no waiting, return with timeout */
        if (timeout == 0) {
            baseUnLock(lock_status);
            return E_TIMEOUT;
        }
        //return OK
        ptcb = getCurrentTask();
        ptcb->returnMsg = E_OK;
        TaskToWait((list_t*)psemcb, psemcb->sortType, ptcb);
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
            return ptcb->returnMsg;
        } else { //wake up from sem queue
            stopTimer(ptcb->timer->timerID); //remove timer from delayList
            return E_OK;
        }
    }
    return E_OK;
}

err_t sem_trytake(uint16_t semID) {
    return sem_take(semID, 0);
}


err_t sem_release(uint16_t semID)
{
    SemCB_t *psemcb=&SEMTbl[semID];
    reg_t  need_schedule_sem;
    
    need_schedule_sem = 0;
    reg_t lock_status = baseLock();

    if (!list_isempty((list_t*)psemcb)) {
        /* resume the suspended task */
        WaitTaskToRdy((list_t*)psemcb);
        need_schedule_sem = 1;
    } else {
        //showAllQ(semID);
        if (psemcb->semCounter < MAX_SEM_VALUE) 
            psemcb->semCounter++;
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


