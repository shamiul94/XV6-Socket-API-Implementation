#define NPORT       128  // maximum number of ports
#define NSOCK        32  // maximum number of sockets
#define MAX_BUFFER_SIZE 128
#define NPROC        64  // maximum number of processes
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       1000  // size of file system in blocks

#define 	E_NOTFOUND 			-1025
#define 	E_ACCESS_DENIED 	-1026
#define 	E_WRONG_STATE 		-1027
#define 	E_FAIL 				-1028
#define 	E_INVALID_ARG 		-1029

