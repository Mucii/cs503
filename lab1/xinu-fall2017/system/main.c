#include <xinu.h>

void sample_proc_cpu() {
        int i, j;
        int LOOP1 = 10; 
        int LOOP2 = 10000000;

        struct procent *pr = &proctab[currpid];

        int v = 0;
        for (i=0; i<LOOP1; i++) {
                for (j=0; j<LOOP2; j++) {
                        v += i * j;
                }   
                kprintf("PID: %d, Loop: %d\n", 
                                currpid, i); 
        }   

        kprintf("===== CPU BOUNDED PID %d ends\n", currpid);
}

void sample_proc_io(uint32 time) {
        int i;
        int LOOP1 = 30; 

        struct procent *pr = &proctab[currpid];

        for (i=0; i<LOOP1; i++) {
                sleepms(time);
                kprintf("PID: %d, Sleep time: %d, Loop: %d\n", 
                                currpid, time, i); 
        }   

        kprintf("===== IO BOUNDED PID %d ends\n", currpid);
}

process main() {
        kprintf("===TS TEST3===\n");
        resched_cntl(DEFER_START);
        for (int i = 0; i < 6; i++) {
                if (i % 2 == 0) {
                        resume(create(sample_proc_cpu, 1024, TS, 20, "cpu-intense", 0));
                }
                else {
                        resume(create(sample_proc_io, 1024, TS, 20, "io-intense", 1, 32));
                }
        }
        resched_cntl(DEFER_STOP);

	return OK;
}