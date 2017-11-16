#include <icu.s>
            .text   
errcode:  .long 0
            .globl  pgfault,errcode  
pgfault:
    popl    errcode   /* pop the error code from top of the stack */
    pushfl              /* store the flag registers                 */
    cli                 /* disable interrupts                       */
    pushal              /* save all general registers               */
    call     pg_fault_handler /* call page fault handler ISR      */
    popal               /* restore general purpose register         */
    popfl               /* restore flag register                    */
    iret                /* return from interrupt handler routine    */
