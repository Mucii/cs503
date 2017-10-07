#include <xinu.h>

devcall pipgetc(struct dentry *devptr) {
    intmask mask;
    pipid32 pipid = did32_to_pipid32((did32)devptr->dvnum);
    struct pipe_t *pipe;
    char ch;

    mask=disable();

    if(isbadpipid(pipid)){
        kprintf("bad2");
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];

    //kprintf("state is %d\n", pipe->state);

    //kprintf("currpid is %d, reader is %d\n", currpid, pipe->reader);

    if(proctab[pipe->writer].prstate == PR_FREE){
        pipe->state=PIPE_OTHER;
    }

    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER) || currpid != pipe->reader){
        //kprintf("state2\n");
    	restore(mask);
    	return SYSERR;
    }

    if(pipe->state == PIPE_OTHER && semtab[pipe->readersem].scount <=0){
        pipdisconnect((did32)devptr->dvnum);
        return SYSERR;
    }

    wait(pipe->readersem);
    //kprintf("count is %d\n", semcount(pipe->readersem));

    // when nonthing to read and wtier disconnected, clean up
    if(pipe->state == PIPE_OTHER && semtab[pipe->readersem].scount < 0){
        pipdisconnect((did32)devptr->dvnum);
        restore(mask);
        return SYSERR;
    }
    ch = pipe->buf[pipe->readerid];
    pipe->readerid++;
    pipe->readerid %= PIPE_SIZE;
    //kprintf(" get %c\n", ch);

    signal(pipe->writersem);

    
    if(pipe->state == PIPE_OTHER && semtab[pipe->readersem].scount < 0){
        pipdisconnect((did32)devptr->dvnum);
        return SYSERR;
    }

    restore(mask);
    return ch;
}

