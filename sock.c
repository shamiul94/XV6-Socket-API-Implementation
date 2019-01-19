#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sock.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"

int currPort = 0;

struct {
    struct spinlock lock;
    Socket sock[NSOCK];
} stable;

int retExit(int code) {
    release(&stable.lock); // not sure
    return code;
}

void
sinit(void) {
    initlock(&stable.lock, "stable");
    Socket *ps;

    for (ps = stable.sock; ps < &stable.sock[NSOCK]; ps++) {
        ps->state = CLOSED;
    }
}

int isAlreadyOpenOnThisPort(int lport) {
    Socket *ps;
    int alreadyOpened = 0;

    for (ps = stable.sock; ps < &stable.sock[NSOCK]; ps++) {
        if (ps->state != CLOSED && ps->localPort == lport) {
            alreadyOpened = 1;
            break;
        }
    }
    return alreadyOpened;
}


int
listen(int lport) {

    if (lport < 0 || lport >= NPORT) {
        cprintf("Invalid Parameter Error: E_INVALID_ARG\n");
        return retExit(E_INVALID_ARG);
    }

    acquire(&stable.lock); // not sure

    int ret = -1;

    // if another socket is already opened on this port
    if (isAlreadyOpenOnThisPort(lport)) {
        goto EXIT;
    }

    Socket *ps, *pLocalSocket; // p = pointer

    int freePortFound = 0;
    for (ps = stable.sock; ps < &stable.sock[NSOCK]; ps++) {
        if (ps->state == CLOSED) {
            freePortFound = 1;
            pLocalSocket = ps;
            break;
        }
    }

    // if every socket is taken by other ports and can't be assigned to this port.
    if (!freePortFound) {
        cprintf("No more socket can be opened (Limit exceeded) Error.\n");
        return retExit(E_FAIL);
    }


    pLocalSocket->state = LISTENING;
    pLocalSocket->localPort = lport;
    pLocalSocket->remotePort = -1; // dummy value
    pLocalSocket->ownerProcessID = myproc()->pid;
    pLocalSocket->hasData = 0;

    sleep(pLocalSocket, &stable.lock);


    if (pLocalSocket->state == CONNECTED) {
        ret = 0;
    } else {
        ret = -1;
    }

    EXIT:
    release(&stable.lock); // not sure
    return ret;
}


int
connect(int rport, const char *host) {

    if (rport < 0 || rport >= NPORT) {
        cprintf("Invalid Parameter Error: E_INVALID_ARG\n");
        return retExit(E_INVALID_ARG);
    }

    if (!(strncmp(host, "localhost", strlen(host)) == 0 || strncmp(host, "127.0.0.1", strlen(host)) == 0)) {
        cprintf("Invalid Parameter Error: E_INVALID_ARG\n");
        return retExit(E_INVALID_ARG);
    }

    int ret = -1;

    acquire(&stable.lock);

    Socket *ps, *pRemoteSocket = 0, *pLocalSocket;


/**************** Confirm Remote Socket (Server) ***************/

    int remotePortFound = 0;

    for (ps = stable.sock; ps < &stable.sock[NSOCK]; ps++) {
        if (ps->state == LISTENING && ps->localPort == rport) {
            remotePortFound = 1;
            pRemoteSocket = ps;
            break;
        }
    }


    if (!remotePortFound) {
        cprintf("Socket not found at Port Error.\n");
        return retExit(E_NOTFOUND);
    }


/***********Confirm availability of Local Socket (Client) *************/

    int lport = (currPort + 1) % NPORT;
    currPort++;

    while (isAlreadyOpenOnThisPort(lport)) {
        lport = (currPort + 1) % NPORT;
        currPort++;
    }

    int freePortFound = 0;
    for (ps = stable.sock; ps < &stable.sock[NSOCK]; ps++) {
        if (ps->state == CLOSED) {
            freePortFound = 1;
            pLocalSocket = ps;
            break;
        }
    }

    if (!freePortFound) {
        cprintf("No more socket can be opened (Limit exceeded) Error.\n");
        return retExit(E_FAIL);
    }


    pLocalSocket->state = CONNECTED;
    pRemoteSocket->state = CONNECTED; // attribute of remote socket

    pLocalSocket->localPort = lport;
    pLocalSocket->remotePort = rport;

    pRemoteSocket->remotePort = lport; // attribute of remote socket

    pLocalSocket->ownerProcessID = myproc()->pid;
    pLocalSocket->hasData = 0;
    pRemoteSocket->hasData = 0;

    wakeup(pRemoteSocket); // have to check

    ret = lport;
    cprintf("##########################################################################\n");
    cprintf("#  Client opened at Port: %d and it's connected with Server at Port: %d  #\n", pLocalSocket->localPort,
            pRemoteSocket->localPort);
    cprintf("##########################################################################\n");


//    EXIT:
    release(&stable.lock);
    return ret;
}

Socket *getSocketByPort(int lport) {
    Socket *ps, *sock = 0;

    for (ps = stable.sock; ps < &stable.sock[NSOCK]; ps++) {
        if (ps->localPort == lport) {
            sock = ps;
            break;
        }
    }
    return sock;
}

int
send(int lport, const char *data, int n) {

    if (lport < 0 || lport >= NPORT) {
        cprintf("Invalid Parameter Error: E_INVALID_ARG\n");
        return retExit(E_INVALID_ARG);
    }

    if (n <= 0 || n > MAX_BUFFER_SIZE) {
        cprintf("Invalid Parameter Error: E_INVALID_ARG\n");
        return retExit(E_INVALID_ARG);
    }

    int ret = -1;
    acquire(&stable.lock);
    Socket *pRemoteSocket, *pLocalSocket;

    pLocalSocket = getSocketByPort(lport);

    if (!pLocalSocket) {
        cprintf("Socket not found at Port Error.\n");
        return retExit(E_NOTFOUND);
    }

    if (pLocalSocket->ownerProcessID != myproc()->pid) {
        cprintf("Access denied from foreign process Error: PID = %d\n", myproc()->pid);
        return retExit(E_ACCESS_DENIED);

    }
    // accessed by another process.

    pRemoteSocket = getSocketByPort(pLocalSocket->remotePort);

    if (!pRemoteSocket) {
        cprintf("Socket not found at Port Error.\n");
        return retExit(E_NOTFOUND);
    }


    if (pRemoteSocket->state != CONNECTED || pLocalSocket->state != CONNECTED) {
        cprintf("Local socket port: %d and Remote socket Port: %d aren't connected Error.\n", pLocalSocket->localPort,
                pRemoteSocket->localPort);
        return retExit(E_WRONG_STATE);
    }


    while (pRemoteSocket->hasData) {
        sleep(pRemoteSocket, &stable.lock);
    }

    strncpy(pRemoteSocket->buffer, data, n);
    pRemoteSocket->hasData = 1;
    ret = 0;

    wakeup(pLocalSocket); // check later***

//    EXIT:
    release(&stable.lock);
    return ret;
}


int
recv(int lport, char *data, int n) {

    if (lport < 0 || lport >= NPORT) {
        cprintf("Invalid Parameter Error: E_INVALID_ARG\n");
        return retExit(E_INVALID_ARG);
    }
    if (n <= 0 || n > MAX_BUFFER_SIZE) {
        cprintf("Invalid Parameter Error: E_INVALID_ARG\n");
        return retExit(E_INVALID_ARG);
    }

    int ret = -1;
    acquire(&stable.lock);

    Socket *pRemoteSocket, *pLocalSocket;

    pLocalSocket = getSocketByPort(lport);

    if (!pLocalSocket) {
        cprintf("Socket not found at Port Error.\n");
        return retExit(E_NOTFOUND);
    } // no socket on this port


    if (pLocalSocket->ownerProcessID != myproc()->pid) {
        cprintf("Access denied from foreign process Error: PID = %d\n", myproc()->pid);
        return retExit(E_ACCESS_DENIED);
    }

    pRemoteSocket = getSocketByPort(pLocalSocket->remotePort);

    if (!pRemoteSocket) {
        cprintf("Socket not found at Port Error.\n");
        return retExit(E_NOTFOUND);
    }

    if (pRemoteSocket->state != CONNECTED || pLocalSocket->state != CONNECTED) {
        cprintf("Local socket port: %d and Remote socket Port: %d aren't connected Error.\n", pLocalSocket->localPort,
                pRemoteSocket->localPort);
        return retExit(E_WRONG_STATE);
    }

    while (!pLocalSocket->hasData) {
        sleep(pRemoteSocket, &stable.lock);
    }

    strncpy(data, pLocalSocket->buffer, n);

    pLocalSocket->hasData = 0;

    wakeup(pLocalSocket); // check later***

    ret = 0;

//    EXIT:
    release(&stable.lock);
    return ret;
}

int
disconnect(int lport) {
    if (lport < 0 || lport >= NPORT) {
        cprintf("Invalid Parameter Error: E_INVALID_ARG\n");
        return retExit(E_INVALID_ARG);
    }

    acquire(&stable.lock);

    Socket *pLocalSocket;

    pLocalSocket = getSocketByPort(lport);

    if (!pLocalSocket) {
        cprintf("Socket not found at Port Error.\n");
        return retExit(E_NOTFOUND);
    }

    pLocalSocket->state = CLOSED;
    pLocalSocket->localPort = -1;
    pLocalSocket->remotePort = -1;
    pLocalSocket->hasData = 0;

    release(&stable.lock);

    return 0;
}
