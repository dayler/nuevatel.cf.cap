/*
 * appmessages.hpp
 */
#ifndef APPMESSAGES_HPP
#define	APPMESSAGES_HPP

#include "message.hpp"

/**
 * <p>Application messages factory.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class AppMessages {

public:

    /* public constants */
    static char VERSION;

    static char FAILED;
    static char ACCEPTED;

    /* constants for message code */
    static int ECHO_CALL;
    static int REGISTER_CALL;

    /* constants for register */
    static int LOCAL_ID_IE;     // int
    static int REMOTE_ID_IE;    // int
    static int INDEX_IE;        // short

    static int RET_IE;          // byte

    /**
     * Creates a new async call message.
     * @param &code const int
     * @param *ies std::vector<IE*>
     * @return *Message
     */
    static Message* newAsyncCall(const int &code, std::vector<IE*> *ies) {
        return new Message(VERSION, Message::ASYNC | Message::CALL, code, ies);
    }

    /**
     * Creates a new async ret message.
     * @param &code const int
     * @param *ies std::vector<IE*>
     * @return *Message
     */
    static Message* newAsyncRet(const int &code, std::vector<IE*> *ies) {
        return new Message(VERSION, Message::ASYNC | Message::RET, code, ies);
    }

    /**
     * Creates a new sync call message.
     * @param &code const int
     * @param *ies std::vector<IE*>
     * @return *Message
     */
    static Message* newCall(const int &code, std::vector<IE*> *ies) {
        return new Message(VERSION, Message::SYNC | Message::CALL, code, ies);
    }

    /**
     * Creates a new sync ret message.
     * @param *ies std::vector<IE*>
     * @return *Message
     */
    static Message* newRet(std::vector<IE*> *ies) {
        return new Message(VERSION, Message::SYNC | Message::RET, 0, ies);
    }
};

char AppMessages::VERSION=0x20;

char AppMessages::FAILED=0;
char AppMessages::ACCEPTED=1;

int AppMessages::ECHO_CALL=0;
int AppMessages::REGISTER_CALL=1;

int AppMessages::LOCAL_ID_IE=0;
int AppMessages::REMOTE_ID_IE=1;
int AppMessages::INDEX_IE=2;

int AppMessages::RET_IE=0;

#endif	/* APPMESSAGES_HPP */
