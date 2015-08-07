/*
 * ie.hpp
 */
#ifndef IE_HPP
#define	IE_HPP

#include <string>

/**
 * <p>The IE abstract class should be used to define information elements.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class IE {

public:

    /* constants */
    static char L_LENGTH;
    /* lLength 2 bit 0 and 1 */
    static char L_LENGTH_0;
    static char L_LENGTH_1;
    static char L_LENGTH_2;

    static char IE_TYPE;

    /** Line separator. */
    static std::string LINE_SEPARATOR;

    virtual ~IE() {}

    /**
     * Creates and returns a copy of this.
     * @return *IE
     */
    virtual IE* clone()=0;

    /**
     * Returns the type.
     * @return char
     */
    virtual char getType()=0;

    /**
     * Returns the code.
     * @return int
     */
    virtual int getCode()=0;

    /**
     * Returns the byte array.
     * @param *ch char
     * @param &length int
     */
    virtual void getBytes(char *b, int &length)=0;

    /**
     * Returns the XML.
     * @return std::string
     */
    virtual std::string toXML()=0;
};

char IE::L_LENGTH=0x3;

char IE::L_LENGTH_0=0x0;
char IE::L_LENGTH_1=0x1;
char IE::L_LENGTH_2=0x2;

char IE::IE_TYPE=0xf0;

std::string IE::LINE_SEPARATOR("\r\n");

/**
 * IE type.
 */
class TYPE {

    /** The type. */
    char type;
    /** The name. */
    std::string name;

    TYPE(const char &type, const std::string &name) {
        this->type=type;
        this->name=name;
    }

public:

    /* type */
    static TYPE BYTE;           // byte
    static TYPE SHORT;          // short
    static TYPE INT;            // int
    static TYPE LONG;           // long
    static TYPE BYTE_ARRAY;     // byteArray
    static TYPE COMPOSITE;      // composite

    /**
     * Returns the type.
     * @return char
     */
    char getType() {
        return type;
    }

    /**
     * Returns the name.
     * @return std::string
     */
    std::string getName() {
        return name;
    }
};

TYPE TYPE::BYTE      =TYPE(0x00, "byte");
TYPE TYPE::SHORT     =TYPE(0x10, "short");
TYPE TYPE::INT       =TYPE(0x20, "int");
TYPE TYPE::LONG      =TYPE(0x30, "long");
TYPE TYPE::BYTE_ARRAY=TYPE(0x80, "byteArray");
TYPE TYPE::COMPOSITE =TYPE(0xf0, "composite");

#endif	/* IE_HPP */
