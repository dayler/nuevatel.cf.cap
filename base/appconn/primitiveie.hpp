/*
 * primitiveie.hpp
 */
#ifndef PRIMITIVEIE_HPP
#define	PRIMITIVEIE_HPP

#include "ie.hpp"
#include "../exception.hpp"

/**
 * <p>The PrimitiveIE abstract class should be used to define primitive IE type.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class PrimitiveIE : public IE {

protected:

    /** The code. */
    char code;

    /**
     * Constructor for use by subclasses.
     * @param &code const int
     */
    PrimitiveIE(const int &code) {
        if(code < 0 || code > 0xff) {
            std::stringstream ss;
            ss << "illegal code " << (int)code;
            throw Exception(ss.str(), __FILE__, __LINE__);
        }
        this->code=(char)code;
    }

    virtual ~PrimitiveIE() {}

public:

    int getCode() {
        return code & 0xff;
    }
};

/**
 * <p>The ByteIE class should be used to define byte IE type.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class ByteIE : public PrimitiveIE {

    /** The value. */
    char value;

public:

    /**
     * Creates a new instance of ByteIE.
     * @param &code const int
     * @param &value const char
     */
    ByteIE(const int &code, const char &value) : PrimitiveIE(code) {
        this->value=value;
    }

    /**
     * Creates a new instance of ByteIE.
     * @param &code const int
     * @param *value char
     * @param &length const int
     */
    ByteIE(const int &code, const char *value, const int &length) : PrimitiveIE(code) {
        if(length!=1) throw Exception("illegal value", __FILE__, __LINE__);
        this->value=*value;
    }

    IE* clone() {
        return new ByteIE(code, value);
    }

    char getType() {
        return TYPE::BYTE.getType();
    }

    void getBytes(char *b, int &length) {
        length=3;
        b[0]=TYPE::BYTE.getType();
        b[1]=code;
        b[2]=value;
    }

    std::string toXML() {
        std::stringstream ss;
        ss << "<" << TYPE::BYTE.getName() << " code=\"" << getCode() << "\" value=\"" << (int)value << "\"/>" << LINE_SEPARATOR;
        return ss.str();
    }

    /**
     * Returns the value.
     * @return char
     */
    char getValue() {
        return value;
    }
};

/**
 * <p>The ShortIE class should be used to define short IE type.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class ShortIE : public PrimitiveIE {

    /** The value. */
    short value;

public:

    /**
     * Creates a new instance of ShortIE.
     * @param &code const int
     * @param &value const short
     */
    ShortIE(const int &code, const short &value) : PrimitiveIE(code) {
        this->value=value;
    }

    /**
     * Creates a new instance of ShortIE.
     * @param &code const int
     * @param *value char
     * @param &length const int
     */
    ShortIE(const int &code, const char *value, const int &length) : PrimitiveIE(code) {
        if(length!=2) throw Exception("illegal value", __FILE__, __LINE__);
        this->value=(((value[0] & 0xff) << 8) |
		     (value[1] & 0xff));
    }

    IE* clone() {
        return new ShortIE(code, value);
    }

    char getType() {
        return TYPE::SHORT.getType();
    }

    void getBytes(char *b, int &length) {
        length=4;
        b[0]=TYPE::SHORT.getType();
        b[1]=code;
        b[2]=(char)((value >> 8) & 0xff);
        b[3]=(char)(value & 0xff);
    }

    std::string toXML() {
        std::stringstream ss;
        ss << "<" << TYPE::SHORT.getName() << " code=\"" << getCode() << "\" value=\"" << value << "\"/>" << LINE_SEPARATOR;
        return ss.str();
    }

    /**
     * Returns the value.
     * @return short
     */
    short getValue() {
        return value;
    }
};

/**
 * <p>The IntIE class should be used to define int IE type.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class IntIE : public PrimitiveIE {

    /** The value. */
    int value;

public:

    /**
     * Creates a new instance of IntIE.
     * @param &code const int
     * @param &value const int
     */
    IntIE(const int &code, const int &value) : PrimitiveIE(code) {
        this->value=value;
    }

    /**
     * Creates a new instance of IntIE.
     * @param &code const int
     * @param *value char
     * @param &length const int
     */
    IntIE(const int &code, const char *value, const int &length) : PrimitiveIE(code) {
        if(length!=4) throw Exception("illegal value", __FILE__, __LINE__);
        this->value=(((value[0] & 0xff) << 24) |
		     ((value[1] & 0xff) << 16) |
		     ((value[2] & 0xff) << 8) |
		     (value[3] & 0xff));
    }

    IE* clone() {
        return new IntIE(code, value);
    }

    char getType() {
        return TYPE::INT.getType();
    }

    void getBytes(char *b, int &length) {
        length=6;
        b[0]=TYPE::INT.getType();
        b[1]=code;
        b[2]=(char)((value >> 24) & 0xff);
        b[3]=(char)((value >> 16) & 0xff);
        b[4]=(char)((value >> 8) & 0xff);
        b[5]=(char)(value & 0xff);
    }

    std::string toXML() {
        std::stringstream ss;
        ss << "<" << TYPE::INT.getName() << " code=\"" << getCode() << "\" value=\"" << value << "\"/>" << LINE_SEPARATOR;
        return ss.str();
    }

    /**
     * Returns the value.
     * @return int
     */
    int getValue() {
        return value;
    }
};

/**
 * <p>The LongIE class should be used to define long IE type.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class LongIE : public PrimitiveIE {

    /** The value. */
    long value;

public:

    /**
     * Creates a new instance of LongIE.
     * @param &code const int
     * @param &value const long
     */
    LongIE(const int &code, const long &value) : PrimitiveIE(code) {
        this->value=value;
    }

    /**
     * Creates a new instance of LongIE.
     * @param &code const int
     * @param *value char
     * @param &length const int
     */
    LongIE(const int &code, const char *value, const int &length) : PrimitiveIE(code) {
        if(length!=8) throw Exception("illegal value", __FILE__, __LINE__);
        this->value=(((long)(value[0] & 0xff) << 56) |
                     ((long)(value[1] & 0xff) << 48) |
		     ((long)(value[2] & 0xff) << 40) |
		     ((long)(value[3] & 0xff) << 32) |
                     ((long)(value[4] & 0xff) << 24) |
		     ((long)(value[5] & 0xff) << 16) |
		     ((long)(value[6] & 0xff) << 8) |
		     (long)(value[7] & 0xff));
    }

    IE* clone() {
        return new LongIE(code, value);
    }

    char getType() {
        return TYPE::LONG.getType();
    }

    void getBytes(char *b, int &length) {
        length=10;
        b[0]=TYPE::LONG.getType();
        b[1]=code;
        b[2]=(char)((value >> 56) & 0xff);
        b[3]=(char)((value >> 48) & 0xff);
        b[4]=(char)((value >> 40) & 0xff);
        b[5]=(char)((value >> 32) & 0xff);
        b[6]=(char)((value >> 24) & 0xff);
        b[7]=(char)((value >> 16) & 0xff);
        b[8]=(char)((value >> 8) & 0xff);
        b[9]=(char)(value & 0xff);
    }

    std::string toXML() {
        std::stringstream ss;
        ss << "<" << TYPE::LONG.getName() << " code=\"" << getCode() << "\" value=\"" << value << "\"/>" << LINE_SEPARATOR;
        return ss.str();
    }

    /**
     * Returns the value.
     * @return long
     */
    long getValue() {
        return value;
    }
};

static char hexCh[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

/**
 * Returns the hexadecimal string for the char array.
 * @param *b const char
 * @param &length const int
 * @return std::string
 */
static std::string toHexString(const char *b, const int &length) {
    std::stringstream ss;
    for(int i=0; i < length; i++) {
        ss << hexCh[(b[i] >> 4) & 0xf];
        ss << hexCh[b[i] & 0xf];
    }
    return ss.str();
}

/**
 * Returns the char for the hexadecimal char.
 * @param ch
 * @return char
 */
static char hexToByte(const char &ch) {
    if('0'<=ch && ch<='9') return (char)(ch - '0');
    if('a'<=ch && ch<='f') return (char)(ch - 'a' + 10);
    if('A'<=ch && ch<='F') return (char)(ch - 'A' + 10);
    return -1;
}

/**
 * Returns the char array for the hexadecimal string.
 * @param &s const std::string
 * @param *b char
 * @param &length int
 */
static void toByteArray(const std::string &s, char *b, int &length) {
    int len=s.length();
    if((len & 1)!=0) throw Exception("illegal string length " + s, __FILE__, __LINE__);

    for(int i=0; i < len; i+=2) {
        char h=hexToByte(s.at(i));
        char l=hexToByte(s.at(i + 1));
        if(h==-1 || l==-1) throw Exception("illegal character " + s, __FILE__, __LINE__);
        b[i >> 1]=(char)((h << 4) + l);
    }
    length=len >> 1;
}

/**
 * <p>The ByteArrayIE class should be used to define byte array IE type.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class ByteArrayIE : public PrimitiveIE {

    /** The value. */
    char *value;
    int length;

public:

    /**
     * Creates a new instance of ByteArrayIE.
     * @param &code const int
     * @param *value const char
     * @param &length const int
     */
    ByteArrayIE(const int &code, const char *value, const int &length) : PrimitiveIE(code) {
        this->length=length;
        if(length > 0) {
            this->value=new char[length];
            for(int i=0; i < length; i++) this->value[i]=value[i];
        }
    }

    /**
     * Creates a new instance of ByteArrayIE.
     * @param &code const int
     * @param &str const std::string
     */
    ByteArrayIE(const int &code, const std::string &str) : PrimitiveIE(code) {
        length=str.length();
        if(length > 0) {
            value=new char[length];
            for(int i=0; i < length; i++) value[i]=str.at(i);
        }
    }

    ~ByteArrayIE() {
        if(length > 0) delete[] value;
    }

    IE* clone() {
        return new ByteArrayIE(code, value, length);
    }

    char getType() {
        return TYPE::BYTE_ARRAY.getType();
    }

    void getBytes(char *b, int &length) {
        int tmpIndex;
        if(this->length<=0) {
            b[0]=TYPE::BYTE_ARRAY.getType();
            b[1]=code;
            tmpIndex=2;
        }
        else if(this->length<=0xff) {
            b[0]=TYPE::BYTE_ARRAY.getType() | L_LENGTH_1;
            b[1]=code;
            b[2]=(char)this->length;
            tmpIndex=3;
        }
        else if(this->length<=0xffff) {
            b[0]=TYPE::BYTE_ARRAY.getType() | L_LENGTH_2;
            b[1]=code;
            b[2]=(char)((this->length >> 8) & 0xff);
            b[3]=(char)(this->length & 0xff);
            tmpIndex=4;
        }
        else {
            std::stringstream ss;
            ss << "illegal length " << this->length;
            throw Exception(ss.str(), __FILE__, __LINE__);
        }

        for(int i=0; i < this->length; i++) b[i + tmpIndex]=value[i];
        length=this->length + tmpIndex;
    }

    std::string toXML() {
        std::stringstream ss;
        ss << "<" << TYPE::BYTE_ARRAY.getName() << " code=\"" << getCode() << "\" value=\"" << getHexString() << "\"/>" << LINE_SEPARATOR;
        return ss.str();
    }

    /**
     * Returns the value.
     * @return long
     */
    void getValue(char *value, int &length) {
        length=this->length;
        for(int i=0; i < length; i++) value[i]=this->value[i];
    }

    /**
     * Returns the string.
     * @return std::string
     */
    std::string getString() {
        std::stringstream ss;
        for(int i=0; i < length; i++) ss << value[i];
        return ss.str();
    }

    /**
     * Returns the hexadecimal string.
     * @return std::string
     */
    std::string getHexString() {
        return toHexString(value, length);
    }
};

#endif	/* PRIMITIVEIE_HPP */
