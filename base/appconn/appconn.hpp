/*
 * appconn.hpp
 */
#ifndef APPCONN_HPP
#define	APPCONN_HPP

#include "conn.hpp"

/**
 * <p>The AppConn class should be used/extended to define application connection behavior.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class AppConn {

public:

    /* constants */
    static int DEFAULT_PORT;

protected:

    /** The localId. */
    int localId;

    /** The remoteId. */
    int remoteId;

    /** The taskSet. */
    TaskSet *taskSet;

private:

    /** The connMap. */
    std::map<short, Conn*> connMap;
    boost::mutex connMapMutex;
    std::map<short, Conn*>::iterator connIterator;

protected:

    /**
     * Creates a new instance of AppConn.
     * @param &localId const int
     * @param &remoteId const int
     * @param *taskSet TaskSet
     */
    AppConn(const int &localId, const int &remoteId, TaskSet *taskSet) {
        this->localId=localId;
        this->remoteId=remoteId;
        if(taskSet==NULL) throw Exception("null taskSet", __FILE__, __LINE__);
        this->taskSet=taskSet;
    }

    /**
     * Adds the conn.
     * @param *conn Conn
     */
    void add(Conn *conn) {
        if(conn!=NULL) {
            boost::lock_guard<boost::mutex> lock(connMapMutex);
            std::map<short, Conn*>::iterator iter=connMap.find(conn->getIndex());
            if(iter!=connMap.end()) {
                delete iter->second;
                connMap.erase(iter);
            }
            connMap.insert(std::pair<short, Conn*>(conn->getIndex(), conn));
            connIterator=connMap.begin();
        }
    }

    /**
     * Returns the next conn with getState() ONLINE, otherwise null.
     * @return *Conn
     */
    Conn* next() {
        short connIndex=0;
        while(connIndex < size()) {
            boost::lock_guard<boost::mutex> lock(connMapMutex);
            Conn *tmpConn=NULL;
            if(connIterator==connMap.end()) {
                connIterator=connMap.begin();
                if(connIterator!=connMap.end()) tmpConn=connIterator->second;
            }
            else tmpConn=connIterator->second;
            connIterator++;
            if(tmpConn!=NULL && tmpConn->getState()==Conn::ONLINE) return tmpConn;
            connIndex++;
        }
        return NULL;
    }

    /**
     * Removes the conn.
     * @param *conn Conn
     */
    void remove(Conn *conn) {
        if(conn!=NULL) {
            boost::lock_guard<boost::mutex> lock(connMapMutex);
            std::map<short, Conn*>::iterator iter=connMap.find(conn->getIndex());
            if(iter!=connMap.end()) {
                delete iter->second;
                connMap.erase(iter);
            }
            connIterator=connMap.begin();
        }
    }

    /**
     * Returns the vector.
     * @return std::vector<Conn*>
     */
    std::vector<Conn*> getVector() {
        boost::lock_guard<boost::mutex> lock(connMapMutex);
        std::vector<Conn*> connVector;
        for(std::map<short, Conn*>::iterator iter=connMap.begin(); iter!=connMap.end(); iter++)
            connVector.push_back(iter->second);
        return connVector;
    }

    /**
     * Returns the size.
     * @return int
     */
    int size() {
        boost::lock_guard<boost::mutex> lock(connMapMutex);
        return connMap.size();
    }

public:

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
};

int AppConn::DEFAULT_PORT=8482;

#endif	/* APPCONN_HPP */
