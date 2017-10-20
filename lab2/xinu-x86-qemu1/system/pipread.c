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

    if(pipe->reader != currpid){
        restore(mask);
        return SYSERR;
    }


    for(i=0; i<len; i++){
        // check the state each time
        if(pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER){
            restore(mask);
            return SYSERR;
        }
        // check whether writer disconnect each time, if writer diconnect, semcount<=0 means we can not read anything
        if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <=0){
            pipdisconnect((did32)devptr->dvnum);
            restore(mask);
            return i;
        }
    	wait(pipe->readersem);

        if((pipe->state!=PIPE_CONNECTED && pipe->state!=PIPE_OTHER)){
            restore(mask);
            return i;
        }

        //this is valid because we only have one reader, if count<0 means there is nothing to read.
        if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <0){
            pipdisconnect((did32)devptr->dvnum);
            restore(mask);
            return i;
        }

    	buf[i] = pipe->buf[pipe->readerid];
    	pipe->readerid++;
    	pipe->readerid %= PIPE_SIZE;

    	signal(pipe->writersem);
    }

    //if(pipe->state == PIPE_OTHER && semcount(pipe->readersem) <0){
        //pipdisconnect((did32)devptr->dvnum);
    //}


    restore(mask);
	return i;
}
