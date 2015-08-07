/*
 * capapp.hpp
 */
#ifndef CAPAPP_HPP
#define	CAPAPP_HPP

#include "capdialog.hpp"
#include "cftask.hpp"

/** The appState. */
static char appState=0;

static char OFFLINE=0;
static char ONLINE=1;

/**
 * The sigintHandler.
 */
static void sigintHandler() {
    if(appState==ONLINE) appState=OFFLINE;
    else exit(0);
}

/** The appMutex */
boost::mutex appMutex;

/**
 * <p>The PutBlockService class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class PutBlockService : public PutBlockQueue {

    /** The dialogMap. */
    DialogMap *dialogMap;

public:

    PutBlockService(DialogMap *dialogMap) {
        this->dialogMap=dialogMap;
    }

private:

    void run() {
        try {
            // FtThreadRegister
            if(FtThreadRegister()==RETURNok) {
                while(appState==ONLINE) {
                    Block *block=blockQueue.waitAndPop();
                    CapBlock *capBlock=block->getCapBlock();
                    //capPrintBlock(capBlock);
                    Dialog *dialog;
                    dialog=block->getDialog();
                    if(dialog->getState()!=Dialog::CLOSE_0 && dialog->getState()!=Dialog::ABORT_0) {
                        int capPutBlockRet=-1;
                        {
                            boost::lock_guard<boost::mutex> lock(appMutex);
                            capPutBlockRet=capPutBlock(block->getCapBlock());
                        }
                        if(capPutBlockRet==0) {
                            if(capBlock->serviceType==capReq) {
                                if(capBlock->serviceMsg==CAP_OPEN) {
                                    dialog->setDialogId(capBlock->dialogId);
                                    dialogMap->put(dialog);
                                }
                                else if(capBlock->serviceMsg==CAP_CLOSE) {
                                    if(dialog->getState()==Dialog::KILL_0) {
                                        dialog->setState(Dialog::KILL_1);
                                        std::stringstream ss;
                                        ss << "dialog killed ";
                                        ss << std::hex << dialog->getDialogId();
                                        Logger::getLogger()->logp(&Level::WARNING, "PutBlockService", "run", ss.str());
                                    }
                                    else dialog->setState(Dialog::CLOSE_0);
                                }
                            }
                        }
                        else {
                            std::stringstream ss;
                            ss << "capPutBlock failed " << capBlock->serviceMsg << " ";
                            ss << std::hex << dialog->getDialogId();
                            Logger::getLogger()->logp(&Level::SEVERE, "PutBlockService", "run", ss.str());
                            dialog->setState(Dialog::ABORT_0);
                        }
                    }
                    delete block;
                }
            }
            else {
                std::stringstream ss;
                ss << "FtThreadRegister() failed, errno=" << errno;
                throw Exception(ss.str(), __FILE__, __LINE__);
            }
        }
        catch(Exception e) {
            Logger::getLogger()->logp(&Level::SEVERE, "PutBlockService", "run", e.toString());
        }
        // FtThreadUnregister
        FtThreadUnregister();
    }
};

/**
 * <p>The CAPApp singleton class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CAPApp : public Thread {

public:

    /* constants for properties */
    static std::string LOCAL_ID;
    static std::string REMOTE_ID;
    static std::string LOGICAL_NAME;
    static std::string SSN;
    static std::string N_DIALOGS;
    static std::string N_INVOKES;
    static std::string N_COM_BUFS;
    static std::string NODE_NAME;
    static std::string STAND_ALONE;

private:

    /** The argc. */
    int argc;

    /** The argv. */
    char** argv;

    /** The appId. */
    int appId;

    /** The localId. */
    int localId;

    /** The remoteId. */
    int remoteId;

    /** The capInit. */
    CapInit capInit;

    /* properties */
    std::string logicalName;
    bool standAlone;

    /** The dialogMap. */
    DialogMap *dialogMap;

    /** The putBlockService. */
    PutBlockService *putBlockService;

    /** The taskSet*/
    TaskSet *taskSet;
    SetSessionTask *setSessionTask;
    TestSessionTask *testSessionTask;

    /** The appClient. */
    AppClient *appClient;

    /** The dialogTaskService. */
    Executor *dialogTaskService;

    /* private variables */
    CapBlock capBlock;

public:

    /**
     * Creates a new instance of CAPApp.
     * @param &argc const int
     * @param **argv char
     * @param *properties Properties
     * @throws Exception
     */
    CAPApp(const int &argc, char** argv, Properties *properties) throw(Exception) {
        this->argc=argc;
        this->argv=argv;
        setProperties(properties);
        // dialogMap
        dialogMap=new DialogMap();
        // putBlockService
        putBlockService=new PutBlockService(dialogMap);
        // taskSet
        taskSet=new TaskSet();
        setSessionTask=new SetSessionTask(dialogMap, putBlockService);
        testSessionTask=new TestSessionTask(dialogMap, putBlockService);
        taskSet->add(CFMessage::SET_SESSION_CALL, setSessionTask);
        taskSet->add(CFMessage::TEST_SESSION_CALL, testSessionTask);
        // appClient
        appClient=new AppClient(localId, remoteId, taskSet, properties);
        // dialogTaskService
        dialogTaskService=new Executor();
    }

    ~CAPApp() {
        interrupt();
        delete dialogTaskService;
        delete appClient;
        delete setSessionTask;
        delete testSessionTask;
        delete taskSet;
        delete putBlockService;
        delete dialogMap;
    }

    /**
     * Starts this.
     */
    void start() {
        try {
            capInit.protocol=C7_PROTOCOL;
            capInit.debugFile=stdout;

            if(standAlone) {
                // FtAttach
                if(FtAttach(logicalName.c_str(), // process logical name
                            argv[0],             // process executable name
                            " ",                 // execution parameters
                            0,                   // execution priority
                            0,                   // RT time quantum
                            0,                   // RT time quantum
                            0,                   // process class identifier
                            10)==-1) {           // max. wait for CPT entry
                    std::stringstream ss;
                    ss << "FTAttach() failed, errno=" << errno + "(" << LastErrorReport << ")";
                    throw Exception(ss.str(), __FILE__, __LINE__);
                }
            }

            // FtRegister
            if(FtRegister(argc,             // command Line Argument count
                          argv,             // command Line Arguments
                          FALSE,            // Debug Printouts Required ?
                          FALSE,            // msg Activity Monitor Required ?
                          TRUE,             // Ipc Queue Required ?
                          TRUE,             // flush Ipc Queue Before Start ?
                          FALSE,            // allow Ipc Msg Queueing Always
                          TRUE,             // process Has SIGINT Handler
                          (U16)AUTOrestart, // automatic Restart allowed ?
                          0,                // process Class Designation
                          0,                // initial Process State Declaration
                          0,                // event Distribution Filter Value
                          10)==-1) {        // retry
                std::stringstream ss;
                ss << "FtRegister() failed, errno=" << errno + "(" << LastErrorReport << ")";
                throw Exception(ss.str(), __FILE__, __LINE__);
            }

            // FtAssignHandler
            if(FtAssignHandler(SIGINT, sigintHandler)==RETURNerror) {
                std::stringstream ss;
                ss << "cannot assign SIGINT handler, errno=" << errno;
                throw Exception(ss.str(), __FILE__, __LINE__);
            }

            // SYSattach
            int gAliasNameIndex=SYSattach(capInit.nodeName, FALSE);
            if(gAliasNameIndex==RETURNerror) {
                std::stringstream ss;
                ss << "SYSattach() failed, errno=" << errno + "(" << LastErrorReport << ")";
                throw Exception(ss.str(), __FILE__, __LINE__);
            }

            // SYSbind
            if(SYSbind(gAliasNameIndex,
                       FALSE,              // non-designatable
                       MTP_SCCP_TCAP_USER,
                       capInit.ssn,
                       SCCP_TCAP_CLASS)!=RETURNok) {
                std::stringstream ss;
                ss << "SYSbind() failed, errno=" << errno + "(" << LastErrorReport << ")";
                throw Exception(ss.str(), __FILE__, __LINE__);
            }

            // capInitialize
            appId=capInitialize(&capInit, argc, argv);

            if(appId==-1) {
                std::stringstream ss;
                ss << "capInitialize() failed, errno=" << errno + "(" << capInit.errorReport << ")";
                throw Exception(ss.str(), __FILE__, __LINE__);
            }
            else {
                std::stringstream ss;
                ss << "capInitialize(), appId=" << appId;
                Logger::getLogger()->logp(&Level::INFO, "CAPApp", "start", ss.str());
            }

            // CscUIS
            CscUIS(gAliasNameIndex, capInit.ssn);

            Thread::start();

            // ONLINE
            appState=ONLINE;

            appClient->start();
            putBlockService->start();
        }
        catch(Exception e) {
            Logger::getLogger()->logp(&Level::SEVERE, "CAPApp", "start", e.toString());
            interrupt();
        }
    }

    /**
     * Interrupts this.
     */
    void interrupt() {
        appState=OFFLINE;
        putBlockService->join();
        appClient->interrupt();
        FtTerminate(NOrestart, 1);
    }

private:

    void run() {
        try {
            // FtThreadRegister
            if(FtThreadRegister()==RETURNok) {
                union {
                    Header_t hdr;
                    cblock_t cblock;
                    char ipc[MAXipcBUFFERsize];
                } ipc;
                while(appState==ONLINE) {
                    if(FtGetIpcEx(&ipc.hdr,
                                  0,                    // any message type
                                  sizeof(ipc),          // max. size to rcv
			          TRUE,                 // truncate if large
			          TRUE,                 // blocking read
			          TRUE)==RETURNerror) { // interruptible
                        if(errno!=EINTR) {              // not interrupt
                            std::stringstream ss;
                            ss << "FtGetIpcEx() failed, errno=" << errno;
                            Logger::getLogger()->logp(&Level::SEVERE, "CAPApp", "run", ss.str());
                        }
                    }
                    else {
                        switch(ipc.hdr.messageType) {
                            case N_NOTICE_IND:
                            case N_UNITDATA_IND: {
                                boost::lock_guard<boost::mutex> lock(appMutex);
                                capTakeMsg(&ipc.cblock);
                            }
                                break;
                            case N_STATE_IND: {
                                std::stringstream ss;
                                scmg_nstate_t *nstate;
                                nstate=&((iblock_t *)&ipc.hdr)->primitives.nstate;
                                ss << "N_STATE_IND PC=" << nstate->NS_affect_pc << " SSN=" << (int)nstate->NS_affect_ssn << " ";
                                if(nstate->NS_user_status==SCMG_UIS) ss << "UIS";
                                else if(nstate->NS_user_status==SCMG_UOS) ss << "UOS";
                                Logger::getLogger()->logp(&Level::WARNING, "CAPApp", "run", ss.str());
                            }
                                break;
                            case N_PCSTATE_IND: {
                                std::stringstream ss;
                                scmg_pcstate_t *pcstate;
                                pcstate=&((iblock_t *)&ipc.hdr)->primitives.pcstate;
                                ss << "N_PCSTATE_IND PC=" << pcstate->pc_pc << " ";
                                if(pcstate->pc_status==SCMG_INACCESSABLE) ss << "INACCESSABLE";
                                else if(pcstate->pc_status==SCMG_ACCESSABLE) ss << "ACCESSABLE";
                                Logger::getLogger()->logp(&Level::WARNING, "CAPApp", "run", ss.str());
                            }
                                break;
                            case TAP_STATE_CHANGE:
                                Logger::getLogger()->logp(&Level::INFO, "CAPApp", "run", "TAP_STATE_CHANGE received");
                                break;
                            default: {
                                std::stringstream ss;
                                ss << "unknown ipc messageType received " << ipc.hdr.messageType;
                                Logger::getLogger()->logp(&Level::WARNING, "CAPApp", "run", ss.str());
                            }
                                break;
                        }
                    }
                    {
                        boost::lock_guard<boost::mutex> lock(appMutex);
                        while(capGetBlock(&capBlock)==0) handle(&capBlock);
                    }
                }
            }
            else {
                std::stringstream ss;
                ss << "FtThreadRegister() failed, errno=" << errno;
                throw Exception(ss.str(), __FILE__, __LINE__);
            }
        }
        catch(Exception e) {
            Logger::getLogger()->logp(&Level::SEVERE, "CAPApp", "run", e.toString());
        }
        interrupt();
        // FtThreadUnregister
        FtThreadUnregister();
    }

    /**
     * Handles the capBlock.
     * @param *capBlock CapBlock
     */
    void handle(CapBlock *capBlock) {
        //capPrintBlock(capBlock);
        if(capBlock->serviceType==capRsp || capBlock->serviceType==capError || capBlock->serviceType==capProviderError) {
            Dialog *dialog=dialogMap->get(capBlock->dialogId);
            if(dialog!=NULL) dialog->handle(capBlock);
        }
        else if(capBlock->serviceType==capReq) {
            if(capBlock->serviceMsg==CAP_OPEN) {
                CAPDialog *dialog=new CAPDialog(appId, localId, putBlockService, appClient, dialogTaskService);
                dialog->setDialogId(capBlock->dialogId);
                dialog->init();
                dialogMap->put(dialog);
                dialog->handle(capBlock);
            }
            else {
                Dialog *dialog=dialogMap->get(capBlock->dialogId);
                if(dialog!=NULL) dialog->handle(capBlock);
            }
        }
    }

private:

    /**
     * Sets the properties.
     * @param *properties Properties
     */
    void setProperties(Properties *properties) {
        if(properties==NULL) throw Exception("null properties", __FILE__, __LINE__);

        // localId
        std::string strLocalId=properties->getProperty(LOCAL_ID);
        if(strLocalId.length() > 0) localId=atoi(strLocalId.c_str());
        else throw Exception("illegal " + LOCAL_ID, __FILE__, __LINE__);

        // remoteId
        std::string strRemoteId=properties->getProperty(REMOTE_ID);
        if(strRemoteId.length() > 0) remoteId=atoi(strRemoteId.c_str());
        else throw Exception("illegal " + REMOTE_ID, __FILE__, __LINE__);

        // logicalName
        logicalName=properties->getProperty(LOGICAL_NAME);
        if(logicalName.length()==0) throw Exception("illegal " + LOGICAL_NAME, __FILE__, __LINE__);

        // capInit.ssn
        std::string strSSN=properties->getProperty(SSN, "146");
        if(strSSN.length() > 0) capInit.ssn=(unsigned char)atoi(strSSN.c_str());
        else throw Exception("illegal " + SSN, __FILE__, __LINE__);

        // capInit.nDialogs
        std::string strNDialogs=properties->getProperty(N_DIALOGS, "16383");
        if(strNDialogs.length() > 0) capInit.nDialogs=atoi(strNDialogs.c_str());
        else throw Exception("illegal " + N_DIALOGS, __FILE__, __LINE__);

        // capInit.nInvokes
        std::string strNInvokes=properties->getProperty(N_INVOKES, "16383");
        if(strNInvokes.length() > 0) capInit.nInvokes=atoi(strNInvokes.c_str());
        else throw Exception("illegal " + N_INVOKES, __FILE__, __LINE__);

        // capInit.nComBufs
        std::string strNComBufs=properties->getProperty(N_COM_BUFS, "16");
        if(strNComBufs.length() > 0) capInit.nComBufs=atoi(strNComBufs.c_str());
        else throw Exception("illegal " + N_COM_BUFS, __FILE__, __LINE__);

        // capInit.nodeName
        std::string nodeName=properties->getProperty(NODE_NAME);
        if(nodeName.length() > 0) strcpy(capInit.nodeName, nodeName.c_str());
        else throw Exception("illegal " + NODE_NAME, __FILE__, __LINE__);

        // standAlone
        std::string strStandAlone=properties->getProperty(STAND_ALONE, "false");
        if(strStandAlone.compare("true")==0) standAlone=true;
        else standAlone=false;
    }
};

std::string CAPApp::LOCAL_ID="localId";
std::string CAPApp::REMOTE_ID="remoteId";
std::string CAPApp::LOGICAL_NAME="logicalName";
std::string CAPApp::SSN="SSN";
std::string CAPApp::N_DIALOGS="nDialogs";
std::string CAPApp::N_INVOKES="nInvokes";
std::string CAPApp::N_COM_BUFS="nComBufs";
std::string CAPApp::NODE_NAME="nodeName";
std::string CAPApp::STAND_ALONE="standAlone";

#endif	/* CAPAPP_HPP */
