/*
 * exception.hpp
 */
#ifndef EXCEPTION_HPP
#define	EXCEPTION_HPP

#include <sstream>
#include <string>

/**
 * <p>The Exception class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Exception {

protected:

    /* protected variables */
    std::string name;
    std::string message;
    std::string filename;
    int line;

public:

    /**
     * Creates a new instance of Exception.
     */
    Exception() {
        this->name="Exception";
        this->message="unknown exception";
        this->filename="";
        this->line=0;
    }

    /**
     * Creates a new instance of Exception.
     * @param &message const std::string
     * @param &filename const std::string
     * @param &line const int
     */
    Exception(const std::string &message, const std::string &filename, const int &line) {
        this->name="Exception";
        this->message=message;
        this->filename=filename;
        this->line=line;
    }

    virtual ~Exception() {}

    /**
     * Returns the name.
     * @return std::string
     */
    std::string getName() {
        return name;
    }

    /**
     * Returns the message.
     * @return message
     */
    std::string getMessage() {
        return message;
    }

    /**
     * Returns the filename.
     * @return filename
     */
    std::string getFilename() {
        return filename;
    }

    /**
     * Returns the line.
     * @return line
     */
    int getLine() {
        return line;
    }

    /**
     * Returns the string representation.
     * @return std::string
     */
    std::string toString() {
        std::stringstream ss;
        ss << name << ": " << message << " " << filename << ":" << line;
        return ss.str();
    }
};

#endif	/* EXCEPTION_HPP */
