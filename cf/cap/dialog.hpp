/*
 * dialog.hpp
 */
#ifndef DIALOG_HPP
#define	DIALOG_HPP

#include "../../base/timer.hpp"
#include <capv480.h>
#include <time.h>

/**
 * <p>The Dialog abstract class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Dialog {

    /** The dialogId. */
    int dialogId;

    /** The invokeId. */
    int invokeId;
    boost::mutex invokeIdMutex;

    /** The state. */
    char state;
    boost::mutex stateMutex;

    /** The checkTime. */
    time_t checkTime;

    /** The killTime. */
    time_t killTime;

public:

    /* constants for state */
    static char NEW;
    static char OPEN;
    static char W_OPEN;
    static char INVOKE;
    static char W_INVOKE;
    static char CLOSE_0;
    static char CLOSE_1;
    static char W_CLOSE_0;
    static char W_CLOSE_1;
    static char ABORT_0;
    static char ABORT_1;
    static char KILL_0;
    static char KILL_1;

    /* constants */
    static int TIME_24S;
    static int TIME_7200S;

    /**
     * Creates a new instance of Dialog.
     */
    Dialog() {
        dialogId=0;
        invokeId=0;
        state=0;
        time(&checkTime);
        killTime=checkTime + 7200;
        checkTime+=24;
    }

    /**
     * Creates a new instance of Dialog.
     * @param &timeToKill const int
     */
    Dialog(const int &timeToKill) {
        dialogId=0;
        invokeId=0;
        state=0;
        time(&checkTime);
        killTime=checkTime + timeToKill;
        checkTime+=24;
    }

    virtual ~Dialog() {}

    /**
     * Initializes the dialog.
     */
    virtual void init()=0;

    /**
     * Checks the dialog.
     */
    virtual void check()=0;

    /**
     * Implement this method to handle the capBlock.
     * @param *capBlock CapBlock
     */
    virtual void handle(CapBlock *capBlock)=0;

    /**
     * Implement this method for dialog.
     * @return void
     */
    virtual void run()=0;

    /**
     * Sets the dialogId.
     * @param &dialogId const int
     */
    void setDialogId(const int &dialogId) {
        this->dialogId=dialogId;
    }

    /**
     * Returns the dialogId.
     * @return int
     */
    int getDialogId() {
        return dialogId;
    }

    /**
     * Sets the invokeId.
     * @param invokeId
     */
    void setInvokeId(const int &invokeId) {
        boost::lock_guard<boost::mutex> lock(invokeIdMutex);
        this->invokeId=invokeId;
    }

    /**
     * Increments invokeId.
     */
    void incrementInvokeId() {
        boost::lock_guard<boost::mutex> lock(invokeIdMutex);
        invokeId++;
    }

    /**
     * Returns the invokeId.
     * @return int
     */
    int getInvokeId() {
        boost::lock_guard<boost::mutex> lock(invokeIdMutex);
        return invokeId;
    }

    /**
     * Sets the state.
     * @param &state const char
     */
    void setState(const char &state) {
        boost::lock_guard<boost::mutex> lock(stateMutex);
        if(this->state!=CLOSE_0 &&
           this->state!=CLOSE_1 &&
           this->state!=ABORT_0 &&
           this->state!=ABORT_1 &&
           this->state!=KILL_0 &&
           this->state!=KILL_1)
            this->state=state;
        else if(this->state==CLOSE_0 && state==CLOSE_1) this->state=state;
        else if(this->state==ABORT_0 && state==ABORT_1) this->state=state;
        else if(this->state==KILL_0 && state==KILL_1) this->state=state;
    }

    /**
     * Returns the state.
     * @return char
     */
    char getState() {
        boost::lock_guard<boost::mutex> lock(stateMutex);
        return state;
    }

    /**
     * Sets the checkTime.
     * @param &checkTime const time_t
     */
    void setCheckTime(const time_t &checkTime) {
        this->checkTime=checkTime;
    }

    /**
     * Returns the checkTime.
     * @return time_t
     */
    time_t getCheckTime() {
        return checkTime;
    }

    /**
     * Returns the killTime.
     * @return time_t
     */
    time_t getKillTime() {
        return killTime;
    }
};

char Dialog::NEW=0;
char Dialog::OPEN=1;
char Dialog::W_OPEN=2;
char Dialog::INVOKE=3;
char Dialog::W_INVOKE=4;
char Dialog::CLOSE_0=5;
char Dialog::CLOSE_1=6;
char Dialog::W_CLOSE_0=7;
char Dialog::W_CLOSE_1=8;
char Dialog::ABORT_0=9;
char Dialog::ABORT_1=10;
char Dialog::KILL_0=11;
char Dialog::KILL_1=12;

int Dialog::TIME_24S=24;
int Dialog::TIME_7200S=7200;

/**
 * <p>The DialogTask class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class DialogTask : public Thread {

    /** The dialog. */
    Dialog *dialog;

public:

    /**
     * Creates a new instance of DialogTask.
     * @param *dialog Dialog
     */
    DialogTask(Dialog *dialog) {
        this->dialog=dialog;
    }

    virtual ~DialogTask() {}

private:

    void run() {
        dialog->run();
    }
};

/**
 * <p>The DialogMap class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class DialogMap {

    /* private constants */
    static int CHECK_PERIOD;

    /** The dialogMap. */
    std::map<int, Dialog*> dialogMap;
    boost::mutex dialogMapMutex;

    /** The checkTimer. */
    Timer *checkTimer;
    TimerTask *checkTimerTask;

public:

    /**
     * Creates a new instance of DialogMap.
     */
    DialogMap() {
        checkTimer=new Timer();
        checkTimerTask=new CheckTimerTask(this);
        checkTimer->scheduleAtFixedRate(checkTimerTask, CHECK_PERIOD, CHECK_PERIOD);
    }

    ~DialogMap() {
        checkTimer->cancel();
        delete checkTimer;
        delete checkTimerTask;
        for(std::map<int, Dialog*>::iterator iter=dialogMap.begin(); iter!=dialogMap.end(); iter++) delete iter->second;
    }

public:

    /**
     * Returns the dialog for the given dialogId.
     * @param &dialogId const int
     * @return *Dialog
     */
    Dialog* get(const int &dialogId) {
        boost::lock_guard<boost::mutex> lock(dialogMapMutex);
        std::map<int, Dialog*>::iterator iter=dialogMap.find(dialogId);
        if(iter!=dialogMap.end()) return iter->second;
        else return NULL;
    }

    /**
     * Puts a dialog.
     * @param *dialog Dialog
     */
    void put(Dialog *dialog) {
        if(dialog!=NULL) {
            boost::lock_guard<boost::mutex> lock(dialogMapMutex);
            std::map<int, Dialog*>::iterator iter=dialogMap.find(dialog->getDialogId());
            if(iter!=dialogMap.end()) {
                delete iter->second;
                dialogMap.erase(iter);
            }
            dialogMap.insert(std::pair<int, Dialog*>(dialog->getDialogId(), dialog));
        }
    }

    /**
     * Removes the dialog for the given dialogId.
     * @param &dialogId const int
     */
    void remove(const int &dialogId) {
        boost::lock_guard<boost::mutex> lock(dialogMapMutex);
        std::map<int, Dialog*>::iterator iter=dialogMap.find(dialogId);
        if(iter!=dialogMap.end()) {
            delete iter->second;
            dialogMap.erase(iter);
        }
    }

private:

    /**
     * Checks all dialogs.
     */
    void check() {
        std::map<int, Dialog*> tmpDialogMap;
        {
            boost::lock_guard<boost::mutex> lock(dialogMapMutex);
            tmpDialogMap=dialogMap;
        }
        time_t checkTime;
        time(&checkTime);
        for(std::map<int, Dialog*>::iterator iter=tmpDialogMap.begin(); iter!=tmpDialogMap.end(); iter++) {
            Dialog *dialog=iter->second;
            if(dialog->getCheckTime() < checkTime) {
                if(dialog->getState()==Dialog::CLOSE_1 ||
                   dialog->getState()==Dialog::ABORT_1 ||
                   dialog->getState()==Dialog::KILL_1) {
                    remove(iter->first);
                }
                else {
                    if(dialog->getKillTime() < checkTime) dialog->setState(Dialog::KILL_0);
                    dialog->check();
                    dialog->setCheckTime(checkTime + Dialog::TIME_24S);
                }
            }
        }
    }

    /**
     * The CheckTimerTask class.
     */
    class CheckTimerTask : public TimerTask {

        /** The dialogMap. */
        DialogMap *dialogMap;

    public:

        /**
         * Creates a new instance of CheckTimerTask.
         * @param *dialogMap DialogMap
         */
        CheckTimerTask(DialogMap *dialogMap) {
            this->dialogMap=dialogMap;
        }

    private:

        void run() {
            dialogMap->check();
        }
    };
};

int DialogMap::CHECK_PERIOD=16000;

#endif	/* DIALOG_HPP */
