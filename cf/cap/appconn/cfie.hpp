/*
 * cfie.hpp
 */
#ifndef CFIE_HPP
#define	CFIE_HPP

#include "../../../base/appconn/compositeie.hpp"

/**
 * <p>The CFIE abstract class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CFIE {

public:

    /* constants for appconn */
    static int ID_IE;           // id
    static int TYPE_IE;         // byte
    static int AUX_TYPE_IE;     // string
    static int NAME_IE;         // name
    static int LOCATION_IE;     // location
    static int SESSION_ARG_IE;  // sessionArg
    static int ACTION_IE;       // byte
    static int MEDIA_ARG_IE;    // mediaArg
    static int EVENT_TYPE_IE;   // byte
    static int EVENT_ARG_IE;    // eventArg
    static int WATCH_TYPE_IE;   // byte
    static int WATCH_FLAGS_IE;  // byte
    static int WATCH_ARG_IE;    // watchArg

    virtual ~CFIE() {}

    /**
     * Returns the ie.
     * @return *IE
     */
    virtual IE* toIE()=0;
};

int CFIE::ID_IE         =0x10;
int CFIE::TYPE_IE       =0x11;
int CFIE::AUX_TYPE_IE   =0x12;
int CFIE::NAME_IE       =0x13;
int CFIE::LOCATION_IE   =0x14;
int CFIE::SESSION_ARG_IE=0x15;
int CFIE::ACTION_IE     =0x16;
int CFIE::MEDIA_ARG_IE  =0x17;
int CFIE::EVENT_TYPE_IE =0x18;
int CFIE::EVENT_ARG_IE  =0x19;
int CFIE::WATCH_TYPE_IE =0x20;
int CFIE::WATCH_FLAGS_IE=0x21;
int CFIE::WATCH_ARG_IE  =0x22;

/**
 * <p>The Id class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Id : public CFIE {

    /* constants for appconn */
    static int ID_ID0_IE;
    static int ID_ID1_IE;

    /* private variables */
    std::string id0;
    int id1;

public:

    /**
     * Creates a new instance of Id.
     * @param &id0 const std::string
     * @param &id1 const int
     */
    Id(const std::string &id0, const int &id1) {
        this->id0=id0;
        this->id1=id1;
    }

    /**
     * Creates a new instance if Id.
     * @param *ie IE
     */
    Id(IE *ie) {
        // id0
        id0=dynamic_cast<CompositeIE*>(ie)->getString(ID_ID0_IE);

        // id1
        id1=dynamic_cast<CompositeIE*>(ie)->getInt(ID_ID1_IE);
    }

    IE* toIE() {
        std::vector<IE*> ies;
        // id0
        ByteArrayIE id0IE(ID_ID0_IE, id0);
        ies.push_back(&id0IE);

        // id1
        IntIE id1IE(ID_ID1_IE, id1);
        ies.push_back(&id1IE);

        return new CompositeIE(ID_IE, &ies);
    }

    /**
     * Returns the id0.
     * @return std::string
     */
    std::string getId0() {
        return id0;
    }

    /**
     * Returns the id1.
     * @return int
     */
    int getId1() {
        return id1;
    }
};

int Id::ID_ID0_IE=0;
int Id::ID_ID1_IE=1;

/**
 * The SERVICE_TYPE.
 */
class SERVICE_TYPE {

    /** The type. */
    char type;

    SERVICE_TYPE(const char &type) {
        this->type=type;
    }

public:

    /* type */
    static SERVICE_TYPE SPEECH;
    static SERVICE_TYPE VIDEO;
    static SERVICE_TYPE DATA;
    static SERVICE_TYPE MESSAGE;

    /**
     * Returns the type.
     * @return char
     */
    char getType() {
        return type;
    }

    /**
     * Returns the type.
     * @param &type const char
     * @return char
     */
    static char getType(const char &type) {
        return type & 0x70;
    }
};

SERVICE_TYPE SERVICE_TYPE::SPEECH =SERVICE_TYPE(0x10);
SERVICE_TYPE SERVICE_TYPE::VIDEO  =SERVICE_TYPE(0x20);
SERVICE_TYPE SERVICE_TYPE::DATA   =SERVICE_TYPE(0x30);
SERVICE_TYPE SERVICE_TYPE::MESSAGE=SERVICE_TYPE(0x40);

/**
 * The REQUEST_TYPE.
 */
class REQUEST_TYPE {

    /** The type. */
    char type;

    REQUEST_TYPE(const char &type) {
        this->type=type;
    }

public:

    /* type */
    static REQUEST_TYPE O;
    static REQUEST_TYPE T;
    static REQUEST_TYPE FWO;
    static REQUEST_TYPE FWT;
    static REQUEST_TYPE PDPCE;
    static REQUEST_TYPE PDPCEA;
    static REQUEST_TYPE PDPCHP;
    static REQUEST_TYPE DCCA;

    /**
     * Returns the type.
     * @return char
     */
    char getType() {
        return type;
    }

    /**
     * Returns the type.
     * @param &type const char
     * @return char
     */
    static char getType(const char &type) {
        return type & 0xf;
    }
};

REQUEST_TYPE REQUEST_TYPE::O     =REQUEST_TYPE(0x1);
REQUEST_TYPE REQUEST_TYPE::T     =REQUEST_TYPE(0x2);
REQUEST_TYPE REQUEST_TYPE::FWO   =REQUEST_TYPE(0x3);
REQUEST_TYPE REQUEST_TYPE::FWT   =REQUEST_TYPE(0x4);
REQUEST_TYPE REQUEST_TYPE::PDPCE =REQUEST_TYPE(0x5);
REQUEST_TYPE REQUEST_TYPE::PDPCEA=REQUEST_TYPE(0x6);
REQUEST_TYPE REQUEST_TYPE::PDPCHP=REQUEST_TYPE(0x7);
REQUEST_TYPE REQUEST_TYPE::DCCA  =REQUEST_TYPE(0x8);

/**
 * <p>The Name class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Name : public CFIE {

    /* constants for appconn */
    static int NAME_NAME_IE;
    static int NAME_TYPE_IE;

    /* private variables */
    std::string name;
    char type;

public:

    /**
     * Creates a new instance of Name.
     * @param &name const std::string
     * @param &type const char
     */
    Name(const std::string &name, const char &type) {
        this->name=name;
        this->type=type;
    }

    /**
     * Creates a new instance of Name.
     * @param *ie IE
     */
    Name(IE *ie) {
        // name
        name=dynamic_cast<CompositeIE*>(ie)->getString(NAME_NAME_IE);

        // type
        type=dynamic_cast<CompositeIE*>(ie)->getByte(NAME_TYPE_IE);
    }

    IE* toIE() {
        return toIE(NAME_IE);
    }

    /**
     * Returns the ie.
     * @param &code const int
     * @return *IE
     */
    IE* toIE(const int &code) {
        std::vector<IE*> ies;
        // name
        ByteArrayIE nameIE(NAME_NAME_IE, name);
        ies.push_back(&nameIE);

        // type
        ByteIE typeIE(NAME_TYPE_IE, type);
        ies.push_back(&typeIE);

        return new CompositeIE(code, &ies);
    }

    /**
     * Returns the name.
     * @return std::string
     */
    std::string getName() {
        return name;
    }

    /**
     * Returns the type.
     * @return char
     */
    char getType() {
        return type;
    }
};

int Name::NAME_NAME_IE=0;
int Name::NAME_TYPE_IE=1;

/**
 * <p>The Location class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Location : public CFIE {

    /* constants for appconn */
    static int LOCATION_G_LOCATION_IE;
    static int LOCATION_NODE_ID_IE;

    /* private variables */
    std::string gLocation;
    std::string nodeId;

public:

    /**
     * Creates a new instance of Location.
     * @param &gLocation const std::string
     * @param &nodeId const std::string
     */
    Location(const std::string &gLocation, const std::string &nodeId) {
        this->gLocation=gLocation;
        this->nodeId=nodeId;
    }

    /**
     * Creates a new instance of Location.
     * @param *ie IE
     */
    Location(IE *ie) {
        // gLocation
        gLocation=dynamic_cast<CompositeIE*>(ie)->getString(LOCATION_G_LOCATION_IE);

        // nodeId
        nodeId=dynamic_cast<CompositeIE*>(ie)->getString(LOCATION_NODE_ID_IE);
    }

    IE* toIE() {
        std::vector<IE*> ies;

        IE *gLocationIE=NULL;
        IE *nodeIdIE=NULL;

        // gLocation
        if(gLocation.length() > 0) {
            gLocationIE=new ByteArrayIE(LOCATION_G_LOCATION_IE, gLocation);
            ies.push_back(gLocationIE);
        }

        // nodeId
        if(nodeId.length() > 0) {
            nodeIdIE=new ByteArrayIE(LOCATION_NODE_ID_IE, nodeId);
            ies.push_back(nodeIdIE);
        }

        CompositeIE *locationIE=new CompositeIE(LOCATION_IE, &ies);

        if(gLocationIE!=NULL) delete gLocationIE;
        if(nodeIdIE!=NULL) delete nodeIdIE;

        return locationIE;
    }

    /**
     * Returns the gLocation.
     * @return std::string
     */
    std::string getGLocation() {
        return gLocation;
    }

    /**
     * Returns the nodeId.
     * @return std::string
     */
    std::string getNodeId() {
        return nodeId;
    }
};

int Location::LOCATION_G_LOCATION_IE=0;
int Location::LOCATION_NODE_ID_IE=1;

/**
 * <p>The SessionArg class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class SessionArg : public CFIE {

    /* constants for appconn */
    static int SESSION_ARG_FROM_NAME_IE;
    static int SESSION_ARG_TO_NAME_IE;
    static int SESSION_ARG_APN_IE;
    static int SESSION_ARG_QOS_IE;
    static int SESSION_ARG_UEI_IE;
    static int SESSION_ARG_REFERENCE_IE;

    /* private variables */
    Name *fromName;
    Name *toName;
    std::string apn;
    std::string qos;
    std::string uei;
    std::string reference;

public:

    /**
     * Creates a new instance of SessionArg.
     * @param *fromName Name
     * @param *toName Name
     * @param &apn const std::string
     * @param &qos const std::string
     * @param &uei const std::string
     * @param &reference const std::string
     */
    SessionArg(Name *fromName, Name *toName, const std::string &apn, const std::string &qos, const std::string &uei, const std::string &reference) {
        this->fromName=fromName;
        this->toName=toName;
        this->apn=apn;
        this->qos=qos;
        this->uei=uei;
        this->reference=reference;
    }

    /**
     * Creates a new instance of SessionArg.
     * @param *ie IE
     */
    SessionArg(IE *ie) {
        // fromName
        CompositeIE *fromNameIE=dynamic_cast<CompositeIE*>(ie)->getComposite(SESSION_ARG_FROM_NAME_IE);
        if(fromNameIE!=NULL) fromName=new Name(fromNameIE);
        else fromName=NULL;

        // toName
        CompositeIE *toNameIE=dynamic_cast<CompositeIE*>(ie)->getComposite(SESSION_ARG_TO_NAME_IE);
        if(toNameIE!=NULL) toName=new Name(toNameIE);
        else toName=NULL;

        // apn
        apn=dynamic_cast<CompositeIE*>(ie)->getString(SESSION_ARG_APN_IE);

        // qos
        qos=dynamic_cast<CompositeIE*>(ie)->getString(SESSION_ARG_QOS_IE);

        // uei
        uei=dynamic_cast<CompositeIE*>(ie)->getString(SESSION_ARG_UEI_IE);

        // reference
        reference=dynamic_cast<CompositeIE*>(ie)->getString(SESSION_ARG_REFERENCE_IE);
    }

    ~SessionArg() {
        if(fromName!=NULL) delete fromName;
        if(toName!=NULL) delete toName;
    }

    IE* toIE() {
        std::vector<IE*> ies;

        IE *fromNameIE=NULL;
        IE *toNameIE=NULL;
        IE *apnIE=NULL;
        IE *qosIE=NULL;
        IE *ueiIE=NULL;
        IE *referenceIE=NULL;

        // fromName
        if(fromName!=NULL) {
            fromNameIE=fromName->toIE(SESSION_ARG_FROM_NAME_IE);
            ies.push_back(fromNameIE);
        }

        // toName
        if(toName!=NULL) {
            toNameIE=toName->toIE(SESSION_ARG_TO_NAME_IE);
            ies.push_back(toNameIE);
        }

        // apn
        if(apn.length() > 0) {
            apnIE=new ByteArrayIE(SESSION_ARG_APN_IE, apn);
            ies.push_back(apnIE);
        }

        // qos
        if(qos.length() > 0) {
            qosIE=new ByteArrayIE(SESSION_ARG_QOS_IE, qos);
            ies.push_back(qosIE);
        }

        // uei
        if(uei.length() > 0) {
            ueiIE=new ByteArrayIE(SESSION_ARG_UEI_IE, uei);
            ies.push_back(ueiIE);
        }

        // reference
        if(reference.length() > 0) {
            referenceIE=new ByteArrayIE(SESSION_ARG_REFERENCE_IE, reference);
            ies.push_back(referenceIE);
        }

        CompositeIE *sessionArgIE=new CompositeIE(SESSION_ARG_IE, &ies);

        if(fromNameIE!=NULL) delete fromNameIE;
        if(toNameIE!=NULL) delete toNameIE;
        if(apnIE!=NULL) delete apnIE;
        if(qosIE!=NULL) delete qosIE;
        if(referenceIE!=NULL) delete referenceIE;

        return sessionArgIE;
    }

    /**
     * Returns the fromName.
     * @return *Name
     */
    Name* getFromName() {
        return fromName;
    }

    /**
     * Returns the toName.
     * @return *Name
     */
    Name* getToName() {
        return toName;
    }

    /**
     * Returns the apn.
     * @return std::string
     */
    std::string getAPN() {
        return apn;
    }

    /**
     * Returns the qos.
     * @return std::string
     */
    std::string getQOS() {
        return qos;
    }

    /**
     * Returns the uei.
     * @return std::string
     */
    std::string getUEI() {
        return uei;
    }

    /**
     * Returns the reference.
     * @return std::string
     */
    std::string getReference() {
        return reference;
    }
};

int SessionArg::SESSION_ARG_FROM_NAME_IE=0;
int SessionArg::SESSION_ARG_TO_NAME_IE=1;
int SessionArg::SESSION_ARG_APN_IE=2;
int SessionArg::SESSION_ARG_QOS_IE=3;
int SessionArg::SESSION_ARG_UEI_IE=4;
int SessionArg::SESSION_ARG_REFERENCE_IE=5;

/**
 * The MEDIA_ACTION.
 */
class MEDIA_ACTION {

    /** The action. */
    char action;

    MEDIA_ACTION(const char &action) {
        this->action=action;
    }

public:

    /* action */
    static MEDIA_ACTION NEW_MEDIA;
    static MEDIA_ACTION END_MEDIA;

    /**
     * Returns the action.
     * @return char
     */
    char getAction() {
        return action;
    }

    /**
     * Returns the action.
     * @param &action const char
     * @return char
     */
    static char getMediaAction(const char &action) {
        return action & 0x30;
    }
};

MEDIA_ACTION MEDIA_ACTION::NEW_MEDIA=MEDIA_ACTION(0x10);
MEDIA_ACTION MEDIA_ACTION::END_MEDIA=MEDIA_ACTION(0x20);

/**
 * The SESSION_ACTION enum.
 */
class SESSION_ACTION {

    /** The action. */
    char action;

    SESSION_ACTION(const char &action) {
        this->action=action;
    }

public:

    /* action */
    static SESSION_ACTION ACCEPT;
    static SESSION_ACTION MODIFY;
    static SESSION_ACTION END;

    /**
     * Returns the action.
     * @return char
     */
    char getAction() {
        return action;
    }

    /**
     * Returns the action.
     * @param &action const char
     * @return char
     */
    static char getSessionAction(const char &action) {
        return action & 0xf;
    }
};

SESSION_ACTION SESSION_ACTION::ACCEPT=SESSION_ACTION(0x1);
SESSION_ACTION SESSION_ACTION::MODIFY=SESSION_ACTION(0x2);
SESSION_ACTION SESSION_ACTION::END   =SESSION_ACTION(0x3);

/**
 * <p>The MediaArg class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class MediaArg : public CFIE {

    /* constants for appconn */
    static int MEDIA_ARG_MEDIA_ARG0_IE;
    static int MEDIA_ARG_MEDIA_ARG1_IE;
    static int MEDIA_ARG_MEDIA_ARG2_IE;

    /* private variables */
    std::string mediaArg0;
    char mediaArg1;
    int mediaArg2;

public:

    /**
     * Creates a new instance of MediaArg.
     * @param &mediaArg0 const std::string
     * @param &mediaArg1 const char
     * @param &mediaArg2 const int
     */
    MediaArg(const std::string &mediaArg0, const char &mediaArg1, const int &mediaArg2) {
        this->mediaArg0=mediaArg0;
        this->mediaArg1=mediaArg1;
        this->mediaArg2=mediaArg2;
    }

    /**
     * Creates a new instance if MediaArg.
     * @param *ie IE
     */
    MediaArg(IE *ie) {
        // mediaArg0
        mediaArg0=dynamic_cast<CompositeIE*>(ie)->getString(MEDIA_ARG_MEDIA_ARG0_IE);

        // mediaArg1
        mediaArg1=dynamic_cast<CompositeIE*>(ie)->getByte(MEDIA_ARG_MEDIA_ARG1_IE);

        // mediaArg2
        mediaArg2=dynamic_cast<CompositeIE*>(ie)->getInt(MEDIA_ARG_MEDIA_ARG2_IE);
    }

    IE* toIE() {
        std::vector<IE*> ies;

        IE* mediaArg0IE=NULL;
        IE* mediaArg2IE=NULL;

        // mediaArg0
        if(mediaArg0.length() > 0) {
            mediaArg0IE=new ByteArrayIE(MEDIA_ARG_MEDIA_ARG0_IE, mediaArg0);
            ies.push_back(mediaArg0IE);
        }

        // mediaArg1
        ByteIE mediaArg1IE(MEDIA_ARG_MEDIA_ARG1_IE, mediaArg1);
        ies.push_back(&mediaArg1IE);

        if(mediaArg2>=0) {
            // mediaArg2
            mediaArg2IE=new IntIE(MEDIA_ARG_MEDIA_ARG2_IE, mediaArg2);
            ies.push_back(mediaArg2IE);
        }

        CompositeIE *mediaArgIE=new CompositeIE(MEDIA_ARG_IE, &ies);

        if(mediaArg0IE!=NULL) delete mediaArg0IE;
        if(mediaArg2IE!=NULL) delete mediaArg2IE;

        return mediaArgIE;
    }

    /**
     * Returns the mediaArg0.
     * @return std::string
     */
    std::string getMediaArg0() {
        return mediaArg0;
    }

    /**
     * Returns the mediaArg1.
     * @return char
     */
    char getMediaArg1() {
        return mediaArg1;
    }

    /**
     * Returns the mediaArg2.
     * @return int
     */
    int getMediaArg2() {
        return mediaArg2;
    }
};

int MediaArg::MEDIA_ARG_MEDIA_ARG0_IE=0;
int MediaArg::MEDIA_ARG_MEDIA_ARG1_IE=1;
int MediaArg::MEDIA_ARG_MEDIA_ARG2_IE=2;

/**
 * The EVENT_TYPE.
 */
class EVENT_TYPE {

    /** The type. */
    char type;

    EVENT_TYPE(const char &type) {
        this->type=type;
    }

public:

    /* type */
    static EVENT_TYPE ROUTE_SELECT_FAILURE_2;
    static EVENT_TYPE O_CALLED_PARTY_BUSY_2;
    static EVENT_TYPE O_NO_ANSWER_2;
    static EVENT_TYPE O_ANSWER_2;
    static EVENT_TYPE O_DISCONNECT_1;
    static EVENT_TYPE O_DISCONNECT_2;
    static EVENT_TYPE O_ABANDON_1;
    static EVENT_TYPE T_BUSY_2;
    static EVENT_TYPE T_NO_ANSWER_2;
    static EVENT_TYPE T_ANSWER_2;
    static EVENT_TYPE T_DISCONNECT_1;
    static EVENT_TYPE T_DISCONNECT_2;
    static EVENT_TYPE T_ABANDON_1;
    static EVENT_TYPE PDP_CONTEXT_ESTABLISHMENT_ACK;
    static EVENT_TYPE DISCONNECT;
    static EVENT_TYPE ENTITY_RELEASED_GPRS;
    static EVENT_TYPE U_ABORT;
    static EVENT_TYPE P_ABORT;
    static EVENT_TYPE SET_SESSION_FAILED;

    /**
     * Returns the type.
     * @return char
     */
    char getType() {
        return type;
    }
};

EVENT_TYPE EVENT_TYPE::ROUTE_SELECT_FAILURE_2       =EVENT_TYPE(1);
EVENT_TYPE EVENT_TYPE::O_CALLED_PARTY_BUSY_2        =EVENT_TYPE(2);
EVENT_TYPE EVENT_TYPE::O_NO_ANSWER_2                =EVENT_TYPE(3);
EVENT_TYPE EVENT_TYPE::O_ANSWER_2                   =EVENT_TYPE(4);
EVENT_TYPE EVENT_TYPE::O_DISCONNECT_1               =EVENT_TYPE(5);
EVENT_TYPE EVENT_TYPE::O_DISCONNECT_2               =EVENT_TYPE(6);
EVENT_TYPE EVENT_TYPE::O_ABANDON_1                  =EVENT_TYPE(7);
EVENT_TYPE EVENT_TYPE::T_BUSY_2                     =EVENT_TYPE(8);
EVENT_TYPE EVENT_TYPE::T_NO_ANSWER_2                =EVENT_TYPE(9);
EVENT_TYPE EVENT_TYPE::T_ANSWER_2                   =EVENT_TYPE(10);
EVENT_TYPE EVENT_TYPE::T_DISCONNECT_1               =EVENT_TYPE(11);
EVENT_TYPE EVENT_TYPE::T_DISCONNECT_2               =EVENT_TYPE(12);
EVENT_TYPE EVENT_TYPE::T_ABANDON_1                  =EVENT_TYPE(13);
EVENT_TYPE EVENT_TYPE::PDP_CONTEXT_ESTABLISHMENT_ACK=EVENT_TYPE(14);
EVENT_TYPE EVENT_TYPE::DISCONNECT                   =EVENT_TYPE(15);
EVENT_TYPE EVENT_TYPE::ENTITY_RELEASED_GPRS         =EVENT_TYPE(16);
EVENT_TYPE EVENT_TYPE::U_ABORT                      =EVENT_TYPE(17);
EVENT_TYPE EVENT_TYPE::P_ABORT                      =EVENT_TYPE(18);
EVENT_TYPE EVENT_TYPE::SET_SESSION_FAILED           =EVENT_TYPE(19);

/**
 * <p>The EventArg class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class EventArg : public CFIE {

    /* constants for appconn */
    static int EVENT_ARG_EVENT_ARG0_IE;

    /* private variables */
    int eventArg0;

public:

    /**
     * Creates a new instance of EventArg.
     * @param eventArg0 String
     */
    EventArg(const int &eventArg0) {
        this->eventArg0=eventArg0;
    }

    /**
     * Creates a new instance if EventArg.
     * @param *ie IE
     */
    EventArg(IE *ie) {
        eventArg0=dynamic_cast<CompositeIE*>(ie)->getInt(EVENT_ARG_EVENT_ARG0_IE);
    }

    IE* toIE() {
        std::vector<IE*> ies;
        // eventArg0
        IntIE eventArg0IE(EVENT_ARG_EVENT_ARG0_IE, eventArg0);
        ies.push_back(&eventArg0IE);

        return new CompositeIE(EVENT_ARG_IE, &ies);
    }

    /**
     * Returns the eventArg0.
     * @return int
     */
    int getEventArg0() {
        return eventArg0;
    }
};

int EventArg::EVENT_ARG_EVENT_ARG0_IE=0;

/**
 * The WATCH_TYPE.
 */
class WATCH_TYPE {

    /** The type. */
    char type;

    WATCH_TYPE(const char &type) {
        this->type=type;
    }

public:

    /* type */
    static WATCH_TYPE A_TIME_WATCH;
    static WATCH_TYPE TIME_WATCH;
    static WATCH_TYPE D_A_TIME_WATCH;
    static WATCH_TYPE D_TIME_WATCH;
    static WATCH_TYPE D_A_UNIT_WATCH;
    static WATCH_TYPE D_UNIT_WATCH;
    static WATCH_TYPE THRESHOLD;
    static WATCH_TYPE QHT;
    static WATCH_TYPE FINAL;
    static WATCH_TYPE QUOTA_EXHAUSTED;
    static WATCH_TYPE VALIDITY_TIME;
    static WATCH_TYPE OTHER_QUOTA_TYPE;
    static WATCH_TYPE RATING_CONDITION_CHANGE;
    static WATCH_TYPE FORCED_REAUTHORIZATION;
    static WATCH_TYPE POOL_EXHAUSTED;

    /**
     * Returns the type.
     * @return char
     */
    char getType() {
        return type;
    }
};

WATCH_TYPE WATCH_TYPE::A_TIME_WATCH           =WATCH_TYPE(1);
WATCH_TYPE WATCH_TYPE::TIME_WATCH             =WATCH_TYPE(2);
WATCH_TYPE WATCH_TYPE::D_A_TIME_WATCH         =WATCH_TYPE(3);
WATCH_TYPE WATCH_TYPE::D_TIME_WATCH           =WATCH_TYPE(4);
WATCH_TYPE WATCH_TYPE::D_A_UNIT_WATCH         =WATCH_TYPE(5);
WATCH_TYPE WATCH_TYPE::D_UNIT_WATCH           =WATCH_TYPE(6);
WATCH_TYPE WATCH_TYPE::THRESHOLD              =WATCH_TYPE(10);
WATCH_TYPE WATCH_TYPE::QHT                    =WATCH_TYPE(11);
WATCH_TYPE WATCH_TYPE::FINAL                  =WATCH_TYPE(12);
WATCH_TYPE WATCH_TYPE::QUOTA_EXHAUSTED        =WATCH_TYPE(13);
WATCH_TYPE WATCH_TYPE::VALIDITY_TIME          =WATCH_TYPE(14);
WATCH_TYPE WATCH_TYPE::OTHER_QUOTA_TYPE       =WATCH_TYPE(15);
WATCH_TYPE WATCH_TYPE::RATING_CONDITION_CHANGE=WATCH_TYPE(16);
WATCH_TYPE WATCH_TYPE::FORCED_REAUTHORIZATION =WATCH_TYPE(17);
WATCH_TYPE WATCH_TYPE::POOL_EXHAUSTED         =WATCH_TYPE(18);

/**
 * <p>The WatchArg class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class WatchArg : public CFIE {

    /* constants for appconn */
    static int WATCH_ARG_WATCH_ARG0_IE;
    static int WATCH_ARG_WATCH_ARG1_IE;
    static int WATCH_ARG_WATCH_ARG2_IE;
    static int WATCH_ARG_WATCH_ARG3_IE;
    static int WATCH_ARG_WATCH_ARG4_IE;
    static int WATCH_ARG_WATCH_ARG5_IE;

    /* private variables */
    int watchArg0;
    long watchArg1;
    long watchArg2;
    long watchArg3;
    long watchArg4;
    int watchArg5;

public:

    /**
     * Creates a new instance of WatchArg.
     * @param &watchArg0 const int
     * @param &watchArg1 const long
     * @param &watchArg2 const long
     * @param &watchArg3 const long
     * @param &watchArg4 const long
     * @param &watchArg5 const int
     */
    WatchArg(const int &watchArg0, const long &watchArg1, const long &watchArg2, const long &watchArg3, const long &watchArg4, const int &watchArg5) {
        this->watchArg0=watchArg0;
        this->watchArg1=watchArg1;
        this->watchArg2=watchArg2;
        this->watchArg3=watchArg3;
        this->watchArg4=watchArg4;
        this->watchArg5=watchArg5;
    }

    /**
     * Creates a new instance if WatchArg.
     * @param *ie IE
     */
    WatchArg(IE *ie) {
        // watchArg0
        watchArg0=dynamic_cast<CompositeIE*>(ie)->getInt(WATCH_ARG_WATCH_ARG0_IE);

        // watchArg1
        watchArg1=dynamic_cast<CompositeIE*>(ie)->getLong(WATCH_ARG_WATCH_ARG1_IE);

        // watchArg2
        watchArg2=dynamic_cast<CompositeIE*>(ie)->getLong(WATCH_ARG_WATCH_ARG2_IE);

        // watchArg3
        watchArg3=dynamic_cast<CompositeIE*>(ie)->getLong(WATCH_ARG_WATCH_ARG3_IE);

        // watchArg4
        watchArg4=dynamic_cast<CompositeIE*>(ie)->getLong(WATCH_ARG_WATCH_ARG4_IE);

        // watchArg5
        watchArg5=dynamic_cast<CompositeIE*>(ie)->getInt(WATCH_ARG_WATCH_ARG5_IE);
    }

    IE* toIE() {
        std::vector<IE*> ies;

        IE *watchArg0IE=NULL;
        IE *watchArg1IE=NULL;
        IE *watchArg2IE=NULL;
        IE *watchArg3IE=NULL;
        IE *watchArg4IE=NULL;
        IE *watchArg5IE=NULL;

        // watchArg0
        if(watchArg0>=0) {
            watchArg0IE=new IntIE(WATCH_ARG_WATCH_ARG0_IE, watchArg0);
            ies.push_back(watchArg0IE);
        }

        // watchArg1
        if(watchArg1>=0) {
            watchArg1IE=new LongIE(WATCH_ARG_WATCH_ARG1_IE, watchArg1);
            ies.push_back(watchArg1IE);
        }

        // watchArg2
        if(watchArg2>=0) {
            watchArg2IE=new LongIE(WATCH_ARG_WATCH_ARG2_IE, watchArg2);
            ies.push_back(watchArg2IE);
        }

        // watchArg3
        if(watchArg3>=0) {
            watchArg3IE=new LongIE(WATCH_ARG_WATCH_ARG3_IE, watchArg3);
            ies.push_back(watchArg3IE);
        }

        // watchArg4
        if(watchArg4>=0) {
            watchArg4IE=new LongIE(WATCH_ARG_WATCH_ARG4_IE, watchArg4);
            ies.push_back(watchArg4IE);
        }

        // watchArg5
        if(watchArg5>=0) {
            watchArg0IE=new IntIE(WATCH_ARG_WATCH_ARG5_IE, watchArg5);
            ies.push_back(watchArg5IE);
        }

        CompositeIE *watchArgIE=new CompositeIE(WATCH_ARG_IE, &ies);

        if(watchArg0IE!=NULL) delete watchArg0IE;
        if(watchArg1IE!=NULL) delete watchArg1IE;
        if(watchArg2IE!=NULL) delete watchArg2IE;
        if(watchArg3IE!=NULL) delete watchArg3IE;
        if(watchArg4IE!=NULL) delete watchArg4IE;
        if(watchArg5IE!=NULL) delete watchArg5IE;

        return watchArgIE;
    }

    /**
     * Returns the watchArg0.
     * @return int
     */
    int getWatchArg0() {
        return watchArg0;
    }

    /**
     * Returns the watchArg1.
     * @return long
     */
    long getWatchArg1() {
        return watchArg1;
    }

    /**
     * Returns the watchArg2.
     * @return long
     */
    long getWatchArg2() {
        return watchArg2;
    }

    /**
     * Returns the watchArg3.
     * @return long
     */
    long getWatchArg3() {
        return watchArg3;
    }

    /**
     * Returns the watchArg4.
     * @return long
     */
    long getWatchArg4() {
        return watchArg4;
    }

    /**
     * Returns the watchArg5.
     * @return int
     */
    int getWatchArg5() {
        return watchArg5;
    }
};

int WatchArg::WATCH_ARG_WATCH_ARG0_IE=0;
int WatchArg::WATCH_ARG_WATCH_ARG1_IE=1;
int WatchArg::WATCH_ARG_WATCH_ARG2_IE=2;
int WatchArg::WATCH_ARG_WATCH_ARG3_IE=3;
int WatchArg::WATCH_ARG_WATCH_ARG4_IE=4;
int WatchArg::WATCH_ARG_WATCH_ARG5_IE=5;

#endif	/* CFIE_HPP */
