#include <xinu.h>

status pipconnect(did32 devpipe, pid32 writer, pid32 reader) {
    intmask mask;
    struct pipe_t *pipe;
    pipid32 pipid;

    mask = disable();
    pipid = did32_to_pipid32(devpipe);

    //bad pid or pipid or writer == reader
    if(isbadpipid(pipid) || isbadpid(writer) || isbadpid(reader) || writer == reader){
    	restore(mask);
    	return SYSERR;
    }

    pipe = &pipe_tables[pipid];
    //  check the state 
    if(pipe->state != PIPE_USED){
    	restore(mask);
    	return SYSERR;
    }

    pipe->state = PIPE_CONNECTED;
    pipe->writer=writer;
    pipe->reader=reader;
    semreset(pipe->writersem, PIPE_SIZE);
    semreset(pipe->readersem, 0);
    restore(mask);
	return OK;
}

