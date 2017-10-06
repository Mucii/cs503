#include <xinu.h>

status pipdelete(did32 devpipe) {
    intmastk mask;   // interrupt mask
    struct pipe_t *pipe;  // pipe for the table entry
    pipid32 pipid;

    mask=disbale();
    
    pipid = did32_to_pipid32(devpipe);



    // if bad pipid
    if(badpipid(pipid)){
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid]; 

    //if is not the the owner of the pipe
    if(currpid != pipe->owner){
        restore(mask);
        return SYSERR;
    }

    // if already free
    if(pipe->state == PIPE_FREE){
    	restore(mask);
    	return OK;
    }

    pipe->state = PIPE_FREE;

    pipe->writer = -1;
    pipe->reader = -1;
    pipe->writerid = 0;
    pipe->readerid = 0;
    semdelete(pipe->writersem);
    semdelete(pipe->readersem);


    return OK;
}
