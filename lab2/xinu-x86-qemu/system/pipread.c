#include <xinu.h>

uint32 pipread(struct dentry *devptr, char* buf, uint32 len) {
    intmask mask;
    pipid32 pipid = did32_to_pipid32((did32)devptr->dvnum);
    struct pipe_t *pipe;
    int i;

    mask = disable();

    if(isbadpipid(pipid)){
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];

    //kprintf("readerr is %d\n",pipe->readerid);

    //kprintf("writerr is %d\n",pipe->writerid);


    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER) || currpid != pipe->reader){
        restore(mask);
        return SYSERR;
    }

    if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <=0){
        pipdisconnect((did32)devptr->dvnum);
        restore(mask);
        return SYSERR;
    }

    for(i=0; i<len; i++){
    	wait(pipe->readersem);

    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER)){
        restore(mask);
        return i;
    }

    if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <0){
        pipdisconnect((did32)devptr->dvnum);
        restore(mask);
        return i;
    }

    	buf[i] = pipe->buf[pipe->readerid];
    	pipe->readerid++;
    	pipe->readerid %= PIPE_SIZE;
        //kprintf(" get %c\n", buf[i]);

    	signal(pipe->writersem);
    }

    if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <0){
        pipdisconnect((did32)devptr->dvnum);
    }


    restore(mask);
	return i;
}
