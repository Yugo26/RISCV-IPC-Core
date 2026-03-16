#include "os.h"

EventCB_t   EveTbl[MAX_EVENT];
static uint32_t    EveMap[MAX_EVENT/MAP_SIZE];

extern uint8_t need_schedule;

err_t eventCreate()
{
    reg_t lock_status;
    lock_status = baseLock();
    for(int i = 0;i<MAX_EVENT;i++){
        int mapIndex = i / MAP_SIZE;
        int mapOffset = i % MAP_SIZE;
        if((EveMap[mapIndex] & (1<<mapOffset))==0){
            EveMap[mapIndex] |= (1<<mapOffset);
            EveTbl[i].id = i;
            EveTbl[i].eventFlags = 0;
            list_init((list_t*)&EveTbl[i].node);
            list_init((list_t*)&EveTbl[i].waitList);
            baseUnLock(lock_status);
            return i;
        }
    }
    baseUnLock(lock_status);
    return E_CREATE_FAIL;
}

/**
 * @brief Wait for Event Flags
 * @details Supports complex multi-event synchronization logic, featuring spurious wakeup prevention and an Auto-Clear mechanism.
 */

err_t event_wait(uint32_t id, uint32_t waitBits, uint8_t mode, uint32_t timeout)
{
    EventCB_t *event = &EveTbl[id];
    taskCB_t  *ptcb;
    reg_t lock_status;
    lock_status = baseLock();

    ptcb = getCurrentTask();
    /* fast path check:Stay unblocked if conditions are met*/
    if((mode & EVENT_WAIT_AND)==EVENT_WAIT_AND){
        if((event->eventFlags & waitBits)==waitBits){
            if((mode & EVENT_WAIT_CLR)==EVENT_WAIT_CLR){
                event->eventFlags &= ~waitBits;
            }
            baseUnLock(lock_status);
            return E_OK;
        }
    }
    if((mode & EVENT_WAIT_OR)==EVENT_WAIT_OR){
        if((event->eventFlags & waitBits)!=0){
            if((mode & EVENT_WAIT_CLR)==EVENT_WAIT_CLR){
                event->eventFlags &= ~waitBits;
            }
            baseUnLock(lock_status);
            return E_OK;
        }
    }
    /* Conditions not met; preparing to sleep*/
    if (timeout == 0) {
        baseUnLock(lock_status);
        return E_TIMEOUT;
    }
    /* Record the conditions task is waiting for */
    ptcb->returnMsg = E_OK;
    ptcb->waitEventBits = waitBits;
    ptcb->waitEventMode = mode;
    list_t *waitlist = &event->waitList;
    TaskToWait(waitlist, FIFO, ptcb);
    if (timeout > 0) {
        //waiting in delayList
        setCurTimerCnt(ptcb->timer->timerID,timeout, timeout);
        startTimer(ptcb->timer->timerID);
    }  
    baseUnLock(lock_status);
    task_yield();

    if(ptcb->returnMsg != E_OK){
        reg_t lock_status_2 = baseLock();
        list_remove(&ptcb->node); 
        baseUnLock(lock_status_2);
        return ptcb->returnMsg;
    } else{
        stopTimer(ptcb->timer->timerID);
        lock_status = baseLock();
        int condition_still_met = 0;
        /* prevent Spurious Wakeup by rechecking the status of event*/
        if((mode & EVENT_WAIT_AND) == EVENT_WAIT_AND){
            if((event->eventFlags & waitBits) == waitBits) condition_still_met = 1;
        }
        else if((mode & EVENT_WAIT_OR) == EVENT_WAIT_OR){
            if((event->eventFlags & waitBits) != 0) condition_still_met = 1;
        }
        if (condition_still_met == 0) {
            DEBUG("[Kernel] Spurious Wakeup Detected! Flags=0x%x\n", event->eventFlags);
            baseUnLock(lock_status);
            return -1;
        }
        /* Auto-Clear*/
        DEBUG("[Kernel] Woke up Valid! Mode=0x%x, CLR_Flag=0x%x\n", mode, EVENT_WAIT_CLR);
        if ((mode & EVENT_WAIT_CLR) == EVENT_WAIT_CLR) {
            DEBUG("[Kernel] Clearing Flags... Before: 0x%x\n", event->eventFlags);
            event->eventFlags &= ~waitBits;
            DEBUG("[Kernel] Cleared! After: 0x%x\n", event->eventFlags);
        }
    }
    baseUnLock(lock_status);    
    return E_OK;
}

/**
 * @brief Set Event Flags
 * @details Adopt Broadcast. Upon an event status change, 
 *          the waitlist is scanned to wake up tasks that meet their conditions.
 */

err_t event_set(uint32_t id, uint32_t setFlags)
{
    EventCB_t *event = &EveTbl[id];
    taskCB_t  *ptcb;
    reg_t lock_status;
    lock_status = baseLock();
    need_schedule = 0;
    /* set eventbit*/
    event->eventFlags |= setFlags;
    /* Broadcast & Safe Iteration*/
    list_t *waitL = &event->waitList;
    list_t *pwaitL = waitL->next;
    while(pwaitL != waitL){
        ptcb = (taskCB_t*)pwaitL;
        uint32_t pwaitBits = ptcb->waitEventBits;
        uint8_t  pwaitMode = ptcb->waitEventMode;
        int condition_met = 0;
        list_t *next_node = pwaitL->next;
        if((pwaitMode&EVENT_WAIT_AND)==EVENT_WAIT_AND){
            if((event->eventFlags&pwaitBits)==pwaitBits){
                condition_met = 1;
            }
        }
        if((pwaitMode&EVENT_WAIT_OR)==EVENT_WAIT_OR){
            if((event->eventFlags&pwaitBits)!=0){
                condition_met = 1;
            }
        }
        /* if conditions are met, remove the task form waitlist and wakeup to ready*/
        if(condition_met){
            list_remove(pwaitL);
            task_resume(ptcb);
            ptcb->returnMsg = E_OK;
            need_schedule = 1;
        }
        pwaitL = next_node;
    }
    DEBUG("[Debug] Set : EventCB Addr = %p, Flags = 0x%x\n", event, event->eventFlags);
    baseUnLock(lock_status);
    if(need_schedule){
        task_yield();
    }
    return E_OK;
}