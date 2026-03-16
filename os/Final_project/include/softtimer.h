#ifndef __TIMER_H__
#define __TIMER_H__
#include "types.h"

//all 08

#define   TMR_RUNNING     0       /*!< Timer State: Running             */
#define   TMR_STOPPED     1       /*!< Timer State: Stopped             */
#define   TMR_ONE_SHOT    0       /* Timer Type: one shot */       
#define   TMR_PERIOD      1       /* Timer Type: period */

typedef struct timerCB                  /* Timer Control Block Define.        */
{
    list_t            node;
    uint16_t          timerID;             /*!< Timer ID.                        */
    uint8_t           timerType;           /*!< Timer Type.                      */
    uint8_t           timerState;          /*!< Timer State.                     */
    int               timerCnt;            /*!< Timer Counter.                   */
    uint32_t          timerReload;         /*!< Timer Reload Counter Value.      */	
    void (*timerCallBack)(void *parameter); /*!< Call-back Function When Timer overrun. */	
    void *parameter;
}timerCB_t;


#endif /* __TIMER_H__ */