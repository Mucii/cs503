/* pipe.h: contains all the macros and definition for the pipe type in XINU */

#define MAXPIPES 10
#define PIPE_SIZE 1024


#define	PIPE_FREE 0
#define	PIPE_USED 1
#define PIPE_CONNECTED 2
#define PIPE_OTHER 3


#define isbadpipid(pipid)( (pipid < 0) || \
        (pipid >= MAXPIPES))

struct pipe_t {
	pipid32 pipid;			    // Pipe ID
	int state;                  // Pipe state defined by the enum
    pid32 owner;                // process create the pipe
    pid32 writer;               // the id of writer process
    pid32 reader;				// the id of reader process
    char buf[PIPE_SIZE];        // buffer for the pipe
    int writerid;               // writer index of the buffer
    int readerid;               // reader index of the buffer
    sid32 writersem;            // semaphore for the writer
    sid32 readersem;             // semaphore for rhe reader           
};

extern struct pipe_t pipe_tables[MAXPIPES];	// Table for all pipes
