#include <xinu.h>

uint32 pipwrite(struct dentry *devptr, char* buf, uint32 len) {
    intmask mask;
    pipid32 pipid = did32_to_pipid32((did32)devptr->dvnum);
    struct pipe_t *pipe;
    uint32 i;

    mask =disable();

    if(isbadpipid(pipid)){
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];

    // is not the writer
    if(pipe->writer != currpid){
        restore(mask);
        return SYSERR;
    }

    for(i = 0; i< len; i++){

    // state is not right
    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER)){
        //kprintf("state1\n");
        restore(mask);
        return i;
    }

    // if reader is disconnected, stop writting and clean up
    if(pipe->state==PIPE_OTHER){
        pipdisconnect((did32)devptr->dvnum);
        restore(mask);
        return i;
    }
    	wait(pipe->writersem);

    // state is not right
    if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER)){
        restore(mask);
        return i;
    }

    // if reader is disconnected, stop writting and clean up
    if(pipe->state==PIPE_OTHER){
        pipdisconnect((did32)devptr->dvnum);
        restore(mask);
        return i;
    }

    	pipe->buf[pipe->writerid] = buf[i];
    	pipe->writerid++;
    	pipe->writerid%= PIPE_SIZE;

    	signal(pipe->readersem);

    }

    //if(pipe->state == PIPE_OTHER){
        //pipdisconnect((did32)devptr->dvnum);
    //}

    restore(mask);
    return i;
}
