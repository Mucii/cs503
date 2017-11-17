#include <icu.s>
.text
.globl pgfault
pgfault:
pushal
pushfl
call pg_fault_handler
popfl
popal
add $0x4, %esp  /*remove error code which is 4 byte*/
iret
