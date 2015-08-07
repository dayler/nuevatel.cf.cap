/*
 * conn.hpp
 */
#ifndef CONN_HPP
#define	CONN_HPP

#include "appmessages.hpp"
#include "../exception.hpp"
#include "../executor.hpp"
#include "../logger.hpp"
#include "../socket.hpp"
#include "../thread.hpp"
#include "../timer.hpp"
#include "message.hpp"
#include <boost/thread/mutex.hpp>

using namespace logging;

class Conn;

/**
 * <p>The Task abstract class should be extended to define tasks for async or sync call messages.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Task {

public:

    virtual ~Task() {}

    /**
     * Processes async messages or computes a ret for sync call messages, or throws an exception.
     * @param *conn Conn
     * @param *msg Message
     * @return *Message
     * @throws Exception
     */
    virtual Message* execute(Conn *conn, Message *msg) throw(Exception)=0;
};

/**
 * <p>The TaskSet class should be used to map tasks related to incoming async or sync call messages.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class TaskSet {

    /** The taskMap. */
    std::map<int, Task*> taskMap;

public:

    /**
     * Creates a new instance of TaskSet.
     */
    TaskSet() {}

    /**
     * Adds the task.
     * @param *code const int
     * @param *t Task
     */
    void add(const int &code, Task *t) {
        if(code < 0 || code > 0xff) {
            std::stringstream ss;
            ss << "illegal code " << code;
            throw Exception(ss.str(), __FILE__, __LINE__);
        }
        taskMap.erase(code);
        taskMap.insert(std::pair<int, Task*>(code, t));
    }

    /**
     * Returns the task for the given code.
     * @param &code const int
     * @return *Task
     */
    Task* get(const int &code) {
        std::map<int, Task*>::iterator iter=taskMap.find(code);
        if(iter!=taskMap.end()) return iter->second;
        else return NULL;
    }

    /**
     * Removes the task for the given code.
     * @param &code const int
     */
    void remove(const int &code) {
        taskMap.erase(code);
    }
};

/**
 * <p>The Conn abstract class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Conn : public Thread {

public:

    /* constants for state */
    static char OFFLINE;
    static char ONLINE;

protected:

    /** The localId. */
    int localId;

    /** The remoteId. */
    int remoteId;

    /** The index. */
    short index;

    /** The state. */
    char state;
    boost::mutex stateMutex;

private:

    /** The taskSet. */
    TaskSet *taskSet;

protected:

    /** registrable. */
    bool registrable;

private:

    class Dispatcher;

    /** The dispatcherMap. */
    std::map<int, Dispatcher*> dispatcherMap;
    boost::mutex dispatcherMapMutex;

    /** The tmpMessageId. */
    int tmpMessageId;
    boost::mutex tmpMessageIdMutex;

protected:

    /**
     * Creates a new instance of Conn.
     * @param &localId const int
     * @param &remoteId const int
     * @param &index const short
     * @param *taskSet TaskSet
     * @param &registrable const bool
     */
    Conn(const int &localId, const int &remoteId, const short &index, TaskSet *taskSet, const bool &registrable) {
        this->localId=localId;
        this->remoteId=remoteId;
        this->index=index;
        state=0;
        this->taskSet=taskSet;
        this->registrable=registrable;
        tmpMessageId=0;
    }

public:

    virtual ~Conn() {}

    /**
     * Interrupts this.
     */
    virtual void interrupt()=0;

protected:

    /**
     * Writes a msg through the connection.
     * @param *msg Message
     * @throws Exception
     */
    virtual void write(Message *msg) throw(Exception)=0;

    /**
     * Registers this.
     * @return byte
     */
    virtual char reg()=0;

    /**
     * Schedules the echo.
     */
    virtual void scheduleEcho()=0;

private:

    /**
     * Sets the ret.
     * @param *ret Message
     */
    void setRet(Message *ret) {
        boost::lock_guard<boost::mutex> lock(dispatcherMapMutex);
        std::map<int, Dispatcher*>::iterator iter=dispatcherMap.find(ret->getId());
        if(iter!=dispatcherMap.end()) {
            Dispatcher *d=iter->second;
            if(d->wait) d->setRet(ret);
        }
    }

    /**
     * Handles the given msg.
     * @param *msg Message
     * @throws Exception
     */
    void handle(Message *msg) throw(Exception) {
        // async or sync call
        if(msg->isAsync() || msg->isCall()) {
            Task *task=taskSet->get(msg->getCode());
            if(task!=NULL) {
                // sync call
                if(msg->isSync() && msg->isCall()) {
                    boost::system_time timeoutTime=boost::get_system_time() + boost::posix_time::seconds(msg->getTimeToLive());
                    int id=msg->getId();
                    Message *ret=task->execute(this, msg);
                    if(boost::get_system_time() < timeoutTime) {
                        if(ret!=NULL) {
                            ret->setId(id);
                            try {
                                write(ret);
                            } catch(Exception e) {
                                delete ret;
                                throw e;
                            }
                        }
                        else {
                            std::stringstream ss;
                            ss << "null ret code " << msg->getCode();
                            Logger::getLogger()->logp(&Level::WARNING, "Conn", "handle", ss.str());
                        }
                    }
                    else {
                        std::stringstream ss;
                        ss << "task.execute timeout id " << remoteId << " code " << msg->getCode();
                        Logger::getLogger()->logp(&Level::WARNING, "Conn", "handle", ss.str());
                    }
                    if(ret!=NULL) delete ret;
                }
                else task->execute(this, msg);
            }
            else {
                std::stringstream ss;
                ss << "null task code " << msg->getCode();
                Logger::getLogger()->logp(&Level::WARNING, "Conn", "handle", ss.str());
            }
        }
        // sync ret
        else if(msg->isSync() && msg->isRet()) setRet(dynamic_cast<Message*>(msg->clone()));
    }

public:

    /**
     * Dispatches the given msg, returns the ret message for sync call messages.
     * @param *msg Message
     * @return *Message
     * @throws Exception
     */
    Message* dispatch(Message *msg) throw(Exception) {
        Dispatcher d(this, msg);
        return d.call();
    }

    /**
     * Returns the localId.
     * @return int
     */
    int getLocalId() {
        return localId;
    }

    /**
     * Returns the remoteId.
     * @return int
     */
    int getRemoteId() {
        return remoteId;
    }

    /**
     * Returns the index.
     * @return short
     */
    short getIndex() {
        return index;
    }

protected:

    /**
     * Sets the state.
     * @param state char
     */
    void setState(char state) {
        boost::lock_guard<boost::mutex> lock(stateMutex);
        this->state=state;
    }

public:

    /**
     * Returns the state.
     * @return char
     */
    char getState() {
        boost::lock_guard<boost::mutex> lock(stateMutex);
        return state;
    }

private:

    /**
     * Inserts the dispatcher.
     * @param &id const int
     * @param *dispatcher Dispatcher
     */
    void insert(const int &id, Dispatcher *dispatcher) {
        boost::lock_guard<boost::mutex> lock(dispatcherMapMutex);
        dispatcherMap.erase(id);
        dispatcherMap.insert(std::pair<int, Dispatcher*>(id, dispatcher));
    }

    /**
     * Erases the dispatcher.
     * @param &id const int
     */
    void erase(const int &id) {
        boost::lock_guard<boost::mutex> lock(dispatcherMapMutex);
        dispatcherMap.erase(id);
    }

    /**
     * Returns the tmpMessageId.
     * @return int
     */
    int getTmpMessageId() {
        boost::lock_guard<boost::mutex> lock(tmpMessageIdMutex);
        int tmp=tmpMessageId++;
        if(tmpMessageId==INT_MAX) tmpMessageId=0;
        return tmp;
    }

protected:

    /**
     * The Handler class.
     */
    class Handler : public Thread {

        /** The conn. */
        Conn *conn;

        /* private variables */
        char version;
        char type;
        int id;
        short timeToLive;
        int code;
        char *value;
        int length;

    public:

        /**
         * Creates a new instance of Handler.
         * @param *conn Conn
         * @param &version const char
         * @param &type const char
         * @param &id const int
         * @param &timeToLive const short
         * @param &code const int
         * @param *value const char
         * @param &length const int
         */
        Handler(Conn *conn, const char &version, const char &type, const int &id, const short &timeToLive, const int &code, const char *value, const int &length) {
            this->conn=conn;
            this->version=version;
            this->type=type;
            this->id=id;
            this->timeToLive=timeToLive;
            this->code=code;
            this->value=new char[length];
            for(int i=0; i < length; i++) this->value[i]=value[i];
            this->length=length;
        }

        ~Handler() {
            delete[] value;
        }

    private:

        void run() {
            try {
                Message msg(version, type, id, timeToLive, code, value, length);
                conn->handle(&msg);
            } catch(Exception e) {
                Logger::getLogger()->logp(&Level::WARNING, "Handler", "run", e.toString());
            }
        }
    };

private:

    /**
     * The Dispatcher class.
     */
    class Dispatcher {

        /** The conn. */
        Conn *conn;

        /** The msg. */
        Message *msg;

        /** The ret. */
        Message *ret;

        /** The retCond. */
        boost::condition_variable retCond;

        /** The retMutex. */
        boost::mutex retMutex;

    public:

        /* private variables */
        bool wait;

        /**
         * Creates a new instance of Dispatcher.
         * @param *conn Conn
         * @param *msg Message
         */
        Dispatcher(Conn *conn, Message *msg) {
            this->conn=conn;
            this->msg=msg;
            ret=NULL;
            wait=false;
        }

        Message* call() throw(Exception) {
            if(msg->isSync() && msg->isCall()) {
                int tmpMessageId=conn->getTmpMessageId();
                msg->setId(tmpMessageId);
                try {
                    // wait
                    wait=true;
                    conn->insert(tmpMessageId, this);
                    conn->write(msg);
                    {
                        boost::unique_lock<boost::mutex> lock(retMutex);
                        boost::system_time timeoutTime=boost::get_system_time() + boost::posix_time::seconds(msg->getTimeToLive());
                        while(wait && boost::get_system_time() < timeoutTime) retCond.timed_wait(lock, timeoutTime);
                    }
                    wait=false;
                    conn->erase(tmpMessageId);
                } catch(Exception e) {
                    wait=false;
                    conn->erase(tmpMessageId);
                    throw e;
                }
                if(ret==NULL) {
                    std::stringstream ss;
                    ss << "sync call timeout id " << conn->getRemoteId() << " code " << msg->getCode();
                    throw Exception(ss.str(), __FILE__, __LINE__);
                }
            }
            else conn->write(msg);
            return ret;
        }

        /**
         * Sets the ret.
         * @param *ret Message
         */
        void setRet(Message *ret) {
            {
                boost::lock_guard<boost::mutex> lock(retMutex);
                this->ret=ret;
                wait=false;
            }
            if(ret!=NULL) retCond.notify_one();
        }
    };
};

char Conn::OFFLINE=0;
char Conn::ONLINE=1;

/**
 * <p>The AbstractConn abstract class should be extended to implement write and run methods.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class AbstractConn : public Conn {

    /* constants */
    static int MAX_FAILED_ECHO;
    static int ECHO_PERIOD;

    /** The address. */
    std::string address;

    /** The port. */
    int port;

protected:

    /** The socket. */
    Socket *socket;

    /** The handleService. */
    Executor *handleService;

private:

    /* private variables */
    Timer *echoTimer;
    TimerTask *echoTimerTask;
    int failedEcho;

    /** The interruptMutex. */
    boost::mutex interruptMutex;

protected:

    /**
     * Creates a new instance of AbstractConn.
     * @param &localId const int
     * @param &remoteId const int
     * @param &index const short
     * @param *taskSet TaskSet
     * @param &registrable const bool
     * @param &address const std::string
     * @param &port const int
     */
    AbstractConn(const int &localId, const int &remoteId, const short &index, TaskSet *taskSet, const bool &registrable, const std::string &address, const int &port) : Conn(localId, remoteId, index, taskSet, registrable) {
        this->address=address;
        this->port=port;
        socket=NULL;
        handleService=NULL;
        echoTimer=NULL;
        echoTimerTask=NULL;
    }

public:

    virtual ~AbstractConn() {
        interrupt();
    }

    void start() {
        try {
            socket=new Socket(address, port);
            handleService=new Executor();
            Thread::start();
            if(registrable) {
                if(reg()==AppMessages::ACCEPTED) {
                    scheduleEcho();
                    setState(ONLINE);
                    std::stringstream ss;
                    ss << "register to id " << remoteId << " index " << index << " accepted";
                    Logger::getLogger()->logp(&Level::FINE, "AbstractConn", "start", ss.str());
                }
                else {
                    interrupt();
                    std::stringstream ss;
                    ss << "register to id " << remoteId << " index " << index << " failed";
                    Logger::getLogger()->logp(&Level::WARNING, "AbstractConn", "start", ss.str());
                }
            }
            else setState(ONLINE);
        } catch(SocketException se) {
            interrupt();
            Logger::getLogger()->logp(&Level::FINE, "AbstractConn", "start", se.toString());
        }
    }

    void interrupt() {
        boost::lock_guard<boost::mutex> lock(interruptMutex);
        setState(OFFLINE);
        if(echoTimer!=NULL) {
            echoTimer->cancel();
            delete echoTimer;
            echoTimer=NULL;
        }
        if(echoTimerTask!=NULL) {
            delete echoTimerTask;
            echoTimerTask=NULL;
        }
        if(handleService!=NULL) {
            delete handleService;
            handleService=NULL;
        }
        if(socket!=NULL) {
            if(socket->isConnected()) socket->close();
            delete socket;
            socket=NULL;
        }
    }

protected:

    char reg() {
        IntIE localIdIE(AppMessages::LOCAL_ID_IE, localId);
        IntIE remoteIdIE(AppMessages::REMOTE_ID_IE, remoteId);
        ShortIE indexIE(AppMessages::INDEX_IE, index);
        std::vector<IE*> ies;
        ies.push_back(&localIdIE);
        ies.push_back(&remoteIdIE);
        ies.push_back(&indexIE);
        Message *registerCall=AppMessages::newCall(AppMessages::REGISTER_CALL, &ies);
        Message *registerRet=NULL;
        try {
            registerRet=dispatch(registerCall);
        } catch(Exception e) {}
        delete registerCall;
        if(registerRet!=NULL) {
            char ret=registerRet->getByte(AppMessages::RET_IE);
            delete registerRet;
            return ret;
        }
        else return AppMessages::FAILED;
    }

    void scheduleEcho() {
        failedEcho=0;
        if(echoTimer!=NULL) {
            echoTimer->cancel();
            delete echoTimer;
        }
        if(echoTimerTask!=NULL) delete echoTimerTask;
        echoTimer=new Timer();
        echoTimerTask=new EchoTimerTask(this);
        echoTimer->scheduleAtFixedRate(echoTimerTask, ECHO_PERIOD, ECHO_PERIOD);
    }

private:

    /**
     * Increments failedEcho.
     */
    void incrementFailedEcho() {
        if(++failedEcho==MAX_FAILED_ECHO) {
            std::stringstream ss;
            ss << "max failed echo id " << remoteId;
            Logger::getLogger()->logp(&Level::WARNING, "AbstractConn", "failedEcho", ss.str());
            interrupt();
        }
    }

    /**
     * The EchoTimerTask class.
     */
    class EchoTimerTask : public TimerTask {

        /** The abstractConn. */
        AbstractConn *abstractConn;

    public:

        /**
         * Creates a new instance of EchoTimerTask.
         * @param *abstractConn AbstractConn
         */
        EchoTimerTask(AbstractConn *abstractConn) {
            this->abstractConn=abstractConn;
        }

    private:

        void run() {
            Message *echoCall=AppMessages::newCall(AppMessages::ECHO_CALL, NULL);
            Message *echoRet=NULL;
            try {
                echoRet=abstractConn->dispatch(echoCall);
            } catch(Exception e) {}
            delete echoCall;
            if(echoRet!=NULL) delete echoRet;
            else abstractConn->incrementFailedEcho();
        }
    };
};

int AbstractConn::MAX_FAILED_ECHO=2;
int AbstractConn::ECHO_PERIOD=32000;

/**
 * <p>The BinConn class should be used for client connection for binary format.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class BinConn : public AbstractConn {

    /* char pointers */
    char *rch;
    char *wch;

    /** The writeMutex. */
    boost::mutex writeMutex;

public:

    /**
     * Creates a new instance of BinConn.
     * @param &localId const int
     * @param &remoteId const int
     * @param &index const short
     * @param *taskSet TaskSet
     * @param &registrable const bool
     * @param &address const std::string
     * @param &port const int
     */
    BinConn(const int &localId, const int &remoteId, const short &index, TaskSet *taskSet, const bool &registrable, const std::string &address, const int &port) : AbstractConn(localId, remoteId, index, taskSet, registrable, address, port) {
        rch=new char[65535];
        wch=new char[65545];
    }

    ~BinConn() {
        delete[] rch;
        delete[] wch;
    }

private:

    void write(Message *msg) throw(Exception) {
        boost::lock_guard<boost::mutex> lock(writeMutex);
        if(socket!=NULL) {
            int len;
            msg->getBytes(wch, len);
            socket->write(wch, len);
        }
    }

    void run() {
        try {
            char tmp[10];
            while(socket!=NULL && socket->isConnected()) {
                read(socket, tmp, 0, 2);
                // version
                char tmpVersion=tmp[0];
                if(tmpVersion!=AppMessages::VERSION) {
                    std::stringstream ss;
                    ss << "illegal version " << (int)tmpVersion;
                    Logger::getLogger()->logp(&Level::WARNING, "BinConn", "run", ss.str());
                    break;
                }

                char tmpType=tmp[1] & (Message::IO_TYPE | Message::MESSAGE_TYPE);
                char tmpLLength=tmp[1] & Message::L_LENGTH;
                int tmpIndex=2;
                // id
                int tmpId=0;
                short tmpTimeToLive=0;
                if((tmpType & Message::IO_TYPE)==Message::SYNC) {
                    read(socket, tmp, tmpIndex, 4);
                    tmpId=(tmp[2] & 0xff); tmpId<<=8;
                    tmpId+=(tmp[3] & 0xff); tmpId<<=8;
                    tmpId+=(tmp[4] & 0xff); tmpId<<=8;
                    tmpId+=tmp[5] & 0xff;
                    tmpIndex=6;
                    // timeToLive
                    if((tmpType & Message::MESSAGE_TYPE)==Message::CALL) {
                        read(socket, tmp, tmpIndex, 1);
                        tmpTimeToLive=tmp[tmpIndex++] & 0xff;
                    }
                }
                // code
                int tmpCode=0;
                if((tmpType & Message::IO_TYPE)==Message::ASYNC || (tmpType & Message::MESSAGE_TYPE)==Message::CALL) {
                    read(socket, tmp, tmpIndex, 1);
                    tmpCode=tmp[tmpIndex++] & 0xff;
                }

                int tmpLength=0;
                if(tmpLLength==Message::L_LENGTH_0) tmpLength=0;
                else if(tmpLLength==Message::L_LENGTH_1) {
                    read(socket, tmp, tmpIndex, 1);
                    tmpLength=tmp[tmpIndex] & 0xff;
                }
                else if(tmpLLength==Message::L_LENGTH_2) {
                    read(socket, tmp, tmpIndex, 2);
                    tmpLength=tmp[tmpIndex] & 0xff; tmpLength<<=8;
                    tmpLength+=tmp[tmpIndex + 1] & 0xff;
                }

                read(socket, rch, 0, tmpLength);

                // handle
                handleService->submit(new Handler(this, tmpVersion, tmpType, tmpId, tmpTimeToLive, tmpCode, rch, tmpLength));
            }
        } catch(Exception e) {
            Logger::getLogger()->logp(&Level::WARNING, "BinConn", "run", e.toString());
        }
        interrupt();
    }

    /**
     * Reads the socket.
     * @param *socket Socket
     * @param *b char
     * @param &off const int
     * @param &len const int
     */
    void read(Socket *socket, char *b, const int &off, const int &len) throw(Exception) {
        int i=0;
        while(i < len) {
            int tmp=socket->read(b, off + i, len - i);
            if(tmp==-1) throw Exception("end of the stream", __FILE__, __LINE__);
            else i+=tmp;
        }
    }
};

#endif	/* CONN_HPP */
