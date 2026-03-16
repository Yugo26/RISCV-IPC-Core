#include "syscallTable.h"
.text
.globl gethid
.align 4
gethid:
    li a7, GETHID
    ecall
    ret

.globl delay
.align 4
delay:
    li a7, DELAY
    ecall
    ret

.globl u_sem_take
.align 4
u_sem_take:
    li a7, SEM_TAKE
    ecall
    ret

.globl u_sem_release
.align 4
u_sem_release:
    li a7, SEM_RELEASE
    ecall
    ret

.globl u_sem_create
.align 4
u_sem_create:
    li a7, SEM_CREATE
    ecall
    ret

.globl u_mut_take
.align 4
u_mut_take:
    li a7, MUT_TAKE
    ecall
    ret

.globl u_mut_release
.align 4
u_mut_release:
    li a7, MUT_RELEASE
    ecall
    ret

.globl u_mut_create
.align 4
u_mut_create:
    li a7, MUT_CREATE
    ecall
    ret

.globl u_msgq_create
.align 4
u_msgq_create:
    li a7, MSGQ_CREATE
    ecall
    ret

.globl u_msgq_send
.align 4
u_msgq_send:
    li a7, MSGQ_SEND
    ecall
    ret

.globl u_msgq_recv
.align 4
u_msgq_recv:
    li a7, MSGQ_RECV
    ecall
    ret

.globl u_event_create
.align 4
u_event_create:
    li a7, EVENT_CREATE
    ecall
    ret

.globl u_event_wait
.align 4
u_event_wait:
    li a7, EVENT_WAIT
    ecall
    ret

.globl u_event_set
.align 4
u_event_set:
    li a7, EVENT_SET
    ecall
    ret

.end