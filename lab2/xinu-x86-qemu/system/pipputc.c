#include <xinu.h>

devcall pipputc(struct dentry *devptr, char ch) {
    intmask mask;
    pipid32 pipid = did32_to_pipid32((did32)devptr->dvnum);
    struct pipe_t *pipe;

    mask = disable();

    //kprintf("char is %c\n",ch);

    if(isbadpipid(pipid)){
        //kprintf("bad1");
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];

    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER) || currpid != pipe->writer){
        //kprintf("state1\n");
    	restore(mask);
    	return SYSERR;
    }

    if(pipe->state==PIPE_OTHER){
        pipdisconnect((did32)devptr->dvnum);
        restore(mask);
        return SYSERR;
    }

    wait(pipe->writersem);

    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER)){
        //kprintf("state1\n");
        restore(mask);
        return SYSERR;
    }

    // if reader is disconnected, stop writting and clean up
    if(pipe->state==PIPE_OTHER){
    	pipdisconnect((did32)devptr->dvnum);
        restore(mask);
    	return SYSERR;
    }

    // write buf
    pipe->buf[pipe->writerid] = ch;
    pipe->writerid++;
    pipe->writerid%= PIPE_SIZE;
    //kprintf(" put %c\n", pipe->buf[pipe->writerid-1]);


    signal(pipe->readersem);
    //kprintf("count is %d\n", semcount(pipe->readersem));

    if(pipe->state==PIPE_OTHER){
        pipdisconnect((did32)devptr->dvnum);
    }

    restore(mask);
    return OK;
}


