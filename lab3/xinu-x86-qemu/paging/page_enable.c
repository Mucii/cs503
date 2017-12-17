#include <xinu.h>

/*thi is to set register of cr0 cr2 cr3 and enable paging.*/


unsigned long tmp;

void set_cr0(unsigned long n) {
  intmask mask;
  mask = disable();

  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");    /* mov (move) value at tmp into %eax register."l" signifies long (see docs on gas assembler)  */
  asm("movl %eax, %cr0");
  asm("popl %eax");

  restore(mask);
  return;
}



void set_cr3(unsigned long n) {
  /* we cannot move anything directly into
     %cr4. This must be done via %eax. Therefore
     we save %eax by pushing it then pop
     it at the end.
  */

  intmask mask;
  mask=disable();

  tmp = n;
  asm("pushl %eax");
  asm("movl tmp, %eax");
  asm("movl %eax, %cr3");
  asm("popl %eax");

  restore(mask);
  return;
}


unsigned long read_cr0(void) {

  intmask mask;
  mask = disable();
  unsigned long local_tmp;

  asm("pushl %eax");
  asm("movl %cr0, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(mask);

  return local_tmp;
}


unsigned long read_cr2(void) {

  intmask mask;
  mask = disable();
  unsigned long local_tmp;

  asm("pushl %eax");
  asm("movl %cr2, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(mask);

  return local_tmp;
}

unsigned long read_cr3(void) {

  intmask mask;
  mask = disable();
  unsigned long local_tmp;

  asm("pushl %eax");
  asm("movl %cr3, %eax");
  asm("movl %eax, tmp");
  asm("popl %eax");

  local_tmp = tmp;

  restore(mask);

  return local_tmp;
}


//enable paging by set cr0 | 0x80000000
void enable_paging(){
  intmask mask;
  mask =disable();
  unsigned long temp = read_cr0();
  temp = temp | (0x80000000);
  set_cr0(temp);
  restore(mask);
  return;
}

// set cr3 the base register of page directory
void set_pd_reg(unsigned long pd_reg){
// use last 20 digits as base
  intmask mask;
  mask =disable();
	pd_reg = pd_reg << 12;
	set_cr3(pd_reg);
  restore(mask);
  return;
}







