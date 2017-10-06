#include <xinu.h>

devcall pipgetc(struct dentry *devptr) {
    intmask mask;
    pipid32 pipid = did32_to_pipid32((did32)devptr->dvnum);
    struct pipe_t *pipe;
    char ch;

    if(isbadpipid(pipid)){
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];

    if(pipe->state!=PIPE_IPE_CONNECTED || pipe->state!=PIPE_OTHER || currpid != pipe->reader){
    	restore(mask);
    	return SYSERR;
    }

    wait(pipe->readersem);

    // when nonthing to read and wtier disconnected, clean up
   if(pipe->state == PIPE_OTHER && semtab[pipe->readersem].scount < 0){
        pipedisconnect((did32)devptr->dvnum);
        restore(mask);
        return OK;
    }

    ch = pipe->buf[pipe->readerid];
    pipe->readerid++;
    pipe->readerid %= PIPE_SIZE;

    signal(pipe->writersem);

    if(pipe->state == PIPE_OTHER){
    	pipedisconnect((did32)devptr->dvnum);
    }

    restore(mask);
    return OK;
}

