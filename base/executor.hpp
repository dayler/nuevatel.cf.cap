/*
 * executor.hpp
 */
#ifndef EXECUTOR_HPP
#define	EXECUTOR_HPP

#include "timer.hpp"
#include <vector>

/**
 * <p>The Executor class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Executor {

    /** The threadVector. */
    std::vector<Thread*> threadVector;
    unsigned int threadIndex;

    /** The deleteTimer. */
    Timer *deleteTimer;
    TimerTask *deleteTimerTask;

    /** The executorMutex. */
    boost::mutex executorMutex;

public:

    /**
     * Creates a new instance of Executor.
     */
    Executor() {
        deleteTimer=new Timer();
        deleteTimerTask=new DeleteTimerTask(this);
        deleteTimer->scheduleAtFixedRate(deleteTimerTask, 64000, 64000);
        threadIndex=0;
    }

    virtual ~Executor() {
        delete deleteTimer;
        delete deleteTimerTask;
        for(unsigned int i=0; i < threadVector.size(); i++) delete threadVector.at(i);
    }

    /**
     * Submits the thread for execution. The thread pointer will be deleted after execution.
     * @param *thread Thread
     */
    void submit(Thread *thread) {
        boost::lock_guard<boost::mutex> lock(executorMutex);
        while(threadIndex < threadVector.size()) {
            Thread *tmpThread=threadVector.at(threadIndex);
            if(tmpThread->getState()==Thread::TERMINATED) {
                delete tmpThread;
                threadVector.at(threadIndex)=thread;
                thread->start();
                if(++threadIndex==threadVector.size()) threadIndex=0;
                return;
            }
            else threadIndex++;
        }
        threadVector.push_back(thread);
        threadIndex=0;
        thread->start();
    }

private:

    /**
     * Deletes terminated threads.
     */
    void deleteTerminated() {
        boost::lock_guard<boost::mutex> lock(executorMutex);
        std::vector<Thread*>::iterator iter;
        iter=threadVector.begin();
        while(iter!=threadVector.end()) {
            Thread *thread=*iter;
            if(thread->getState()==Thread::TERMINATED) {
                delete thread;
                iter=threadVector.erase(iter);
            }
            else iter++;
        }
        threadIndex=0;
    }

    /**
     * The DeleteTimerTask class.
     */
    class DeleteTimerTask : public TimerTask {

        /** The executor. */
        Executor *executor;

    public:

        /**
         * Creates a new instance of DeleteTimerTask.
         * @param *executor Executor
         */
        DeleteTimerTask(Executor *executor) {
            this->executor=executor;
        }

    private:

        void run() {
            executor->deleteTerminated();
        }
    };
};

#endif	/* EXECUTOR_HPP */
