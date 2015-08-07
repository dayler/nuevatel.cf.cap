/*
 * appclient.hpp
 */
#ifndef APPCLIENT_HPP
#define	APPCLIENT_HPP

#include "appconn.hpp"
#include "../properties.hpp"

/**
 * <p>The AppClient class should be used for the application client.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class AppClient : public AppConn {

public:

    /* constants for state */
    static char OFFLINE;
    static char ONLINE;
    /* constants for properties */
    static std::string ADDRESS;
    static std::string PORT;
    static std::string INDEX;
    static std::string SIZE;
    static std::string REGISTRABLE;
    static std::string PERSISTENT;

private:

    /* private constants */
    static int CHECK_PERIOD;

    /** The state. */
    char state;
    boost::mutex stateMutex;

    /* properties */
    std::string address;
    int port;
    short index;
    int size;
    bool registrable;
    bool persistent;

    /* private variables */
    Timer *checkTimer;
    TimerTask *checkTimerTask;

    class EchoTask;
    EchoTask *echoTask;

public:

    /**
     * Creates a new instance of AppClient.
     * @param &localId const int
     * @param &remoteId const int
     * @param *taskSet TaskSet
     * @param *properties Properties
     */
    AppClient(const int &localId, const int &remoteId, TaskSet *taskSet, Properties *properties) : AppConn(localId, remoteId, taskSet) {
        state=0;
        setProperties(properties);
        echoTask=NULL;
        if(registrable) {
            echoTask=new EchoTask();
            this->taskSet->add(AppMessages::ECHO_CALL, echoTask);
        }
        checkTimer=NULL;
        checkTimerTask=NULL;
    }

    ~AppClient() {
        interrupt();
        if(echoTask!=NULL) {
            this->taskSet->remove(AppMessages::ECHO_CALL);
            delete echoTask;
        }
    }

    /**
     * Starts this.
     */
    void start() {
        char tmpState=OFFLINE;
        for(short tmpIndex=index; tmpIndex < (index + size); tmpIndex++) {
            Conn *conn=new BinConn(localId, remoteId, tmpIndex, taskSet, registrable, address, port);
            conn->start();
            if(conn->getState()==Conn::ONLINE) tmpState=ONLINE;
            add(conn);
        }
        setState(tmpState);
        if(persistent) {
            if(checkTimer!=NULL) {
                checkTimer->cancel();
                delete checkTimer;
            }
            if(checkTimerTask!=NULL) delete checkTimerTask;
            checkTimer=new Timer();
            checkTimerTask=new CheckTimerTask(this);
            checkTimer->scheduleAtFixedRate(checkTimerTask, CHECK_PERIOD, CHECK_PERIOD);
        }
    }

    /**
     * Interrupts this.
     */
    void interrupt() {
        setState(OFFLINE);
        if(checkTimer!=NULL) {
            checkTimer->cancel();
            delete checkTimer;
            checkTimer=NULL;
        }
        if(checkTimerTask!=NULL) {
            delete checkTimerTask;
            checkTimerTask=NULL;
        }
        std::vector<Conn*> connVector=getVector();
        for(std::vector<Conn*>::iterator iter=connVector.begin(); iter!=connVector.end(); iter++) {
            Conn *conn=*iter;
            conn->interrupt();
            conn->join();
            remove(conn);
        }
    }

    /**
     * Dispatches the given msg, returns the ret message for sync call messages.
     * @param *msg Message
     * @return *Message
     * @throws Exception
     */
    Message* dispatch(Message *msg) throw(Exception) {
        Conn *conn=next();
        if(conn==NULL) {
            std::stringstream ss;
            ss << "null conn id " << remoteId << " code " << msg->getCode();
            throw Exception(ss.str(), __FILE__, __LINE__);
        }
        return conn->dispatch(msg);
    }

private:

    /**
     * Sets the state.
     * @param &state const char
     */
    void setState(const char &state) {
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
     * Sets the properties.
     * @param *properties Properties
     */
    void setProperties(Properties *properties) {
        if(properties==NULL) throw Exception("null properties", __FILE__, __LINE__);

        // address
        address=properties->getProperty(ADDRESS);
        if(address.length()==0) throw Exception("illegal " + ADDRESS, __FILE__, __LINE__);

        // port
        std::stringstream ss;
        ss << AppConn::DEFAULT_PORT;
        std::string strPort=properties->getProperty(PORT, ss.str());
        if(strPort.length() > 0) port=atoi(strPort.c_str());
        else throw Exception("illegal " + PORT, __FILE__, __LINE__);

        // connIndex
        std::string strIndex=properties->getProperty(INDEX, "0");
        if(strIndex.length() > 0) index=atoi(strIndex.c_str());
        else throw Exception("illegal " + INDEX, __FILE__, __LINE__);

        // size
        std::string strSize=properties->getProperty(SIZE, "8");
        if(strSize.length() > 0) size=atoi(strSize.c_str());
        else throw Exception("illegal " + SIZE, __FILE__, __LINE__);

        // registrable
        std::string strRegistrable=properties->getProperty(REGISTRABLE, "true");
        if(strRegistrable.compare("true")==0) registrable=true;
        else registrable=false;

        // persistent
        std::string strPersistent=properties->getProperty(PERSISTENT, "true");
        if(strPersistent.compare("true")==0) persistent=true;
        else persistent=false;
    }

    /**
     * The EchoTask class.
     */
    class EchoTask : public Task {

    public:

        Message* execute(Conn *conn, Message *msg) throw(Exception) {
            return AppMessages::newRet(NULL);
        }
    };

    /**
     * The CheckTimerTask class.
     */
    class CheckTimerTask : public TimerTask {

        /** The appClient. */
        AppClient *appClient;

    public:

        /**
         * Creates a new instance of CheckTimerTask.
         * @param *appClient AppClient
         */
        CheckTimerTask(AppClient *appClient) {
            this->appClient=appClient;
        }

    private:

        void run() {
            char tmpState=OFFLINE;
            std::vector<Conn*> connVector=appClient->getVector();
            for(std::vector<Conn*>::iterator iter=connVector.begin(); iter!=connVector.end(); iter++) {
                Conn *conn=*iter;
                if(conn->getState()!=Conn::ONLINE) {
                    conn->start();
                    if(conn->getState()==Conn::ONLINE) tmpState=ONLINE;
                }
                else tmpState=ONLINE;
            }
            if(appClient->getState()!=tmpState) appClient->setState(tmpState);
        }
    };
};

char AppClient::OFFLINE=0;
char AppClient::ONLINE=1;

std::string AppClient::ADDRESS="address";
std::string AppClient::PORT="port";
std::string AppClient::INDEX="index";
std::string AppClient::SIZE="size";
std::string AppClient::REGISTRABLE="registrable";
std::string AppClient::PERSISTENT="persistent";

int AppClient::CHECK_PERIOD=8000;

#endif	/* APPCLIENT_HPP */
