#include <xinu.h>

void cleanup(pipid32 pipid){
   struct pipe_t *pipe;
   pipe = &pipe_tables[pipid];

   pipe->state = PIPE_USED;
   pipe->writer=-1;
   pipe->reader=-1;
   pipe->readerid=0;
   pipe->writerid=0;
   semreset(pipe->writersem,PIPE_SIZE);
   semreset(pipe->readersem,0);
}

status pipdisconnect(did32 devpipe) {
      intmask mask;
   	struct pipe_t *pipe;
      //struct procent *proct;
   	pipid32 pipid;

   	mask = disable();

   	pipid = did32_to_pipid32(devpipe);

   	if(isbadpipid(pipid)){
   		restore(mask);
   		return SYSERR;
   	}

   	pipe = &pipe_tables[pipid];


   	// if currpid is not writer or reader
   	if((currpid != pipe->writer) && (currpid != pipe->reader)){
   		restore(mask);
   		return SYSERR;
   	}

   	//check the state and change it
      // as in piazza we do not have to clean device
   	if(pipe->state == PIPE_OTHER){
   		cleanup(pipid);
         //proct = &proctab[currpid];
         // clean up the device
         /*if(proct->prdesc[0] == devpipe){
            proct->prdesc[0] = -1;
         }else if(proct->prdesc[1] == devpipe){
           */
   	}else if (pipe->state == PIPE_CONNECTED){
         // we do not do clean up when only one side disconnected
   		 pipe->state = PIPE_OTHER;
          //proct = &proctab[currpid];
         // clean up the device
         /*if(proct->prdesc[0] == devpipe){
            proct->prdesc[0] = -1;
         }else if(proct->prdesc[1] == devpipe){
            proct->prdesc[1] = -1;
         }*/
   	}else{
   		restore(mask);
   		return SYSERR;
   	}

   	if(pipe->writer == currpid){
   		// disconnnect writer
   		pipe->writer = -1;
   		// signal the read process
   		if(semcount(pipe->readersem)<0){
   			signal(pipe->readersem);
   		}
   	}else{
   		// disconnnect writer
   		pipe->reader = -1;
   		// signal the read process
   		if(semcount(pipe->writersem)<0){
   			signal(pipe->writersem);
   		}
   	}
   	restore(mask);
   	return OK;
}