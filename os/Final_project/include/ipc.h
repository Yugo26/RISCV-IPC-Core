#ifndef _IPC_H
#define _IPC_H

#include "types.h"

typedef struct SemCB
{
    list_t        node;                    /*!< Task waitting list.              */
    uint16_t      id;                      /*!< ECB id                           */
    uint8_t       sortType;                /*!< 0:FIFO 1: Preemptive by prio     */
    uint16_t      semCounter;              /*!< Counter of semaphore.            */
    uint16_t      initialCounter;          /*!< Initial counter of semaphore.    */
} SemCB_t;


typedef struct MboxCB
{
    list_t        node;                    /*!< Task waitting list.              */
    uint16_t      id;                      /*!< ECB id                           */
    uint8_t       sortType;                /*!< 0:FIFO 1: Preemptive by prio     */
    uint8_t       mailCount;               // if mailCount==1 has a mail
    void *        mailPtr;
} MboxCB_t;

typedef struct MutexCB
{
    list_t        node;                  /*!< Task waitting list. */
    uint16_t      id;                    /*!< ECB id */
    uint8_t       sortType;              /*!< 0:FIFO 1: Preemptive by prio */
    uint16_t      semCounter;            /*!< Counter of mutex. */  
    uint16_t      initialCounter;        /*!< Initial counter of semaphore. */  
    taskCB_t*     owner;                 /*!< Recording for which task has the the mutex*/
    int           lockCount;             /*!< Recursive counter*/

} MutexCB_t;

typedef struct MsgQCB
{
    list_t      node;                    /*!< Task waitting list. */
    uint16_t    id;                      /*!< Message Queue id */
    void        *pBuffer;                /*!< Initiate address pointing to a ring buffer */
    uint32_t    bufferSize;              /*!< The total size of buffer. */
    uint32_t    msgSize;                 /*!< The size of one message. */
    uint32_t    maxMsgs;                 /*!< Maximum number for storing messages in the queue*/
    uint32_t    head;                    /*!< The index where consumer reads. */
    uint32_t    tail;                    /*!< The index where producer writes.*/
    uint32_t    msgCount;                /*!< The current amount of message in the queue */
    list_t      sendWaitList;            /*!< Producer's waiting list. */
    list_t      recvWaitList;            /*!< Consumer's waiting list. */
} MsgQCB_t;

#define EVENT_WAIT_OR   0x01
#define EVENT_WAIT_AND  0x02
#define EVENT_WAIT_CLR  0x04

typedef struct EventCB
{
    list_t      node;                    /*!< Task waitting list.*/
    uint32_t    id;                      /*!< Event Group id*/
    uint32_t    eventFlags;              /*!< Event Mask*/
    list_t      waitList;                /*!< Event waiting list*/
} EventCB_t;

#endif  