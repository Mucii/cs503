#include <xinu.h>

uint32 pipwrite(struct dentry *devptr, char* buf, uint32 len) {
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

    for(uint32 i = 0; i< len; i++){
    	wait(pipe-writersem);

    	// check the status each time
    	if(pipe->state!=PIPE_IPE_CONNECTED){
    		if(pipe->state==PIPE_OTHER){
    			pipedisconnect((did32)devptr->dvnum);
    			restore(mask);
    			return i;
    		}else{
    			restore(mask);
                cleanup(pipid);
    			return i;
    		}
    	}

    	pipe->buf[pipe->writerid] = buf[i];
    	pipe->writerid++;
    	pipe->writer %= PIPE_SIZE;


    	signal(pipe->readersem);

    }

    if(pipe->state!=PIPE_OTHER){
        pipedisconnect((did32)devptr->dvnum);
    }

    restore(mask);
    return i;
}
