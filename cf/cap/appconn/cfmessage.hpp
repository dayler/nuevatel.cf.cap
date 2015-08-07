/*
 * cfmessage.hpp
 */
#ifndef CFMESSAGE_HPP
#define	CFMESSAGE_HPP

#include "cfie.hpp"
#include "../../../base/appconn/appmessages.hpp"

/**
 * <p>The CFMessage abstract class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CFMessage {

public:

    /* constants for appconn */
    static int NEW_SESSION_CALL;
    static int EVENT_REPORT_CALL;
    static int WATCH_REPORT_CALL;
    static int SET_SESSION_CALL;
    static int TEST_SESSION_CALL;
    static int TEST_SESSION_ASYNC_RET;
    static int GET_MEDIA_CALL;

    virtual ~CFMessage() {}

    /**
     * Returns the message.
     * @return 8Message
     */
    virtual Message* toMessage()=0;
};

int CFMessage::NEW_SESSION_CALL=0x10;
int CFMessage::EVENT_REPORT_CALL=0x13;
int CFMessage::WATCH_REPORT_CALL=0x16;
int CFMessage::SET_SESSION_CALL=0x17;
int CFMessage::TEST_SESSION_CALL=0x18;
int CFMessage::TEST_SESSION_ASYNC_RET=0x19;

/**
 * <p>The NewSessionCall class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class NewSessionCall : public CFMessage {

    /* private variables */
    Id *id;
    char type;
    std::string auxType;
    Name *name;
    Location *location;
    SessionArg *sessionArg;

public:

    /**
     * Creates a new instance of NewSessionCall.
     * @param *id Id
     * @param &type const char
     * @param &auxType const std::string
     * @param *name Name
     * @param *location Location
     * @param *sessionArg SessionArg
     */
    NewSessionCall(Id *id, const char &type, const std::string &auxType, Name *name, Location *location, SessionArg *sessionArg) {
        this->id=id;
        this->type=type;
        this->auxType=auxType;
        this->name=name;
        this->location=location;
        this->sessionArg=sessionArg;
    }

    ~NewSessionCall() {
        if(id!=NULL) delete id;
        if(name!=NULL) delete name;
        if(location!=NULL) delete location;
        if(sessionArg!=NULL) delete sessionArg;
    }

    Message* toMessage() {
        std::vector<IE*> ies;

        IE* idIE=NULL;
        IE* auxTypeIE=NULL;
        IE* nameIE=NULL;
        IE* locationIE=NULL;
        IE* sessionArgIE=NULL;

        // id
        if(id!=NULL) {
            idIE=id->toIE();
            ies.push_back(idIE);
        }

        // type
        ByteIE typeIE(CFIE::TYPE_IE, type);
        ies.push_back(&typeIE);

        // auxType
        if(auxType.length() > 0) {
            auxTypeIE=new ByteArrayIE(CFIE::AUX_TYPE_IE, auxType);
            ies.push_back(auxTypeIE);
        }

        // name
        if(name!=NULL) {
            nameIE=name->toIE();
            ies.push_back(nameIE);
        }

        // location
        if(location!=NULL) {
            locationIE=location->toIE();
            ies.push_back(locationIE);
        }

        // sessionArg
        if(sessionArg!=NULL) {
            sessionArgIE=sessionArg->toIE();
            ies.push_back(sessionArgIE);
        }

        Message *msg=AppMessages::newCall(NEW_SESSION_CALL, &ies);

        if(idIE!=NULL) delete idIE;
        if(auxTypeIE!=NULL) delete auxTypeIE;
        if(nameIE!=NULL) delete nameIE;
        if(locationIE!=NULL) delete locationIE;
        if(sessionArgIE!=NULL) delete sessionArgIE;

        return msg;
    }
};

/**
 * <p>The EventReportCall class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class EventReportCall : public CFMessage {

    /* private variables */
    Id *id;
    char type;
    char eventType;
    EventArg *eventArg;

public:

    /**
     * Creates a new instance of EventReportCall.
     * @param *id Id
     * @param &type const char
     * @param &eventType const char
     * @param *eventArg EventArg
     */
    EventReportCall(Id *id, const char &type, const char &eventType, EventArg *eventArg) {
        this->id=id;
        this->type=type;
        this->eventType=eventType;
        this->eventArg=eventArg;
    }

    ~EventReportCall() {
        if(id!=NULL) delete id;
        if(eventArg!=NULL) delete eventArg;
    }

    Message* toMessage() {
        std::vector<IE*> ies;

        IE* idIE=NULL;
        IE* eventArgIE=NULL;

        // id
        if(id!=NULL) {
            idIE=id->toIE();
            ies.push_back(idIE);
        }

        // type
        ByteIE typeIE(CFIE::TYPE_IE, type);
        ies.push_back(&typeIE);

        // eventType
        ByteIE eventTypeIE(CFIE::EVENT_TYPE_IE, eventType);
        ies.push_back(&eventTypeIE);

        // eventArg
        if(eventArg!=NULL) {
            eventArgIE=eventArg->toIE();
            ies.push_back(eventArgIE);
        }

        Message *msg=AppMessages::newCall(EVENT_REPORT_CALL, &ies);

        if(idIE!=NULL) delete idIE;
        if(eventArgIE!=NULL) delete eventArgIE;

        return msg;
    }
};

/**
 * <p>The WatchReportCall class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class WatchReportCall : public CFMessage {

    /* private variables */
    Id *id;
    char watchType;
    char watchFlags;
    WatchArg *watchArg;

public:

    /**
     * Creates a new instance of WatchReportCall.
     * @param *id Id
     * @param &watchType const char
     * @param &watchFlags const char
     * @param *watchArg WatchArg
     */
    WatchReportCall(Id *id, const char &watchType, const char &watchFlags, WatchArg *watchArg) {
        this->id=id;
        this->watchType=watchType;
        this->watchFlags=watchFlags;
        this->watchArg=watchArg;
    }

    ~WatchReportCall() {
        if(id!=NULL) delete id;
        if(watchArg!=NULL) delete watchArg;
    }

    Message* toMessage() {
        std::vector<IE*> ies;

        IE* idIE=NULL;
        IE* watchFlagsIE=NULL;
        IE* watchArgIE=NULL;

        // id
        if(id!=NULL) {
            idIE=id->toIE();
            ies.push_back(idIE);
        }

        // watchType
        ByteIE watchTypeIE(CFIE::WATCH_TYPE_IE, watchType);
        ies.push_back(&watchTypeIE);

        // watchFlags
        if(watchFlags>=0) {
            watchFlagsIE=new ByteIE(CFIE::WATCH_FLAGS_IE, watchFlags);
            ies.push_back(watchFlagsIE);
        }

        // watchArg
        if(watchArg!=NULL) {
            watchArgIE=watchArg->toIE();
            ies.push_back(watchArgIE);
        }

        Message *msg=AppMessages::newCall(WATCH_REPORT_CALL, &ies);

        if(idIE!=NULL) delete idIE;
        if(watchFlagsIE!=NULL) delete watchFlagsIE;
        if(watchArgIE!=NULL) delete watchArgIE;

        return msg;
    }
};

/**
 * <p>The SetSessionRet class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class SetSessionRet : public CFMessage {

    /* private variables */
    char ret;

public:

    /**
     * Creates a new instance of SetSessionRet.
     * @param &ret const char
     */
    SetSessionRet(const char &ret) {
        this->ret=ret;
    }

    Message* toMessage() {
        std::vector<IE*> ies;

        // ret
        ByteIE retIE(AppMessages::RET_IE, ret);
        ies.push_back(&retIE);

        Message *msg=AppMessages::newRet(&ies);

        return msg;
    }
};

/**
 * <p>The TestSessionRet class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class TestSessionRet : public CFMessage {

    /* private variables */
    char ret;

public:

    /**
     * Creates a new instance of TestSessionRet.
     * @param &ret const char
     */
    TestSessionRet(const char &ret) {
        this->ret=ret;
    }

    Message* toMessage() {
        std::vector<IE*> ies;

        // ret
        ByteIE retIE(AppMessages::RET_IE, ret);
        ies.push_back(&retIE);

        Message *msg=AppMessages::newRet(&ies);

        return msg;
    }
};

/**
 * <p>The TestSessionAsyncRet class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class TestSessionAsyncRet : public CFMessage {

    /* private variables */
    Id *id;
    char ret;

public:

    /**
     * Creates a new instance of TestSessionAsyncRet.
     * @param *id Id
     * @param &ret const char
     */
    TestSessionAsyncRet(Id *id, const char &ret) {
        this->id=id;
        this->ret=ret;
    }

    ~TestSessionAsyncRet() {
        if(id!=NULL) delete id;
    }

    Message* toMessage() {
        std::vector<IE*> ies;

        IE* idIE=NULL;

        // id
        if(id!=NULL) {
            idIE=id->toIE();
            ies.push_back(idIE);
        }

        // ret
        ByteIE retIE(AppMessages::RET_IE, ret);
        ies.push_back(&retIE);

        Message *msg=AppMessages::newAsyncRet(TEST_SESSION_ASYNC_RET, &ies);

        if(idIE!=NULL) delete idIE;

        return msg;
    }
};

#endif	/* CFMESSAGE_HPP */
