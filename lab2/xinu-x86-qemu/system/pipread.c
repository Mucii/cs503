#include <xinu.h>

uint32 pipread(struct dentry *devptr, char* buf, uint32 len) {
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

    for(unit32 i=0; i<len; i++){
    	wait(pipe->readersem);

    	// we should clean up the pipe
    	if(pipe->state!=PIPE_IPE_CONNECTED || pipe->state!=PIPE_OTHER){
    		restore(mask);
    		cleanup(pipid);
    		return i;
    	}	

    	// when nonthing to read and wtier disconnected, clean up
   		if(pipe->state == PIPE_OTHER && semtab[pipe->readersem].scount < 0){
        	pipedisconnect((did32)devptr->dvnum);
        	restore(mask);
        	return i;
    	}

    	buf[i] = pipe->buf[pipe->readerid];
    	pipe->readerid++;
    	pipe->readerid %= PIPE_SIZE;

    	signal(pipe->writersem);
    }

    if(pipe->state == PIPE_OTHER){
    	pipedisconnect((did32)devptr->dvnum);
    }

    restore(mask);
	return OK;
}
