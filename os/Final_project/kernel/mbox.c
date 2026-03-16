#include "os.h"

/*!< Table use to save MAIL_BOX              */
static MboxCB_t     MboxTbl[MAX_MBOXS];
static uint32_t     MboxMap[MAX_MBOXS/MAP_SIZE];


/***
 interface 
*/
err_t createMbox(uint8_t sortType)
{
    reg_t lock_status;
    lock_status = baseLock();
    for (int i=0; i<MAX_MBOXS;i++) {
        int mapIndex = i / MAP_SIZE;
        int mapOffset = i % MAP_SIZE;
        if ((MboxMap[mapIndex] & (1<< mapOffset)) == 0)
        {
            MboxMap[mapIndex] |= (1<< mapOffset);
            MboxTbl[i].id = i;
            MboxTbl[i].mailPtr = NULL;
            MboxTbl[i].sortType = sortType;
            list_init((list_t*)&MboxTbl[i].node);
            baseUnLock(lock_status);
            return i;
        }
    }
    baseUnLock(lock_status);
    return E_CREATE_FAIL;
}

 
void delMbox(uint16_t mboxID)
{
    MboxCB_t *pmboxcb = &MboxTbl[mboxID];
    
    /* free mbox control block */
    int mapIndex = mboxID / MAP_SIZE;
    int mapOffset = mboxID % MAP_SIZE;
    reg_t lock_status = baseLock();
    /* wakeup all suspended threads */
    AllWaitTaskToRdy((list_t*)pmboxcb);
    MboxMap[mapIndex] &=~(1<<mapOffset);   
    baseUnLock(lock_status);                               
}


err_t postMbox(uint16_t mboxID, void *pmail){
    MboxCB_t *pmboxcb = &MboxTbl[mboxID];

    if (pmboxcb->mailCount == 0) //mailbox empty
    {
        reg_t lock_status=baseLock();
        pmboxcb->mailPtr = pmail;
        pmboxcb->mailCount = 1;
        //check waiting list
        WaitTaskToRdy((list_t*)pmboxcb);
        baseUnLock(lock_status);
        return E_OK;
    } else {
        return E_FULL;
    }
}
    
/*
 cal to accept a mail, no wait
*/
void * acceptMail(uint16_t mboxID, err_t *perr) {
    MboxCB_t *pmboxcb = &MboxTbl[mboxID];
    void *pmail;
    reg_t lock_status = baseLock();
    if (pmboxcb->mailCount == 1) {
        *perr = E_OK;
        pmail = pmboxcb->mailPtr;
        pmboxcb->mailPtr = NULL;
        pmboxcb->mailCount = 0;
        baseUnLock(lock_status);
        return pmail;
    } else {
        baseUnLock(lock_status);
        *perr = E_EMPTY;
        return NULL;
    }
}

/*
called to wait a mail
timeout >0, 0 for ever
*/
void * waitMail(uint16_t mboxID, uint32_t timeout, err_t *perr) {
    MboxCB_t *pmboxcb = &MboxTbl[mboxID];
    void *pmail;
    reg_t lock_status = baseLock();
    taskCB_t *pcurrentTask;

    if (pmboxcb->mailCount == 1) {
        *perr = E_OK;
        pmail = pmboxcb->mailPtr;
        pmboxcb->mailPtr = NULL;
        pmboxcb->mailCount = 0;
        baseUnLock(lock_status);
        return pmail;
    } else {
        baseUnLock(lock_status);
        pcurrentTask = getCurrentTask();
        pcurrentTask->returnMsg = E_OK;
        if (timeout==0) { //wait forever until mbox deleted or has a mail
            TaskToWait((list_t*)pmboxcb, pmboxcb->sortType, pcurrentTask);
            task_yield();
            //wake up here
            if (pcurrentTask->returnMsg == E_OK) {
                *perr = E_OK;
                lock_status = baseLock();
                pmail = pmboxcb->mailPtr;
                pcurrentTask->pmail = NULL;
                pmboxcb->mailCount = 0;            
                baseUnLock(lock_status);
                return pmail;
            }else{
                //mbox delete
                *perr = E_TIMEOUT;
                return NULL;
            }
        } else { //wait timeout ticks 
            lock_status = baseLock();
            TaskToWait((list_t*)pmboxcb, pmboxcb->sortType, pcurrentTask); //tcb wait in mbox queue
             //timer of task waiting in delayList
            setCurTimerCnt(pcurrentTask->timer->timerID,timeout, timeout);
            startTimer(pcurrentTask->timer->timerID);
            baseUnLock(lock_status);
            //wake up here if timeout or mbox delete or has a mail
            if (pcurrentTask->returnMsg == E_OK) { //wake up from mbox queue
                stopTimer(pcurrentTask->timer->timerID); //remove from delayList
                *perr = E_OK;
                lock_status = baseLock();
                pmail = pmboxcb->mailPtr;
                pcurrentTask->pmail = NULL;
                pmboxcb->mailCount = 0;            
                baseUnLock(lock_status);
                return pmail;
            }else{
                //mbox delete or timeout
                //remove from mbox list task_resume is called by  taskTimeout
                *perr = E_TIMEOUT;
                return NULL;
            }
        }
    }
}