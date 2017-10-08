#include <xinu.h>

devcall pipgetc(struct dentry *devptr) {
    intmask mask;
    pipid32 pipid = did32_to_pipid32((did32)devptr->dvnum);
    struct pipe_t *pipe;
    char ch;

    mask=disable();

    if(isbadpipid(pipid)){
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];

    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER) || currpid != pipe->reader){
    	restore(mask);
    	return SYSERR;
    }

    if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <=0){
        pipdisconnect((did32)devptr->dvnum);
        restore(mask);
        return SYSERR;
    }

    wait(pipe->readersem);
    //kprintf("count is %d\n", semcount(pipe->readersem));

    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER)){
        restore(mask);
        return SYSERR;
    }

    if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <0){
        pipdisconnect((did32)devptr->dvnum);
        restore(mask);
        return SYSERR;
    }

    
    ch = pipe->buf[pipe->readerid];
    pipe->readerid++;
    pipe->readerid %= PIPE_SIZE;
    //kprintf(" get %c\n", ch);

    signal(pipe->writersem);

    
    if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <0){
        pipdisconnect((did32)devptr->dvnum);
    }

    restore(mask);
    return ch;
}

