#include <xinu.h>

// this is the transformation between pipid and did
pipid32 did32_to_pipid32(did32 devpipe) {
    //ASSERT(devpipe >= PIPELINE0 && devpipe <= PIPELINE9);
    return devpipe - PIPELINE0;
}

did32 pipid32_to_did32(pipid32 pip) {
    //ASSERT(pip >= 0 && pip <= 9);
    return PIPELINE0 + pip;
}

static pipid32 newpip(void){
    pipid32 pipid;
    int32 i;


    // go over all table to find available pipe
    for (i=0; i<MAXPIPES; i++ ){
        pipid = i;
        if (pipe_tables[pipid].state == PIPE_FREE){
            return pipid;
        }
    }

    return SYSERR;
}

did32 pipcreate() {
    intmask mask;   // save interrupt mask
    pipid32 pipid;  // pipid id for the pipe
    did32 did;      // did to return
    struct pipe_t *pipe; // pointer to the pipe table entry

    mask = disable();

    if((pipid=newpip()) == SYSERR){
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];
    // change the state
    pipe->state = PIPE_USED;

    // init
    pipe->owner = currpid;
   	pipe->writer = -1;
    pipe->reader = -1;
    pipe->prevwriter = -1;
    pipe->prevreader = -1;
    pipe->writerid = 0;
    pipe->readerid = 0;
    pipe->writersem = semcreate(PIPE_SIZE);
    pipe->readersem = semcreate(0);


    did=pipid32_to_did32(pipid);

    restore(mask);

    return did;
}
