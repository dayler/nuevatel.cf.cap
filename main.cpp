/*
 * main.cpp
 */
#include "cf/cap/capapp.hpp"
#include <iostream>

/**
 * The LoggerHandler class.
 */
class LoggerHandler : public Handler {

public:

    void publish(LogRecord *logRecord) {
        std::stringstream ss;
        time_t seconds;
        seconds=logRecord->getSeconds();
        struct tm *timestamp=localtime(&seconds);
        
        ss << (timestamp->tm_year + 1900) << '-';
        if((timestamp->tm_mon + 1) < 10) ss << '0'; ss << (timestamp->tm_mon + 1) << '-';
        if(timestamp->tm_mday < 10) ss << '0'; ss << timestamp->tm_mday << ' ';
        if(timestamp->tm_hour < 10) ss << '0'; ss << timestamp->tm_hour << ':';
        if(timestamp->tm_min < 10) ss << '0'; ss << timestamp->tm_min << ':';
        if(timestamp->tm_sec < 10) ss << '0'; ss << timestamp->tm_sec;
        ss << ' ' << logRecord->getLevel()->getName();
        ss << ' ' << logRecord->getSourceClass();
        ss << '.' << logRecord->getSourceMethod();
        ss << ' ' << logRecord->getMessage();
        std::cout << ss.str() << std::endl;
    }

    void flush() {}
};

/**
 * The main method.
 * @param argc int
 * @param **argv char
 * @return int
 */
int main(int argc, char** argv) {
    // logger
    LoggerHandler handler;
    Logger::getLogger()->setHandler(&handler);

    if(argc > 1) {
        Properties properties;
        properties.load(argv[1]);
        try {
            CAPApp capApp(argc, argv, &properties);
            capApp.start();
            int s=0;
            while(appState!=ONLINE) {
                sleep(1);
                if(++s==10) break;
            }
            if(appState==ONLINE) while(appState==ONLINE) sleep(1);
            else Logger::getLogger()->logp(&Level::SEVERE, "<void>", "main", "capApp OFFLINE");
            
        } catch(Exception e) {
            Logger::getLogger()->logp(&Level::SEVERE, "<void>", "main", e.toString());
        }
    }
    else std::cerr << "usage: cap <properties>" << std::endl;
    return 0;
}
