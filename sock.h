
enum State {
    CLOSED, LISTENING, CONNECTED
};


struct socket {
    enum State state;
    int localPort;
    int remotePort;
    int ownerProcessID;
    char buffer[128];
    int hasData;
};

typedef struct socket Socket;