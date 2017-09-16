/*  main.c  - main */

#include <xinu.h>

local void foo1(void){
	int i=1;
	while(i<100000000){
		i++;}
	return;
}

local void foo2(void){
	sleepms(2000);
	int i=1;
	while(i<10000000000000){i++;}
	return;
}

local void iobound1(void) {
  int32 i,j,k;
  for (i = 0; i < 30; i++) {
      sleepms(71);
      kprintf("%s, %d\n\n", proctab[currpid].prname,proctab[currpid].prprio);
  }
  kprintf("end %s\n\n",proctab[currpid].prname);
  return;
}

local void iobound2(void) {
  int32 i,j,k;
  for (i = 0; i < 30; i++) {
      sleepms(101);
      kprintf("%s, %d\n\n", proctab[currpid].prname,proctab[currpid].prprio);
  }
  kprintf("end %s\n\n",proctab[currpid].prname);
  return;
}

local void iobound3(void) {
  int32 i,j,k;
  for (i = 0; i < 30; i++) {
      sleepms(157);
      kprintf("%s, %d\n\n", proctab[currpid].prname,proctab[currpid].prprio);
  }
  kprintf("end %s\n\n",proctab[currpid].prname);
  return;
}

local void cpubound(void) {
  int32 i,j,k;
  for (i = 0; i < 10; i++) {
    for (j = 0; j < 100000000; j++) {
      k = 1;
    }
    kprintf("%s, %d\n\n", proctab[currpid].prname,proctab[currpid].prprio);
  }
  kprintf("end %s\n\n",proctab[currpid].prname);
  return;
}

process	main(void)
{

	/*Run the Xinu shell 

	recvclr();
	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	* Wait for shell to exit and recreate it *

	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}
	return OK;*/



	//chgprio(1,50);
	// Teste for aging scheduling 


	// Test 1

	//resume(create(foo1,INITSTK,TSSCHED,25,"ts1",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,25,"ps1",0,NULL));
	//resume(create(foo1,INITSTK,TSSCHED,25,"ts2",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,25,"ps2",0,NULL));


	// Test 2

	//chgprio(PROPORTIONALSHARE,50);

	//resume(create(foo1,INITSTK,TSSCHED,25,"ts1",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,25,"ps1",0,NULL));
	//resume(create(foo1,INITSTK,TSSCHED,25,"ts2",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,25,"ps2",0,NULL));


	// Test 3

	//chgprio(TSSCHED,15);

	// change the prio to 50 to make sure ts take turns

	//resume(create(foo1,INITSTK,TSSCHED,10,"ts1",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps1",0,NULL));
	//resume(create(foo1,INITSTK,TSSCHED,10,"ts2",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps2",0,NULL));



	// PS testing 

	// Test 1

	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps1",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps2",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps3",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps4",0,NULL));


	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,30,"ps1",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps2",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,30,"ps3",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps4",0,NULL));


	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps1",0,NULL));
	//resume(create(foo2,INITSTK,PROPORTIONALSHARE,10,"ps2",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps3",0,NULL));
	//resume(create(foo2,INITSTK,PROPORTIONALSHARE,10,"ps4",0,NULL));

	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps1",0,NULL));
	//resume(create(foo2,INITSTK,PROPORTIONALSHARE,10,"ps2",0,NULL));
	//resume(create(foo1,INITSTK,PROPORTIONALSHARE,10,"ps3",0,NULL));
	//resume(create(foo2,INITSTK,PROPORTIONALSHARE,10,"ps4",0,NULL));

	resume(create(foo1,INITSTK,PROPORTIONALSHARE,50,"ps1",0,NULL));
	resume(create(foo2,INITSTK,PROPORTIONALSHARE,60,"ps2",0,NULL));
	resume(create(foo1,INITSTK,PROPORTIONALSHARE,50,"ps3",0,NULL));
	resume(create(foo2,INITSTK,PROPORTIONALSHARE,60,"ps4",0,NULL));

	// TS scheduling

	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts1", 0, NULL));
  	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts2", 0, NULL));
  	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts3", 0, NULL));
 	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts4", 0, NULL));
 	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts5", 0, NULL));
 	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts6", 0, NULL));

	/*resume(create(iobound1, INITSTK, TSSCHED, 30, "ts1", 0, NULL));
	resume(create(iobound2, INITSTK, TSSCHED, 30, "ts3", 0, NULL));
	resume(create(iobound3, INITSTK, TSSCHED, 30, "ts5", 0, NULL));
  	resume(create(cpubound, INITSTK, TSSCHED, 30, "ts2", 0, NULL));
 	resume(create(cpubound, INITSTK, TSSCHED, 30, "ts4", 0, NULL));
 	resume(create(cpubound, INITSTK, TSSCHED, 30, "ts6", 0, NULL));*/














	//
	//
	// Test 1 - Null process is not running
	//resume(create(iobound, INITSTK, TSSCHED, 30, "ts1", 0, NULL));
  	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts2", 0, NULL));
  	//resume(create(iobound, INITSTK, TSSCHED, 30, "ts3", 0, NULL));
 	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts4", 0, NULL));
 	//resume(create(iobound, INITSTK, TSSCHED, 30, "ts5", 0, NULL));
 	//resume(create(cpubound, INITSTK, TSSCHED, 30, "ts6", 0, NULL));

	return OK;
    
}
