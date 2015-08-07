/*
 * message.hpp
 */
#ifndef MESSAGE_HPP
#define	MESSAGE_HPP

#include "compositeie.hpp"

/**
 * <p>The Message class should be used to define an application message.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Message : public CompositeIE {

public:

    /* constants */
    static char IO_TYPE;
    /* ioType 1 bit 7 */
    static char ASYNC;
    static char SYNC;

    static char MESSAGE_TYPE;
    /* type 1 bit 6 */
    static char CALL;
    static char RET;

    /* timeToLive constants */
    static short TIME_TO_LIVE_1S;
    static short TIME_TO_LIVE_2S;
    static short TIME_TO_LIVE_4S;
    static short TIME_TO_LIVE_8S;

private:

    /* private constants */
    static short DEFAULT_TIME_TO_LIVE;

    /** The version. */
    char version;

    /** The type. */
    char type;

    /** The id. */
    int id;

    /** The timeToLive. */
    short timeToLive;

public:

    /**
     * Creates a new instance of Message.
     * @param &version const char
     * @param &type const char
     * @param &code const int
     * @param *ies std::vector<IE*>
     */
    Message(const char &version, const char &type, const int &code, std::vector<IE*> *ies) : CompositeIE(code, ies) {
        this->version=version;
        this->type=type;
        id=0;
        timeToLive=0;
    }

    /**
     * Creates a new instance of Message.
     * @param &version const char
     * @param &type const char
     * @param &id const int
     * @param &timeToLive const short
     * @param &code const int
     * @param *value const char
     * @param &length const int
     */
    Message(const char &version, const char &type, const int &id, const short &timeToLive, const int &code, const char *value, const int &length) : CompositeIE(code, value, length) {
        this->version=version;
        this->type=type;
        this->id=id;
        this->timeToLive=timeToLive;
    }

    IE* clone() {
        std::vector<IE*> ies;
        for(std::map<int, std::vector<IE*>*>::iterator iter=ieMap.begin(); iter!=ieMap.end(); iter++) {
            std::vector<IE*> *ieVector=iter->second;
            for(std::vector<IE*>::iterator ieIter=ieVector->begin(); ieIter!=ieVector->end(); ieIter++) ies.push_back(*ieIter);
        }
        Message *msg=new Message(version, type, code, &ies);
        msg->setId(id);
        if(timeToLive!=0) msg->setTimeToLive(timeToLive);
        return msg;
    }

    char getType() {
        return type;
    }

    void getBytes(char *b, int &length) {
        int tmpLength=0;
        for(std::map<int, std::vector<IE*>*>::iterator iter=ieMap.begin(); iter!=ieMap.end(); iter++) {
            std::vector<IE*> *ieVector=iter->second;
            for(std::vector<IE*>::iterator ieIter=ieVector->begin(); ieIter!=ieVector->end(); ieIter++) {
                int ieLength;
                IE *ie=*ieIter;
                ie->getBytes(b + tmpLength, ieLength);
                tmpLength+=ieLength;
            }
        }

        char tmp[10];
        int tmpIndex;
        tmp[0]=version;
        tmp[1]=type;
        tmpIndex=2;
        // id
        if(isSync()) {
            tmp[2]=(char)((id >> 24) & 0xff);
            tmp[3]=(char)((id >> 16) & 0xff);
            tmp[4]=(char)((id >> 8) & 0xff);
            tmp[5]=(char)(id & 0xff);
            tmpIndex=6;
            // timeToLive
            if(isCall()) tmp[tmpIndex++]=(char)getTimeToLive();
        }
        // code
        if(isAsync() || isCall()) tmp[tmpIndex++]=code;

        if(tmpLength==0) {}
        else if(tmpLength<=0xff) {
            tmp[1]|=L_LENGTH_1;
            tmp[tmpIndex++]=(char)tmpLength;
        }
        else if(tmpLength<=0xffff) {
            tmp[1]|=L_LENGTH_2;
            tmp[tmpIndex++]=(char)((tmpLength >> 8) & 0xff);
            tmp[tmpIndex++]=(char)(tmpLength & 0xff);
        }
        else {
            std::stringstream ss;
            ss << "illegal length " << length;
            throw Exception(ss.str(), __FILE__, __LINE__);
        }

        for(int i=tmpLength - 1; i>=0; i--) b[i + tmpIndex]=b[i];
        for(int i=0; i < tmpIndex; i++) b[i]=tmp[i];
        length=tmpIndex + tmpLength;
    }

    std::string toXML() {
        std::stringstream ss;
        ss << "<message version=\"" << (int)version << "\" type=\"" << (int)type;
        if(isSync()) {
            ss << "\" id=\"" << id;
            // timeToLive
            if(isCall()) {
                ss << "\" timeToLive=\"" << getTimeToLive();
            }
        }
        // code
        if(isAsync() || isCall()) ss << "\" code=\"" << getCode();
        ss << "\">" << LINE_SEPARATOR;
        for(std::map<int, std::vector<IE*>*>::iterator iter=ieMap.begin(); iter!=ieMap.end(); iter++) {
            std::vector<IE*> *ieVector=iter->second;
            for(std::vector<IE*>::iterator ieIter=ieVector->begin(); ieIter!=ieVector->end(); ieIter++) {
                IE *ie=*ieIter;
                ss << ie->toXML();
            }
        }
        ss << "</message>" << LINE_SEPARATOR;
        return ss.str();
    }

    /**
     * Returns the version.
     * @return char
     */
    char getVersion() {
        return version;
    }

    /**
     * Returns true if the message is async.
     * @return bool
     */
    bool isAsync() {
        return (type & IO_TYPE)==ASYNC;
    }

    /**
     * Returns true if the message is sync.
     * @return bool
     */
    bool isSync() {
        return (type & IO_TYPE)==SYNC;
    }

    /**
     * Returns true if the message is call.
     * @return bool
     */
    bool isCall() {
        return (type & MESSAGE_TYPE)==CALL;
    }

    /**
     * Returns true if the message is ret.
     * @return bool
     */
    bool isRet() {
        return (type & MESSAGE_TYPE)==RET;
    }

    /**
     * Sets the id.
     * @param &id const int
     */
    void setId(const int &id) {
        this->id=id;
    }

    /**
     * Returns the id.
     * @return int
     */
    int getId() {
        return id;
    }

    /**
     * Sets the timeToLive.
     * @param timeToLive short
     */
    void setTimeToLive(const short &timeToLive) {
        if(timeToLive < 1 || timeToLive > 0xff) {
            std::stringstream ss;
            ss << "illegal timeToLive " << timeToLive;
            throw Exception(ss.str(), __FILE__, __LINE__);
        }
        this->timeToLive=timeToLive;
    }

    /**
     * Returns the timeToLive.
     * @return short
     */
    short getTimeToLive() {
        if(timeToLive==0) timeToLive=DEFAULT_TIME_TO_LIVE;
        return timeToLive;
    }
};

char Message::IO_TYPE=0x20;

char Message::ASYNC=0;
char Message::SYNC=0x20;

char Message::MESSAGE_TYPE=0x10;

char Message::CALL=0;
char Message::RET=0x10;

short Message::TIME_TO_LIVE_1S=1;
short Message::TIME_TO_LIVE_2S=2;
short Message::TIME_TO_LIVE_4S=4;
short Message::TIME_TO_LIVE_8S=8;
short Message::DEFAULT_TIME_TO_LIVE=Message::TIME_TO_LIVE_4S;

#endif	/* MESSAGE_HPP */
