/*
 * compositeie.hpp
 */
#ifndef COMPOSITEIE_HPP
#define	COMPOSITEIE_HPP

#include "primitiveie.hpp"
#include <map>
#include <vector>

/**
 * <p>The CompositeIE class should be used to define composite IE type.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CompositeIE : public IE {

protected:

    /** The code. */
    char code;

    /** The ieMap. */
    std::map<int, std::vector<IE*>*> ieMap;

public:

    /**
     * Creates a new instance of CompositeIE.
     * @param &code const int
     * @param *ies std::vector<IE*>
     */
    CompositeIE(const int &code, std::vector<IE*> *ies) {
        if(code < 0 || code > 0xff) {
            std::stringstream ss;
            ss << "illegal code " << (int)code;
            throw Exception(ss.str(), __FILE__, __LINE__);
        }
        this->code=(char)code;
        if(ies!=NULL)
            for(std::vector<IE*>::iterator iter=ies->begin(); iter!=ies->end(); iter++) {
                IE *ie=*iter;
                putIE(ie->clone());
            }
    }

    /**
     * Creates a new instance of CompositeIE.
     * @param &code const int
     * @param *value char
     * @param &length const int
     */
    CompositeIE(const int &code, const char *value, const int &length) {
        if(code < 0 || code > 0xff) {
            std::stringstream ss;
            ss << "illegal code " << (int)code;
            throw Exception(ss.str(), __FILE__, __LINE__);
        }
        this->code=(char)code;

        int tmpIndex=0;
        while(tmpIndex < length) {
            char tmpType=value[tmpIndex] & IE_TYPE;
            char tmpLLength=value[tmpIndex] & L_LENGTH;
            int tmpCode=value[tmpIndex + 1] & 0xff;

            int tmpLength=0;
            if(tmpLLength==L_LENGTH_0) {
                if(tmpType==TYPE::BYTE.getType()) tmpLength=1;
                else if(tmpType==TYPE::SHORT.getType()) tmpLength=2;
                else if(tmpType==TYPE::INT.getType()) tmpLength=4;
                else if(tmpType==TYPE::LONG.getType()) tmpLength=8;
                tmpIndex+=2;
            }
            else if(tmpLLength==L_LENGTH_1) {
                tmpLength=value[tmpIndex + 2] & 0xff;
                tmpIndex+=3;
            }
            else if(tmpLLength==L_LENGTH_2) {
                tmpLength=value[tmpIndex + 2] & 0xff; tmpLength<<=8;
                tmpLength+=value[tmpIndex + 3] & 0xff;
                tmpIndex+=4;
            }

            // IE
            putIE(newIE(tmpType, tmpCode, value + tmpIndex, tmpLength));
            tmpIndex+=tmpLength;
        }
    }

    virtual ~CompositeIE() {
        for(std::map<int, std::vector<IE*>*>::iterator iter=ieMap.begin(); iter!=ieMap.end(); iter++) {
            std::vector<IE*> *ieVector=iter->second;
            for(std::vector<IE*>::iterator ieIter=ieVector->begin(); ieIter!=ieVector->end(); ieIter++) delete *ieIter;
            delete ieVector;
        }
    }

    virtual IE* clone() {
        std::vector<IE*> ies;
        for(std::map<int, std::vector<IE*>*>::iterator iter=ieMap.begin(); iter!=ieMap.end(); iter++) {
            std::vector<IE*> *ieVector=iter->second;
            for(std::vector<IE*>::iterator ieIter=ieVector->begin(); ieIter!=ieVector->end(); ieIter++) ies.push_back(*ieIter);
        }
        return new CompositeIE(code, &ies);
    }

    virtual char getType() {
        return TYPE::COMPOSITE.getType();
    }

    int getCode() {
        return code & 0xff;
    }

    virtual void getBytes(char *b, int &length) {
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

        char *tmp;
        int tmpIndex;
        if(tmpLength==0) {
            tmp=new char[2];
            tmp[0]=TYPE::COMPOSITE.getType();
            tmp[1]=code;
            tmpIndex=2;
        }
        else if(tmpLength<=0xff) {
            tmp=new char[3];
            tmp[0]=TYPE::COMPOSITE.getType() | L_LENGTH_1;
            tmp[1]=code;
            tmp[2]=(char)tmpLength;
            tmpIndex=3;
        }
        else if(tmpLength<=0xffff) {
            tmp=new char[4];
            tmp[0]=TYPE::COMPOSITE.getType() | L_LENGTH_2;
            tmp[1]=code;
            tmp[2]=(char)((tmpLength >> 8) & 0xff);
            tmp[3]=(char)(tmpLength & 0xff);
            tmpIndex=4;
        }
        else {
            std::stringstream ss;
            ss << "illegal length " << length;
            throw Exception(ss.str(), __FILE__, __LINE__);
        }

        for(int i=tmpLength - 1; i>=0; i--) b[i + tmpIndex]=b[i];
        for(int i=0; i < tmpIndex; i++) b[i]=tmp[i];
        length=tmpIndex + tmpLength;
        delete[] tmp;
    }

    virtual std::string toXML() {
        std::stringstream ss;
        ss << "<" << TYPE::COMPOSITE.getName() << " code=\"" << getCode() << "\">" << LINE_SEPARATOR;
        for(std::map<int, std::vector<IE*>*>::iterator iter=ieMap.begin(); iter!=ieMap.end(); iter++) {
            std::vector<IE*> *ieVector=iter->second;
            for(std::vector<IE*>::iterator ieIter=ieVector->begin(); ieIter!=ieVector->end(); ieIter++) {
                IE *ie=*ieIter;
                ss << ie->toXML();
            }
        }
        ss << "</" << TYPE::COMPOSITE.getName() << ">" << LINE_SEPARATOR;
        return ss.str();
    }

private:

    /**
     * Returns the ie for the given type, code and value.
     * @param &type const char
     * @param &code const char
     * @param *value const char
     * @param &length const int
     * @return *IE
     */
    IE* newIE(const char &type, const int &code, const char *value, const int &length) {
        if(type==TYPE::BYTE.getType()) return new ByteIE(code, value, length);
        else if(type==TYPE::SHORT.getType()) return new ShortIE(code, value, length);
        else if(type==TYPE::INT.getType()) return new IntIE(code, value, length);
        else if(type==TYPE::LONG.getType()) return new LongIE(code, value, length);
        else if(type==TYPE::BYTE_ARRAY.getType()) return new ByteArrayIE(code, value, length);
        else if(type==TYPE::COMPOSITE.getType()) return new CompositeIE(code, value, length);
        else return NULL;
    }

    /**
     * Puts the ie in this information element.
     * @param *ie IE
     */
    void putIE(IE *ie) {
        if(ie!=NULL) {
            std::map<int, std::vector<IE*>*>::iterator iter=ieMap.find(ie->getCode());
            std::vector<IE*> *ieVector;
            if(iter!=ieMap.end()) ieVector=iter->second;
            else {
                ieVector=new std::vector<IE*>;
                ieMap.insert(std::pair<int, std::vector<IE*>*>(ie->getCode(), ieVector));
            }
            ieVector->push_back(ie);
        }
    }

public:

    /**
     * Returns true if the compositeIE contains the given code.
     * @param &code const int
     * @return bool
     */
    bool containsIE(const int &code) {
        if(ieMap.find(code)!=ieMap.end()) return true;
        else return false;
    }

    /**
     * Returns the first ie for the given code.
     * @param &code const int
     * @return *IE
     */
    IE* getIE(const int &code) {
        std::map<int, std::vector<IE*>*>::iterator iter=ieMap.find(code);
        if(iter!=ieMap.end()) {
            std::vector<IE*> *ieVector=iter->second;
            if(ieVector->size() > 0) return ieVector->at(0);
        }
        return NULL;
    }

    /**
     * Returns the vector of IEs for the given code.
     * @param &code const int
     * @return *std::vector<IE*>
     */
    std::vector<IE*>* getIEVector(const int &code) {
        std::map<int, std::vector<IE*>*>::iterator iter=ieMap.find(code);
        if(iter!=ieMap.end()) return iter->second;
        else return NULL;
    }

    /**
     * Removes the vector of IEs for the given code.
     * @param &code const int
     */
    void removeIEVector(const int &code) {
        std::map<int, std::vector<IE*>*>::iterator iter=ieMap.find(code);
        if(iter!=ieMap.end()) {
            std::vector<IE*> *ieVector=iter->second;
            for(std::vector<IE*>::iterator ieIter=ieVector->begin(); ieIter!=ieVector->end(); ieIter++) delete *ieIter;
            delete ieVector;
            ieMap.erase(iter);
        }
    }

    /**
     * Returns the first char for the given code.
     * @param &code const int
     * @return char
     */
    char getByte(const int &code) {
        IE *ie=getIE(code);
        if(ie!=NULL && ie->getType()==TYPE::BYTE.getType()) return dynamic_cast<ByteIE*>(ie)->getValue();
        else return 0;
    }

    /**
     * Returns the first short for the given code.
     * @param &code const int
     * @return short
     */
    short getShort(const int &code) {
        IE *ie=getIE(code);
        if(ie!=NULL && ie->getType()==TYPE::SHORT.getType()) return dynamic_cast<ShortIE*>(ie)->getValue();
        else return 0;
    }

    /**
     * Returns the first int for the given code.
     * @param &code const int
     * @return short
     */
    int getInt(const int &code) {
        IE *ie=getIE(code);
        if(ie!=NULL && ie->getType()==TYPE::INT.getType()) return dynamic_cast<IntIE*>(ie)->getValue();
        else return 0;
    }

    /**
     * Returns the first long for the given code.
     * @param &code const int
     * @return long
     */
    long getLong(const int &code) {
        IE *ie=getIE(code);
        if(ie!=NULL && ie->getType()==TYPE::LONG.getType()) return dynamic_cast<LongIE*>(ie)->getValue();
        else return 0;
    }

    /**
     * Returns the first byte array for the given code.
     * @param &code const int
     * @param *value char
     * @param &length int
     */
    void getByteArray(const int &code, char *value, int &length) {
        IE *ie=getIE(code);
        if(ie!=NULL && ie->getType()==TYPE::BYTE_ARRAY.getType()) dynamic_cast<ByteArrayIE*>(ie)->getValue(value, length);
        else {
            value=NULL;
            length=0;
        }
    }

    /**
     * Returns the first string for the given code.
     * @param &code const int
     * @return std::string
     */
    std::string getString(const int &code) {
        IE *ie=getIE(code);
        if(ie!=NULL && ie->getType()==TYPE::BYTE_ARRAY.getType()) return dynamic_cast<ByteArrayIE*>(ie)->getString();
        else return "";
    }

    /**
     * Returns the first composite for the given code.
     * @param &code const int
     * @return CompositeIE
     */
    CompositeIE* getComposite(const int &code) {
        IE *ie=getIE(code);
        if(ie!=NULL && ie->getType()==TYPE::COMPOSITE.getType()) return dynamic_cast<CompositeIE*>(ie);
        else return NULL;
    }
};

#endif	/* COMPOSITEIE_HPP */
