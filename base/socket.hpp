/*
 * socket.hpp
 */
#ifndef SOCKET_HPP
#define	SOCKET_HPP

#include "exception.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * The SocketException class.
 */
class SocketException : public Exception {

public:

    SocketException() : Exception() {
        this->name="SocketException";
    }

    SocketException(const std::string &message, const std::string &filename, const int &line) : Exception(message, filename, line) {
        this->name="SocketException";
    }
};

/**
 * <p>The Socket class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Socket {

    /** The fd. */
    int fd;

    /** The addressInfo. */
    addrinfo *addressInfo;

public:

    /**
     * Creates an unconnected socket.
     */
    Socket() {
        fd=-1;
    }

    /**
     * Creates a connected socket for the given host and port.
     * @param &host const std::string
     * @param &port const int
     * @throws SocketException
     */
    Socket(const std::string &host, const int &port) throw(SocketException) {
        fd=-1;
        connect(host, port);
    }

    virtual ~Socket() {
        freeaddrinfo(addressInfo);
    }

    /**
     * Connects the socket.
     * @param &host const std::string
     * @param &port const int
     * @throws SocketException
     */
    void connect(const std::string &host, const int &port) throw(SocketException) {
        if(!isConnected()) {
            std::stringstream ss;
            ss << port;
            std::string strPort=ss.str();

            addrinfo hints;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family=AF_UNSPEC;
            hints.ai_socktype=SOCK_STREAM;

            addrinfo *ai;
            if(int rv=getaddrinfo(host.c_str(), strPort.c_str(), &hints, &ai)==0) {
                for(addressInfo=ai; addressInfo!=NULL; addressInfo=ai->ai_next) {
                    if((fd=socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol))!=-1) {
                        if(::connect(fd, addressInfo->ai_addr, addressInfo->ai_addrlen)!=-1) break;
                        else close();
                    }
                }
                if(addressInfo==NULL || fd==-1) throw SocketException("connection refused", __FILE__, __LINE__);
            }
            else throw SocketException("getaddrinfo error " + std::string(gai_strerror(rv)), __FILE__, __LINE__);
        }
        else throw SocketException("already connected", __FILE__, __LINE__);
    }

    /**
     * Closes the socket.
     */
    void close() {
        if(fd!=-1) ::close(fd);
        fd=-1;
    }

    /**
     * Returns the remote host address.
     * @return std::string
     */
    std::string getRemoteHostAddress() {
        if(addressInfo!=NULL) {
            char str[INET6_ADDRSTRLEN];
            inet_ntop(addressInfo->ai_family, get_in_addr((sockaddr *)addressInfo->ai_addr), str, sizeof(str));
            return std::string(str);
        }
        else return "";
    }

    /**
     * Returns true if the socket is connected.
     * @return bool
     */
    bool isConnected() {
        if(fd!=-1) return true;
        else return false;
    }

    /**
     * Reads one byte and returns its integer representation, returns -1 if the socket is closed.
     * @return int
     */
    int read() {
        char ch[1];
        int res=read(ch, 1);
        if(res > 0) return ch[0] & 0xff;
        else return res;
    }

    /**
     * Reads len bytes into the ch pointer, returns -1 if the socket is closed.
     * @param *ch char
     * @param &len const int
     * @return int
     */
    int read(char *ch, const int &len) {
        if(isConnected()) {
            if(len<=0) return 0;
            int res=recv(fd, ch, len, 0);
            if(res<=0) {
                if(errno!=EINTR) {
                    close();
                    throw SocketException("read error " + std::string(strerror(errno)), __FILE__, __LINE__);
                }
            }
            else return res;
        }
        return -1;
    }

    /**
     * Reads len bytes into the ch pointer starting at off position, returns -1 if the socket is closed.
     * @param *ch char
     * @param &off const int
     * @param &len const int
     * @return int
     */
    int read(char *ch, const int &off, const int &len) {
        return read(ch + off, len);
    }

    /**
     * Writes len bytes form the ch pointer.
     * @param *ch char
     * @param &len const int
     */
    void write(char *ch, const int &len) {
        if(isConnected()) {
            int res=send(fd, ch, len, 0);
            if(res==-1) {
                close();
                throw SocketException("write error " + std::string(strerror(errno)), __FILE__, __LINE__);
            }
        }
    }

    /**
     * Writes len bytes form the ch pointer starting at off position.
     * @param *ch char
     * @param &off const int
     * @param &len const int
     */
    void write(char *ch, const int &off, const int &len) {
        write(ch + off, len);
    }

private:

    /**
     * Returns the sockaddr according to the sa_family.
     * @param *sa sockaddr
     * @return void*
     */
    void *get_in_addr(sockaddr *sa) {
        if(sa->sa_family==AF_INET) {
            return &(((sockaddr_in*)sa)->sin_addr);
        }
        return &(((sockaddr_in6*)sa)->sin6_addr);
    }
};

#endif	/* SOCKET_HPP */
