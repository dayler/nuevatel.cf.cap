/*
 * thread.hpp
 */
#ifndef THREAD_HPP
#define	THREAD_HPP

#include <boost/thread.hpp>

/**
 * <p>The Thread class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Thread {

    /** The thread. */
    boost::thread *thread;

    /** The state. */
    char state;

public:

    /* constants for state */
    static char NEW;
    static char RUNNABLE;
    static char TERMINATED;

    /**
     * Creates a thread.
     */
    Thread() {
        thread=NULL;
        state=0;
    }

    virtual ~Thread() {
        join();
        if(thread!=NULL) delete thread;
    }

    /**
     * Starts this.
     */
    virtual void start() {
        state=RUNNABLE;
        thread=new boost::thread(&Thread::func, this);
    }

    /**
     * Returns the state.
     * @return unsigned char
     */
    unsigned char getState() {
        return state;
    }

    void join() {
        if(state!=NEW && thread!=NULL) thread->join();
    }

protected:

    /**
     * Implement this method for run.
     */
    virtual void run()=0;

private:

    /**
     * Thread method.
     */
    void func() {
        run();
        state=TERMINATED;
    }
};

char Thread::NEW=0;
char Thread::RUNNABLE=1;
char Thread::TERMINATED=2;

#endif	/* THREAD_HPP */
