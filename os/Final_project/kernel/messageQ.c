#include "os.h"

MsgQCB_t    MsgQTbl[256];
static uint32_t MsgQMap[256/MAP_SIZE];

extern uint8_t need_schedule;

err_t MsgQCreate(uint32_t msgSize, uint32_t maxMsgs)
{
    reg_t lock_status;
    lock_status = baseLock();
    for(int i = 0;i<256;i++){
        int mapIndex = i / MAP_SIZE;
        int mapOffset = i % MAP_SIZE;
        if ((MsgQMap[mapIndex] & (1<< mapOffset)) == 0){
            uint32_t totalBytes = msgSize*maxMsgs;
            void *buffer = malloc(totalBytes);
            if(buffer == NULL){
                baseUnLock(lock_status);
                return -1;
            }
            MsgQMap[mapIndex] |= (1<< mapOffset);      
            MsgQTbl[i].id = i;
            MsgQTbl[i].pBuffer = buffer;
            MsgQTbl[i].msgSize = msgSize;
            MsgQTbl[i].maxMsgs = maxMsgs;
            MsgQTbl[i].head = 0;
            MsgQTbl[i].tail = 0;
            MsgQTbl[i].msgCount = 0;
            list_init(&MsgQTbl[i].sendWaitList);
            list_init(&MsgQTbl[i].recvWaitList);
            list_init(&MsgQTbl[i].node);
            baseUnLock(lock_status);
            return i;
        }
    }
    baseUnLock(lock_status);
    return -1;
}

/**
 * @brief Message Queue Send
 * @details possessing ring buffer pointer arithmetic with O(1) time complexity
 *          and using memcpy to deep copy, ensuring User and Kernel are separated
 */

err_t MsgQSend(uint16_t id, void *pBuffer, uint32_t nBytes, int timeout)
{   
    MsgQCB_t *msgq = &MsgQTbl[id];
    taskCB_t *ptcb;
    reg_t lock_status;
    lock_status = baseLock();

    /* prevent to overwrite the memory*/
    if(nBytes != msgq->msgSize){
        baseUnLock(lock_status);
        return -1;
    }
    /* Producer Blocking if queue is full the task will be added into sendWaitList based on its timeout.*/
    if(msgq->msgCount>=msgq->maxMsgs){
        if (timeout == 0) {
            baseUnLock(lock_status);
            return E_TIMEOUT;
        }
        ptcb = getCurrentTask();
        ptcb->returnMsg = E_OK;
        list_t* psendQ = &msgq->sendWaitList;
        TaskToWait(psendQ, FIFO, ptcb);
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
            /* normal wakeup*/
            stopTimer(ptcb->timer->timerID);
            lock_status = baseLock();
        }        
    }
    /* Circular Buffer Write and Deep Copy*/
    uint32_t offset = (msgq->tail)*(msgq->msgSize);
    void* tarAddr = (char*)msgq->pBuffer+(offset);
    memcpy(tarAddr, pBuffer, nBytes);
    msgq->tail++;
    if (msgq->tail >= msgq->maxMsgs) {
        msgq->tail = 0;
    }
    msgq->msgCount++;
    /* Wakeup Consumer*/
    list_t *precvQ = &msgq->recvWaitList;
    if(!list_isempty(precvQ)){
        WaitTaskToRdy(precvQ);
        need_schedule = 1;
    }
    baseUnLock(lock_status);
    if(need_schedule)
        task_yield();

    return E_OK;
}

/**
 * @brief Message Queue Receive
 * @details implement Consumer function. 
 */

err_t MsgQRecv(uint16_t id, void *pBuffer, uint32_t nBytes, int timeout)
{
    MsgQCB_t *msgq = &MsgQTbl[id];
    taskCB_t *ptcb;
    reg_t lock_status;
    lock_status = baseLock();

    if(nBytes != msgq->msgSize){
        baseUnLock(lock_status);
        return -1;
    }
    /* if the queue is empty, the task will sleep in recvWaitList until producer produce a new message.*/
    if(msgq->msgCount==0){
        if (timeout == 0) {
            baseUnLock(lock_status);
            return E_TIMEOUT;
        }
        ptcb = getCurrentTask();
        ptcb->returnMsg = E_OK;
        list_t *precvQ = &msgq->recvWaitList;
        TaskToWait(precvQ, FIFO, ptcb);
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
        } 
    }
    /* Deep copy from Kernel to User*/
    uint32_t offset = (msgq->head)*(msgq->msgSize);
    void* tarAddr = (char*)msgq->pBuffer+(offset);
    memcpy(pBuffer, tarAddr, nBytes);
    /* Wrap Around*/
    msgq->head++;
    if (msgq->head >= msgq->maxMsgs) {
        msgq->head = 0;
    }
    msgq->msgCount--;
    /* Wakeup Producer*/
    list_t *psendQ = &msgq->sendWaitList;
    if(!list_isempty(psendQ)){
        WaitTaskToRdy(psendQ);
        need_schedule = 1;
    }
    baseUnLock(lock_status);
    if(need_schedule)
        task_yield();
        
    return E_OK;
}