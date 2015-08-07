/*
 * logger.hpp
 */
#ifndef LOGGER_HPP
#define	LOGGER_HPP

#include <string>
#include <time.h>

namespace logging {

    /**
     * The Level class.
     */
    class Level {

        /** The value. */
        int value;
        /** The name. */
        std::string name;

        /**
         * Creates a new instance of Level.
         * @param &value const int
         * @param name const std::string
         */
        Level(const int &value, const std::string &name) {
            this->value=value;
            this->name=name;
        }

    public:

        /* constants */
        static Level OFF;
        static Level SEVERE;
        static Level WARNING;
        static Level INFO;
        static Level CONFIG;
        static Level FINE;
        static Level FINER;
        static Level FINEST;
        static Level ALL;

        /**
         * Returns the value.
         * @return int
         */
        int getValue() {
            return value;
        }

        /**
         * Returns the name.
         * @return std::string
         */
        std::string getName() {
            return name;
        }
    };

    Level Level::OFF    =Level(0xff, "OFF");
    Level Level::SEVERE =Level(0x80, "SEVERE");
    Level Level::WARNING=Level(0x70, "WARNING");
    Level Level::INFO   =Level(0x50, "INFO");
    Level Level::CONFIG =Level(0x40, "CONFIG");
    Level Level::FINE   =Level(0x30, "FINE");
    Level Level::FINER  =Level(0x20, "FINER");
    Level Level::FINEST =Level(0x10, "FINEST");
    Level Level::ALL    =Level(0x00, "ALL");

    /**
     * The LogRecord class.
     */
    class LogRecord {

        /** The level. */
        Level *level;

        /** Class that issued logging call. */
        std::string sourceClass;

        /** Method that issued logging call. */
        std::string sourceMethod;

        /** Non-localized raw message text. */
        std::string message;

        /** Event time in seconds since 1970. */
        time_t seconds;

    public:

        /**
         * Creates a new instance of LogRecord.
         * @param *level Level
         * @param &sourceClass const std::string
         * @param &sourceMethod const std::string
         * @param &message const std::string
         * @param &seconds const time_t
         */
        LogRecord(Level *level, const std::string &sourceClass, const std::string &sourceMethod, const std::string &message, const time_t &seconds) {
            this->level=level;
            this->sourceClass=sourceClass;
            this->sourceMethod=sourceMethod;
            this->message=message;
            this->seconds=seconds;
        }

        /**
         * Returns the level.
         * @return *Level
         */
        Level* getLevel() {
            return level;
        }

        /**
         * Returns the sourceClass.
         * @return std::string
         */
        std::string getSourceClass() {
            return sourceClass;
        }

        /**
         * Returns the sourceMethod.
         * @return std::string
         */
        std::string getSourceMethod() {
            return sourceMethod;
        }

        /**
         * Returns the message.
         * @return std::string
         */
        std::string getMessage() {
            return message;
        }

        /**
         * Return the seconds.
         * @return time_t
         */
        time_t getSeconds() {
            return seconds;
        }
    };

    /**
     * The Handler class.
     */
    class Handler {

    public:

        virtual ~Handler() {}

        /**
         * Publishes the logRecord.
         * @param *logRecord LogRecord
         */
        virtual void publish(LogRecord *logRecord)=0;

        /**
         * Flushes the log.
         */
        virtual void flush()=0;
    };

    /**
     * <p>The Logger singleton class.</p>
     * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
     *
     * @author Eduardo Marin
     * @version 2.0
     */
    class Logger {

        /** The logger. */
        static Logger *logger;

        /** The handler. */
        Handler *handler;

        /** The level. */
        Level *level;

        /**
         * Creates a new instance of Logger.
         */
        Logger() {
            handler=NULL;
            level=&Level::ALL;
        }

        ~Logger() {
            if(logger!=NULL) delete logger;
        }

    public:

        /**
         * Returns the logger.
         * @return *Logger
         */
        static Logger* getLogger() {
            if(logger==NULL) logger=new Logger();
            return logger;
        }

        /**
         * Log the message.
         * @param *level Level
         * @param &sourceClass const std::string
         * @param &sourceMethod const std::string
         * @param &message const std::string
         */
        void logp(Level *level, const std::string &sourceClass, const std::string &sourceMethod, const std::string &message) {
            if(handler!=NULL) {
                if(level->getValue()>=this->level->getValue()) {
                    time_t seconds;
                    time(&seconds);
                    LogRecord logRecord(level, sourceClass, sourceMethod, message, seconds);
                    handler->publish(&logRecord);
                }
            }
        }

        /**
         * Sets the handler.
         * @param *handler Handler
         */
        void setHandler(Handler *handler) {
            this->handler=handler;
        }

        /**
         * Sets the level.
         * @param *level Level
         */
        void setLevel(Level *level) {
            this->level=level;
        }
    };

    Logger* Logger::logger=NULL;
}

#endif	/* LOGGER_HPP */
