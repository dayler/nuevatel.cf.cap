/*
 * cftask.hpp
 */
#ifndef CFTASK_HPP
#define	CFTASK_HPP

#include "block.hpp"
#include "appconn/cfie.hpp"
#include "appconn/cfmessage.hpp"
#include "../../base/appconn/appclient.hpp"

/**
 * <p>The SetSessionTask class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class SetSessionTask : public Task {

    /** The dialogMap. */
    DialogMap *dialogMap;

    /** The putBlockQueue. */
    PutBlockQueue *putBlockQueue;

public:

    /**
     * Creates a new instance of SetSessionTask.
     * @param *dialogMap DialogMap
     * @param *putBlockQueue PutBlockQueue
     */
    SetSessionTask(DialogMap *dialogMap, PutBlockQueue *putBlockQueue) {
        this->dialogMap=dialogMap;
        this->putBlockQueue=putBlockQueue;
    }

    Message* execute(Conn *conn, Message *msg) throw(Exception) {
        // id
        CompositeIE *idIE=msg->getComposite(CFIE::ID_IE);
        if(idIE!=NULL) {
            Id id(idIE);
            Dialog *dialog=dialogMap->get(id.getId1());
            if(dialog!=NULL) {
                // type
                char type=msg->getByte(CFIE::TYPE_IE);
                // action
                char action=msg->getByte(CFIE::ACTION_IE);

                // END_MEDIA
                if(MEDIA_ACTION::getMediaAction(action)==MEDIA_ACTION::END_MEDIA.getAction()) {
                    putBlockQueue->push(new DisconnectForwardConnectionReqBlock(dialog, dialog->getInvokeId()));
                    dialog->incrementInvokeId();
                }
                // ACCEPT
                if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::ACCEPT.getAction()) {
                    // SPEECH VIDEO
                    if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::SPEECH.getType() || SERVICE_TYPE::getType(type)==SERVICE_TYPE::VIDEO.getType()) {
                        if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::O.getType() || REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWO.getType())
                            putBlockQueue->push(new RequestReportBCSMEventReqBlock(dialog, dialog->getInvokeId(), RequestReportBCSMEventReqBlock::O_ALL));
                        else putBlockQueue->push(new RequestReportBCSMEventReqBlock(dialog, dialog->getInvokeId(), RequestReportBCSMEventReqBlock::T_ALL));
                        dialog->incrementInvokeId();
                        putBlockQueue->push(new ContinueReqBlock(dialog, dialog->getInvokeId()));
                        dialog->incrementInvokeId();
                        putBlockQueue->push(new DelimiterReqBlock(dialog, 0));
                    }
                    dialog->setState(Dialog::W_INVOKE);
                }
                // MODIFY
                else if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::MODIFY.getAction()) {
                    // SPEECH VIDEO
                    if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::SPEECH.getType() || SERVICE_TYPE::getType(type)==SERVICE_TYPE::VIDEO.getType()) {
                        // sessionArg
                        CompositeIE *sessionArgIE=msg->getComposite(CFIE::SESSION_ARG_IE);
                        if(sessionArgIE!=NULL) {
                            SessionArg sessionArg(sessionArgIE);
                            if(sessionArg.getToName()!=NULL) {
                                CalledPartyNumber calledPartyNumber(sessionArg.getToName()->getType(), CalledPartyNumber::NPI_ISDN, sessionArg.getToName()->getName(), CalledPartyNumber::INN_RINNA);
                                GenericNumber *genericNumber=NULL;
                                if(sessionArg.getFromName()!=NULL) genericNumber=new GenericNumber(sessionArg.getFromName()->getType(), GenericNumber::NPI_ISDN, sessionArg.getFromName()->getName(), GenericNumber::NI_COMPLETE, GenericNumber::APRI_PRESENTATION_ALLOWED, GenericNumber::SI_USER_PROVIDED_NV, GenericNumber::NQI_ADDITIONAL_CALLING_PARTY_NUMBER);

                                if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::O.getType() || REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWO.getType())
                                    putBlockQueue->push(new RequestReportBCSMEventReqBlock(dialog, dialog->getInvokeId(), RequestReportBCSMEventReqBlock::O_ALL));
                                else putBlockQueue->push(new RequestReportBCSMEventReqBlock(dialog, dialog->getInvokeId(), RequestReportBCSMEventReqBlock::T_ALL));
                                dialog->incrementInvokeId();
                                putBlockQueue->push(new ConnectReqBlock(dialog, dialog->getInvokeId(), &calledPartyNumber, genericNumber));
                                dialog->incrementInvokeId();
                                putBlockQueue->push(new DelimiterReqBlock(dialog, 0));
                                if(genericNumber!=NULL) delete genericNumber;
                            }
                        }
                    }
                    dialog->setState(Dialog::W_INVOKE);
                }
                // END
                else if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::END.getAction()) {
                    // SPEECH VIDEO
                    if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::SPEECH.getType() || SERVICE_TYPE::getType(type)==SERVICE_TYPE::VIDEO.getType()) {
                        Cause cause(Cause::CS_ITU_T, Cause::L_USER, Cause::CAUSE_NORMAL_CALL_CLEARING);
                        putBlockQueue->push(new ReleaseCallReqBlock(dialog, dialog->getInvokeId(), &cause));
                        putBlockQueue->push(new CloseReqBlock(dialog));
                    }
                }
                return SetSessionRet(AppMessages::ACCEPTED).toMessage();
            }
        }
        return SetSessionRet(AppMessages::FAILED).toMessage();
    }
};

/**
 * <p>The TestSessionTask class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class TestSessionTask : public Task {

    /** The dialogMap. */
    DialogMap *dialogMap;

    /** The putBlockQueue. */
    PutBlockQueue *putBlockQueue;

public:

    /**
     * Creates a new instance of TestSessionTask.
     * @param *dialogMap DialogMap
     * @param *putBlockQueue PutBlockQueue
     */
    TestSessionTask(DialogMap *dialogMap, PutBlockQueue *putBlockQueue) {
        this->dialogMap=dialogMap;
        this->putBlockQueue=putBlockQueue;
    }

    Message* execute(Conn *conn, Message *msg) throw(Exception) {
        // id
        CompositeIE *idIE=msg->getComposite(CFIE::ID_IE);
        if(idIE!=NULL) {
            Id id(idIE);
            Dialog *dialog=dialogMap->get(id.getId1());
            if(dialog!=NULL) {
                putBlockQueue->push(new ActivityTestReqBlock(dialog, dialog->getInvokeId()));
                dialog->incrementInvokeId();
                putBlockQueue->push(new DelimiterReqBlock(dialog, 0));
                dialog->setState(Dialog::W_INVOKE);
                return SetSessionRet(AppMessages::ACCEPTED).toMessage();
            }
        }
        return TestSessionRet(AppMessages::FAILED).toMessage();
    }
};

#endif	/* CFTASK_HPP */
