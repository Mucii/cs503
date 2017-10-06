#include <xinu.h>

devcall pipputc(struct dentry *devptr, char ch) {
    intmask mask;
    pipid32 pipid = did32_to_pipid32((did32)devptr->dvnum);
    struct pipe_t *pipe;

    if(isbadpipid(pipid)){
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];

    if(pipe->state!=PIPE_IPE_CONNECTED || pipe->state!=PIPE_OTHER || currpid != pipe->writer){
    	restore(mask);
    	return SYSERR;
    }

    wait(pipe-writersem);

    // if reader is disconnected, stop writting and clean up
    if(pipe->state!=PIPE_OTHER){
    	pipedisconnect((did32)devptr->dvnum);
        restore(mask);
    	return OK;
    }

    // write buf
    pipe->buf[pipe->writerid] = ch;
    pipe->writerid++;
    pipe->writer %= PIPE_SIZE;


    signal(pipe->readersem);

    if(pipe->state!=PIPE_OTHER){
        pipedisconnect((did32)devptr->dvnum);
    }

    restore(mask);
    return OK;
}


