/*
 * timer.hpp
 */
#ifndef TIMER_HPP
#define	TIMER_HPP

#include "thread.hpp"
#include <unistd.h>

/**
 * <p>The TimerTask class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class TimerTask {

public:

    TimerTask() {}

    virtual ~TimerTask() {}

    virtual void run()=0;
};

/**
 * <p>The Timer class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Timer : public Thread {

    /** The timerTask. */
    TimerTask *timerTask;

    /* private variables */
    long delay;
    long period;
    bool cancelled;

    /** The cancelCond. */
    boost::condition_variable cancelCond;

    /** The cancelMutex. */
    boost::mutex cancelMutex;

public:

    /**
     * Creates an unscheduled timer.
     */
    Timer() : Thread() {
        cancelled=false;
    }

    virtual ~Timer() {
        cancel();
    }

    /**
     * Cancels the timer.
     */
    void cancel() {
        {
            boost::lock_guard<boost::mutex> lock(cancelMutex);
            cancelled=true;
        }
        cancelCond.notify_one();
        join();
    }

    /**
     * Schedules the specified timerTask for execution after the specified delay.
     * @param *timerTask TimerTask
     * @param &delay const long
     */
    void schedule(TimerTask *timerTask, const long &delay) {
        cancelled=false;
        this->timerTask=timerTask;
        this->delay=delay;
        period=0;
        start();
    }

    /**
     * Schedules the specified timerTask for repeated fixed-rate execution, beginning after the specified delay.
     * @param *timerTask TimerTask
     * @param &delay const long
     * @param &period const long
     */
    void scheduleAtFixedRate(TimerTask *timerTask, const long &delay, const long &period) {
        cancelled=false;
        this->timerTask=timerTask;
        this->delay=delay;
        this->period=period;
        start();
    }

private:

    void run() {
        {
            boost::unique_lock<boost::mutex> lock(cancelMutex);
            boost::system_time delayTime=boost::get_system_time() + boost::posix_time::millisec(delay);
            while(!cancelled && boost::get_system_time() < delayTime) cancelCond.timed_wait(lock, delayTime);
        }
        if(!cancelled) {
            timerTask->run();
            if(period > 0) {
                while(true) {
                    boost::unique_lock<boost::mutex> lock(cancelMutex);
                    boost::system_time periodTime=boost::get_system_time() + boost::posix_time::millisec(period);
                    while(!cancelled && boost::get_system_time() < periodTime) cancelCond.timed_wait(lock, periodTime);
                    if(!cancelled) timerTask->run();
                    else break;
                }
            }
        }
    }
};

#endif	/* TIMER_HPP */
