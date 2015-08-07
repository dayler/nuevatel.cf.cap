/*
 * capdialog.hpp
 */
#ifndef CAPDIALOG_HPP
#define	CAPDIALOG_HPP

#include "block.hpp"
#include "appconn/cfie.hpp"
#include "appconn/cfmessage.hpp"
#include "../../base/appconn/appclient.hpp"

/**
 * The GPRSReferenceHandler class.
 */
class GPRSReferenceHandler {

    /** The gprsReferenceSuffix. */
    short gprsReferenceSuffix;
    boost::mutex gprsReferenceSuffixMutex;

public:

    /**
     * Creates a new instance of GPRSReferenceHandler.
     * @param &localId const int
     */
    GPRSReferenceHandler() {
        gprsReferenceSuffix=0;
    }

    /**
     * Returns the gprs reference.
     * @param &localId const int
     * @return unsigned int
     */
    unsigned int getGPRSReference(const int &localId) {
        unsigned int gprsReference=localId & 0x7f;
        time_t seconds;
        time(&seconds);
        gprsReference<<=16;
        gprsReference|=(seconds & 0xffff);
        gprsReference<<=8;
        {
            boost::lock_guard<boost::mutex> lock(gprsReferenceSuffixMutex);
            gprsReference|=(gprsReferenceSuffix & 0xff);
            if(gprsReferenceSuffix < 0xff) gprsReferenceSuffix++;
            else gprsReferenceSuffix=0;
        }
        return gprsReference;
    }
};

/**
 * <p>The CAPDialog class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CAPDialog : public Dialog {

    /** The appId. */
    int appId;

    /** The localId. */
    int localId;

    /** The putBlockQueue. */
    PutBlockQueue *putBlockQueue;

    /** The appClient. */
    AppClient *appClient;

    /** The dialogTaskService. */
    Executor *dialogTaskService;

    /** The cfMessageQueue. */
    Queue<CFMessage> cfMessageQueue;

    /** The gprsOriginationReference.*/
    unsigned int gprsOriginationReference;

    /** The gprsDestinationReference.*/
    unsigned int gprsDestinationReference;

    /* private variables */
    GPRSReferenceHandler gprsReferenceHandler;

public:

    /**
     * Creates a new instance of CAPDialog.
     * @param &appId const int
     * @param &localId const int
     * @param *putBlockQueue PutBlockQueue
     * @param *appClient AppClient
     * @param *dialogTaskService Executor
     */
    CAPDialog(const int &appId, const int &localId, PutBlockQueue *putBlockQueue, AppClient *appClient, Executor *dialogTaskService) {
        this->appId=appId;
        this->localId=localId;
        this->putBlockQueue=putBlockQueue;
        this->appClient=appClient;
        this->dialogTaskService=dialogTaskService;
        gprsOriginationReference=0;
        gprsDestinationReference=0;
    }

    ~CAPDialog() {}

    void init() {}

    void check() {
        if(getState()==CLOSE_0) setState(CLOSE_1);
        else if(getState()==W_CLOSE_0) setState(W_CLOSE_1);
        else if(getState()==W_CLOSE_1) putBlockQueue->push(new CloseReqBlock(this));
        else if(getState()==ABORT_0) setState(ABORT_1);
        else if(getState()==KILL_0) putBlockQueue->push(new CloseReqBlock(this));
    }

    /**
     * Compares two objectIDs.
     * @param *id1 ObjectID
     * @param *id2 ObjectID
     * @return bool
     */
    bool compareObjectIDs(ObjectID *id1, ObjectID *id2) {
        if(id1->count!=id2->count) return false;
        if(memcmp(id1->value, id2->value, id1->count * sizeof(unsigned int))==0) return true;
        else return false;
    }

    void handle(CapBlock *capBlock) {
        // capReq
        if(capBlock->serviceType==capReq) {
            // CAP_OPEN
            if(capBlock->serviceMsg==CAP_OPEN) {
                if(capBlock->parameter.openArg.applicationContext.count > 0) {
                    if(compareObjectIDs(&capBlock->parameter.openArg.applicationContext, &cAP_v2_gsmSSF_to_gsmSCF_AC))
                        putBlockQueue->push(new OpenResBlock(this, capDialogAccepted, OpenResBlock::GPRS_REFERENCE_NOT_PRESENT, 0, 0));
                    else if(compareObjectIDs(&capBlock->parameter.openArg.applicationContext, &id_ac_CAP_gprsSSF_gsmSCF_AC)) {
                        if((capBlock->parameter.openArg.dialogPortion.bit_mask & CapDialogPortion_gprsOriginatingReference_present)==CapDialogPortion_gprsOriginatingReference_present) {
                            gprsOriginationReference=capBlock->parameter.openArg.dialogPortion.gprsOriginatingReference;
                            if((capBlock->parameter.openArg.dialogPortion.bit_mask & CapDialogPortion_gprsDestinationReference_present)==CapDialogPortion_gprsDestinationReference_present)
                                gprsDestinationReference=capBlock->parameter.openArg.dialogPortion.gprsDestinationReference;
                            else gprsDestinationReference=gprsReferenceHandler.getGPRSReference(localId);
                            putBlockQueue->push(new OpenResBlock(this, capDialogAccepted, OpenResBlock::GPRS_ORIGINATION_REFERENCE_PRESENT | OpenResBlock::GPRS_DESTINATION_REFERENCE_PRESENT, gprsDestinationReference, gprsOriginationReference));
                        }
                        else {
                            Logger::getLogger()->logp(&Level::WARNING, "CAPDialog", "handle", "gprsOriginatingReference not present");
                            putBlockQueue->push(new OpenResBlock(this, capDialogRefused, capAppContextNotSupported));
                        }
                    }
                    else {
                        Logger::getLogger()->logp(&Level::WARNING, "CAPDialog", "handle", "capDialogRefused");
                        putBlockQueue->push(new OpenResBlock(this, capDialogRefused, capAppContextNotSupported));
                    }
                }
                // state
                setState(W_INVOKE);
            }
            // CAP_INITIAL_DP
            else if(capBlock->serviceMsg==CAP_INITIAL_DP) {
                // id
                std::stringstream ss;
                ss << std::hex << appId << std::hex << getDialogId();
                Id *id=new Id(ss.str(), getDialogId());
                // type
                char type=SERVICE_TYPE::SPEECH.getType();
                if(capBlock->parameter.initialDPArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_collectedInfo) {
                    if(capBlock->parameter.initialDPArg_v2.redirectingPartyID.length > 0) type|=REQUEST_TYPE::FWO.getType();
                    else type|=REQUEST_TYPE::O.getType();
                }
                else if(capBlock->parameter.initialDPArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_termAttemptAuthorized) {
                    if(capBlock->parameter.initialDPArg_v2.redirectingPartyID.length > 0) type|=REQUEST_TYPE::FWT.getType();
                    else type|=REQUEST_TYPE::T.getType();
                }

                // name
                Name *name=NULL;
                // fromName
                Name *fromName=NULL;
                // toName
                Name *toName=NULL;

                if(capBlock->parameter.initialDPArg_v2.redirectingPartyID.length > 0) {
                    CallingPartyNumber callingPartyNumber((char*)capBlock->parameter.initialDPArg_v2.redirectingPartyID.value, capBlock->parameter.initialDPArg_v2.redirectingPartyID.length);
                    if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWO.getType())
                        name=new Name(callingPartyNumber.getAddr(), callingPartyNumber.getNAI());
                    else if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWT.getType())
                        toName=new Name(callingPartyNumber.getAddr(), callingPartyNumber.getNAI());
                }
                if(capBlock->parameter.initialDPArg_v2.callingPartyNumber.length > 0) {
                    CallingPartyNumber callingPartyNumber((char*)capBlock->parameter.initialDPArg_v2.callingPartyNumber.value, capBlock->parameter.initialDPArg_v2.callingPartyNumber.length);
                    if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::O.getType())
                        name=new Name(callingPartyNumber.getAddr(), callingPartyNumber.getNAI());
                    fromName=new Name(callingPartyNumber.getAddr(), callingPartyNumber.getNAI());
                }
                if(capBlock->parameter.initialDPArg_v2.calledPartyNumber.length > 0) {
                    CalledPartyNumber calledPartyNumber((char*)capBlock->parameter.initialDPArg_v2.calledPartyNumber.value, capBlock->parameter.initialDPArg_v2.calledPartyNumber.length);
                    if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::T.getType() || REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWT.getType())
                        name=new Name(calledPartyNumber.getAddr(), calledPartyNumber.getNAI());
                    if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::T.getType() || REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWO.getType())
                        toName=new Name(calledPartyNumber.getAddr(), calledPartyNumber.getNAI());
                }
                if(capBlock->parameter.initialDPArg_v2.calledPartyBCDNumber.length > 0) {
                    Address calledPartyBCDNumber((char*)capBlock->parameter.initialDPArg_v2.calledPartyBCDNumber.value, capBlock->parameter.initialDPArg_v2.calledPartyBCDNumber.length);
                    if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::O.getType())
                        toName=new Name(calledPartyBCDNumber.getAddr(), calledPartyBCDNumber.getTON());
                }

                // cellGlobalId
                std::string cellGlobalId="";
                if((capBlock->parameter.initialDPArg_v2.locationInformation.bit_mask & CapLocationInformation_cellIdOrLAI_present)==CapLocationInformation_cellIdOrLAI_present) {
                    if(capBlock->parameter.initialDPArg_v2.locationInformation.cellIdOrLAI.choice==CapCellIdOrLAI_cellIdFixedLength_chosen && capBlock->parameter.initialDPArg_v2.locationInformation.cellIdOrLAI.u.cellIdFixedLength.length > 0) {
                        CellGlobalId tmpCellGlobalId((char*)capBlock->parameter.initialDPArg_v2.locationInformation.cellIdOrLAI.u.cellIdFixedLength.value, capBlock->parameter.initialDPArg_v2.locationInformation.cellIdOrLAI.u.cellIdFixedLength.length);
                        cellGlobalId=tmpCellGlobalId.getCellGlobalId();
                    }
                }

                // nodeId
                std::string nodeId="";
                if((capBlock->parameter.initialDPArg_v2.locationInformation.bit_mask & CapLocationInformation_vlr_number_present)==CapLocationInformation_vlr_number_present && capBlock->parameter.initialDPArg_v2.locationInformation.vlr_number.length > 0) {
                    Address vlrNumber((char*)capBlock->parameter.initialDPArg_v2.locationInformation.vlr_number.value, capBlock->parameter.initialDPArg_v2.locationInformation.vlr_number.length);
                    nodeId=vlrNumber.getAddr();
                }

                // location
                Location *location=new Location(cellGlobalId, nodeId);

                // reference
                std::string reference="";
                if(capBlock->parameter.initialDPArg_v2.callReferenceNumber.length > 0) {
                    reference=toHexString((char*)capBlock->parameter.initialDPArg_v2.callReferenceNumber.value, capBlock->parameter.initialDPArg_v2.callReferenceNumber.length);
                }

                // sessionArg
                SessionArg *sessionArg=new SessionArg(fromName, toName, "", "", "", reference);

                cfMessageQueue.push(new NewSessionCall(id, type, "", name, location, sessionArg));
                // state
                setState(INVOKE);
                // submit
                dialogTaskService->submit(new DialogTask(this));
            }
            // CAP_INITIAL_DPGPRS
            else if(capBlock->serviceMsg==CAP_INITIAL_DPGPRS) {
                // id
                std::stringstream ss;
                ss << std::hex << gprsOriginationReference << ' ' << std::hex << gprsDestinationReference;
                Id *id=new Id(ss.str(), getDialogId());
                // type
                char type=SERVICE_TYPE::DATA.getType();
                if(capBlock->parameter.initialDPGPRSArg_v3.gPRSEventType==capGPRSEventType_pdp_ContextEstablishment) type|=REQUEST_TYPE::PDPCE.getType();
                else if(capBlock->parameter.initialDPGPRSArg_v3.gPRSEventType==capGPRSEventType_pdp_ContextEstablishmentAcknowledgement) type|=REQUEST_TYPE::PDPCEA.getType();
                else if(capBlock->parameter.initialDPGPRSArg_v3.gPRSEventType==capGPRSEventType_pdp_ContextChangeOfPosition) type|=REQUEST_TYPE::PDPCHP.getType();

                // name
                Name *name=NULL;
                if(capBlock->parameter.initialDPGPRSArg_v3.mSISDN.length > 0) {
                    Address msisdn((char*)capBlock->parameter.initialDPGPRSArg_v3.mSISDN.value, capBlock->parameter.initialDPGPRSArg_v3.mSISDN.length);
                    name=new Name(msisdn.getAddr(), msisdn.getTON());
                }

                // cellGlobalId
                std::string cellGlobalId="";
                if((capBlock->parameter.initialDPGPRSArg_v3.locationInformationGPRS.bit_mask & CapLocationInformationGPRS_cellGlobalIdOrServiceAreaIdOrLAI_present)==CapLocationInformationGPRS_cellGlobalIdOrServiceAreaIdOrLAI_present && capBlock->parameter.initialDPGPRSArg_v3.locationInformationGPRS.cellGlobalIdOrServiceAreaIdOrLAI.length > 0) {
                    CellGlobalId tmpCellGlobalId((char*)capBlock->parameter.initialDPGPRSArg_v3.locationInformationGPRS.cellGlobalIdOrServiceAreaIdOrLAI.value, capBlock->parameter.initialDPGPRSArg_v3.locationInformationGPRS.cellGlobalIdOrServiceAreaIdOrLAI.length);
                    cellGlobalId=tmpCellGlobalId.getCellGlobalId();
                }

                // nodeId
                std::string nodeId="";
                if((capBlock->parameter.initialDPGPRSArg_v3.locationInformationGPRS.bit_mask & CapLocationInformationGPRS_sgsn_Number_present)==CapLocationInformationGPRS_sgsn_Number_present && capBlock->parameter.initialDPGPRSArg_v3.locationInformationGPRS.sgsn_Number.length > 0) {
                    Address sgsn((char*)capBlock->parameter.initialDPGPRSArg_v3.locationInformationGPRS.sgsn_Number.value, capBlock->parameter.initialDPGPRSArg_v3.locationInformationGPRS.sgsn_Number.length);
                    nodeId=sgsn.getAddr();
                }

                // location
                Location *location=new Location(cellGlobalId, nodeId);

                // apn
                std::string apn="";
                if(capBlock->parameter.initialDPGPRSArg_v3.accessPointName.length > 0) {
                    AccessPointName accessPointName((char*)capBlock->parameter.initialDPGPRSArg_v3.accessPointName.value, capBlock->parameter.initialDPGPRSArg_v3.accessPointName.length);
                    apn=accessPointName.getAccessPointName();
                }

                // qos
                std::string qos="";
                if((capBlock->parameter.initialDPGPRSArg_v3.qualityOfService.bit_mask & CapQualityOfService_subscribed_QoS_present)==CapQualityOfService_subscribed_QoS_present) {
                    if(capBlock->parameter.initialDPGPRSArg_v3.qualityOfService.subscribed_QoS.choice==CapGPRS_QoS_short_QoS_format_chosen) {
                        qos=toHexString((char*)capBlock->parameter.initialDPGPRSArg_v3.qualityOfService.subscribed_QoS.u.short_QoS_format.value, capBlock->parameter.initialDPGPRSArg_v3.qualityOfService.subscribed_QoS.u.short_QoS_format.length);
                    }
                    else if(capBlock->parameter.initialDPGPRSArg_v3.qualityOfService.subscribed_QoS.choice==CapGPRS_QoS_long_QoS_format_chosen) {
                        qos=toHexString((char*)capBlock->parameter.initialDPGPRSArg_v3.qualityOfService.subscribed_QoS.u.long_QoS_format.value, capBlock->parameter.initialDPGPRSArg_v3.qualityOfService.subscribed_QoS.u.long_QoS_format.length);
                    }
                }

                // reference
                std::string reference="";
                if(capBlock->parameter.initialDPGPRSArg_v3.chargingID.length > 0) {
                    reference=toHexString((char*)capBlock->parameter.initialDPGPRSArg_v3.chargingID.value, capBlock->parameter.initialDPGPRSArg_v3.chargingID.length);
                }

                // sessionArg
                SessionArg *sessionArg=new SessionArg(NULL, NULL, apn, qos, "", reference);

                cfMessageQueue.push(new NewSessionCall(id, type, "", name, location, sessionArg));
                // state
                setState(INVOKE);
                // submit
                dialogTaskService->submit(new DialogTask(this));
            }
            // CAP_EVENT_REPORT_BCSM
            else if(capBlock->serviceMsg==CAP_EVENT_REPORT_BCSM) {
                // id
                std::stringstream ss;
                ss << std::hex << appId << std::hex << getDialogId();
                Id *id=new Id(ss.str(), getDialogId());
                // type
                char type=SERVICE_TYPE::SPEECH.getType();
                // eventType
                char eventType=0;
                // eventArg
                EventArg *eventArg=NULL;

                if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_routeSelectFailure) {type|=REQUEST_TYPE::O.getType(); eventType=EVENT_TYPE::ROUTE_SELECT_FAILURE_2.getType();}
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_oCalledPartyBusy) {type|=REQUEST_TYPE::O.getType(); eventType=EVENT_TYPE::O_CALLED_PARTY_BUSY_2.getType();}
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_oNoAnswer) {type|=REQUEST_TYPE::O.getType(); eventType=EVENT_TYPE::O_NO_ANSWER_2.getType();}
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_oAnswer) {type|=REQUEST_TYPE::O.getType(); eventType=EVENT_TYPE::O_ANSWER_2.getType();}
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_oDisconnect) {
                    type|=REQUEST_TYPE::O.getType();
                    eventType=EVENT_TYPE::O_DISCONNECT_1.getType();
                    if(capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.choice==CapEventSpecificInformationBCSM_t2_oDisconnectSpecificInfo_chosen) {
                        if(capBlock->parameter.eventReportBCSMArg_v2.legID.choice==CapReceivingSideID_receivingSideID_chosen && capBlock->parameter.eventReportBCSMArg_v2.legID.u.receivingSideID.value[0]==2) eventType=EVENT_TYPE::O_DISCONNECT_2.getType();
                        if(capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.u.oDisconnectSpecificInfo.bit_mask==CapEventSpecificInformationBCSM_t2_oDisconnectSpecificInfo_releaseCause_present && capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.u.oDisconnectSpecificInfo.releaseCause.length > 0) {
                            Cause cause((char*)capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.u.oDisconnectSpecificInfo.releaseCause.value, capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.u.oDisconnectSpecificInfo.releaseCause.length);
                            eventArg=new EventArg(cause.getCause());
                        }
                    }
                }
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_oAbandon) {type|=REQUEST_TYPE::O.getType(); eventType=EVENT_TYPE::O_ABANDON_1.getType();}
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_tBusy) {type|=REQUEST_TYPE::T.getType(); eventType=EVENT_TYPE::T_BUSY_2.getType();}
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_tNoAnswer) {type|=REQUEST_TYPE::T.getType(); eventType=EVENT_TYPE::T_NO_ANSWER_2.getType();}
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_tAnswer) {type|=REQUEST_TYPE::T.getType(); eventType=EVENT_TYPE::T_ANSWER_2.getType();}
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_tDisconnect) {
                    type|=REQUEST_TYPE::T.getType();
                    eventType=EVENT_TYPE::T_DISCONNECT_1.getType();
                    if(capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.choice==CapEventSpecificInformationBCSM_t2_tDisconnectSpecificInfo_chosen) {
                        if(capBlock->parameter.eventReportBCSMArg_v2.legID.choice==CapReceivingSideID_receivingSideID_chosen && capBlock->parameter.eventReportBCSMArg_v2.legID.u.receivingSideID.value[0]==2) eventType=EVENT_TYPE::T_DISCONNECT_2.getType();
                        if(capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.u.tDisconnectSpecificInfo.bit_mask==CapEventSpecificInformationBCSM_t2_tDisconnectSpecificInfo_releaseCause_present && capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.u.tDisconnectSpecificInfo.releaseCause.length > 0) {
                            Cause cause((char*)capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.u.tDisconnectSpecificInfo.releaseCause.value, capBlock->parameter.eventReportBCSMArg_v2.eventSpecificInformationBCSM.u.tDisconnectSpecificInfo.releaseCause.length);
                            eventArg=new EventArg(cause.getCause());
                        }
                    }
                }
                else if(capBlock->parameter.eventReportBCSMArg_v2.eventTypeBCSM==capEventTypeBCSM_t2_tAbandon) {type|=REQUEST_TYPE::T.getType(); eventType=EVENT_TYPE::T_ABANDON_1.getType();}

                cfMessageQueue.push(new EventReportCall(id, type, eventType, eventArg));
                // state
                setState(INVOKE);
                // submit
                dialogTaskService->submit(new DialogTask(this));
            }
            // CAP_EVENT_REPORT_GPRS
            else if(capBlock->serviceMsg==CAP_EVENT_REPORT_GPRS) {
                // id
                std::stringstream ss;
                ss << std::hex << gprsOriginationReference << ' ' << std::hex << gprsDestinationReference;
                Id *id=new Id(ss.str(), getDialogId());
                // type
                char type=SERVICE_TYPE::DATA.getType();
                // eventType
                char eventType=0;

                if(capBlock->parameter.eventReportGPRSArg_v3.gPRSEventType==capGPRSEventType_pdp_ContextEstablishmentAcknowledgement) {
                    putBlockQueue->push(new EventReportGPRSResBlock(this, capBlock->invokeId));
                    eventType=EVENT_TYPE::PDP_CONTEXT_ESTABLISHMENT_ACK.getType();
                }
                else if(capBlock->parameter.eventReportGPRSArg_v3.gPRSEventType==capGPRSEventType_disconnect) {
                    putBlockQueue->push(new EventReportGPRSResBlock(this, capBlock->invokeId));
                    putBlockQueue->push(new ContinueGPRSReqBlock(this, getInvokeId()));
                    incrementInvokeId();
                    putBlockQueue->push(new DelimiterReqBlock(this, 0));
                    eventType=EVENT_TYPE::DISCONNECT.getType();
                }

                cfMessageQueue.push(new EventReportCall(id, type, eventType, NULL));
                // state
                setState(INVOKE);
                // submit
                dialogTaskService->submit(new DialogTask(this));
            }
            // CAP_ENTITY_RELEASED_GPRS
            else if(capBlock->serviceMsg==CAP_ENTITY_RELEASED_GPRS) {
                // id
                std::stringstream ss;
                ss << std::hex << gprsOriginationReference << ' ' << std::hex << gprsDestinationReference;
                Id *id=new Id(ss.str(), getDialogId());
                // type
                char type=SERVICE_TYPE::DATA.getType();
                // eventType
                char eventType=EVENT_TYPE::ENTITY_RELEASED_GPRS.getType();
                putBlockQueue->push(new EntityReleasedGPRSResBlock(this, capBlock->invokeId));
                putBlockQueue->push(new DelimiterReqBlock(this, 0));

                cfMessageQueue.push(new EventReportCall(id, type, eventType, NULL));
                // state
                setState(INVOKE);
                // submit
                dialogTaskService->submit(new DialogTask(this));
            }
            // CAP_APPLY_CHARGING_REPORT
            else if(capBlock->serviceMsg==CAP_APPLY_CHARGING_REPORT) {
                // id
                std::stringstream ss;
                ss << std::hex << appId << std::hex << getDialogId();
                Id *id=new Id(ss.str(), getDialogId());
                // watchType
                char watchType=0;

                if(capBlock->subParameter.cAMEL_CallResult_v2.choice==CapCAMEL_CallResult_v2_timeDurationChargingResult_chosen && capBlock->subParameter.cAMEL_CallResult_v2.u.timeDurationChargingResult.timeInformation.choice==CapTimeInformation_timeIfNoTariffSwitch_chosen) {
                    if(capBlock->subParameter.cAMEL_CallResult_v2.u.timeDurationChargingResult.callActive) watchType=WATCH_TYPE::A_TIME_WATCH.getType();
                    else watchType=WATCH_TYPE::TIME_WATCH.getType();

                    cfMessageQueue.push(new WatchReportCall(id, watchType, capBlock->subParameter.cAMEL_CallResult_v2.u.timeDurationChargingResult.partyToCharge.u.receivingSideID.value[0], new WatchArg(capBlock->subParameter.cAMEL_CallResult_v2.u.timeDurationChargingResult.timeInformation.u.timeIfNoTariffSwitch, -1, -1, -1, -1, -1)));
                    // state
                    setState(INVOKE);
                    // submit
                    dialogTaskService->submit(new DialogTask(this));
                }
            }
            // CAP_APPLY_CHARGING_REPORT_GPRS
            else if(capBlock->serviceMsg==CAP_APPLY_CHARGING_REPORT_GPRS) {
                // id
                std::stringstream ss;
                ss << std::hex << gprsOriginationReference << ' ' << std::hex << gprsDestinationReference;
                Id *id=new Id(ss.str(), getDialogId());
                // watchType
                char watchType=0;
                // watchArg
                WatchArg *watchArg=NULL;

                putBlockQueue->push(new ApplyChargingReportGPRSResBlock(this, capBlock->invokeId));
                putBlockQueue->push(new DelimiterReqBlock(this, 0));

                if(capBlock->parameter.applyChargingReportGPRSArg_v3.chargingResult.choice==CapChargingResult_elapsedTime_chosen && capBlock->parameter.applyChargingReportGPRSArg_v3.chargingResult.u.elapsedTime.choice==CapElapsedTime_timeGPRSIfNoTariffSwitch_chosen) {
                    if(capBlock->parameter.applyChargingReportGPRSArg_v3.active) watchType=WATCH_TYPE::D_A_TIME_WATCH.getType();
                    else watchType=WATCH_TYPE::D_TIME_WATCH.getType();
                    watchArg=new WatchArg(capBlock->parameter.applyChargingReportGPRSArg_v3.chargingResult.u.elapsedTime.u.timeGPRSIfNoTariffSwitch, -1, -1, -1, -1, -1);
                }
                if(capBlock->parameter.applyChargingReportGPRSArg_v3.chargingResult.choice==CapChargingResult_transferredVolume_chosen && capBlock->parameter.applyChargingReportGPRSArg_v3.chargingResult.u.transferredVolume.choice==CapTransferredVolume_volumeIfNoTariffSwitch_chosen) {
                    if(capBlock->parameter.applyChargingReportGPRSArg_v3.active) watchType=WATCH_TYPE::D_A_UNIT_WATCH.getType();
                    else watchType=WATCH_TYPE::D_UNIT_WATCH.getType();
                    watchArg=new WatchArg(-1, capBlock->parameter.applyChargingReportGPRSArg_v3.chargingResult.u.transferredVolume.u.volumeIfNoTariffSwitch, -1, -1, -1, -1);
                }

                cfMessageQueue.push(new WatchReportCall(id, watchType, -1, watchArg));
                // state
                setState(INVOKE);
                // submit
                dialogTaskService->submit(new DialogTask(this));
            }
            // CAP_DELIMITER
            else if(capBlock->serviceMsg==CAP_DELIMITER) {
                //Logger::getLogger()->logp(&Level::INFO, "CAPDialog", "handle", "CAP_DELIMITER");
            }
            // CAP_CLOSE
            else if(capBlock->serviceMsg==CAP_CLOSE) {
                //Logger::getLogger()->logp(&Level::INFO, "CAPDialog", "handle", "CAP_CLOSE");
                setState(CLOSE_0);
            }
            // CAP_U_ABORT, CAP_P_ABORT
            else if(capBlock->serviceMsg==CAP_U_ABORT || capBlock->serviceMsg==CAP_P_ABORT) {
                // id
                std::stringstream ss;
                if(gprsOriginationReference==0 && gprsDestinationReference==0)
                    ss << std::hex << appId << std::hex << getDialogId();
                else ss << std::hex << gprsOriginationReference << ' ' << std::hex << gprsDestinationReference;
                Id *id=new Id(ss.str(), getDialogId());
                // eventType
                char eventType=0;
                if(capBlock->serviceMsg==CAP_U_ABORT) eventType=EVENT_TYPE::U_ABORT.getType();
                else eventType=EVENT_TYPE::P_ABORT.getType();

                cfMessageQueue.push(new EventReportCall(id, 0, eventType, NULL));
                // state
                setState(INVOKE);
                // submit
                dialogTaskService->submit(new DialogTask(this));
            }
            else {
                std::stringstream ss;
                ss << "unknown serviceMsg " << capBlock->serviceMsg;
                Logger::getLogger()->logp(&Level::INFO, "CAPDialog", "handle", ss.str());
                putBlockQueue->push(new CloseReqBlock(this));
            }
        }
        else if(capBlock->serviceType==capRsp) {
            if(capBlock->serviceMsg==CAP_ACTIVITY_TEST) {
                // id
                std::stringstream ss;
                ss << std::hex << appId << std::hex << getDialogId();
                Id *id=new Id(ss.str(), getDialogId());

                cfMessageQueue.push(new TestSessionAsyncRet(id, AppMessages::ACCEPTED));
                // state
                setState(W_INVOKE);
                // submit
                dialogTaskService->submit(new DialogTask(this));
            }
        }
        else if(capBlock->serviceType==capError) {
            std::stringstream ss;
            ss << "capError " << std::hex << appId << std::hex << getDialogId();
            Logger::getLogger()->logp(&Level::INFO, "CAPDialog", "handle", ss.str());
        }
    }

    void run() {
        CFMessage *cfMessage=cfMessageQueue.waitAndPop();
        Message *msg=cfMessage->toMessage();
        Message *ret=NULL;
        try {
            ret=appClient->dispatch(msg);
        } catch(Exception e) {
            Logger::getLogger()->logp(&Level::WARNING, "CAPDialog", "run", e.toString());
        }
        // NEW_SESSION_CALL
        if(msg->getCode()==CFMessage::NEW_SESSION_CALL) {
            // type
            char type=msg->getByte(CFIE::TYPE_IE);

            if(ret!=NULL) {
                // action
                char action=ret->getByte(CFIE::ACTION_IE);

                // NEW_MEDIA
                if(MEDIA_ACTION::getMediaAction(action)==MEDIA_ACTION::NEW_MEDIA.getAction()) {
                    // SPEECH, VIDEO
                    if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::SPEECH.getType() || SERVICE_TYPE::getType(type)==SERVICE_TYPE::VIDEO.getType()) {
                        // mediaArg
                        CompositeIE *mediaArgIE=ret->getComposite(CFIE::MEDIA_ARG_IE);
                        if(mediaArgIE!=NULL) {
                            MediaArg mediaArg(mediaArgIE);
                            if(mediaArg.getMediaArg0().length() > 0) {
                                GenericNumber genericNumber(mediaArg.getMediaArg1(), GenericNumber::NPI_ISDN, mediaArg.getMediaArg0(), GenericNumber::NI_COMPLETE, GenericNumber::APRI_ADDRESS_NA, GenericNumber::SI_USER_PROVIDED_NV, GenericNumber::NQI_RESERVED_DIALED_DIGITS);

                                if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::O.getType() || REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWO.getType())
                                    putBlockQueue->push(new RequestReportBCSMEventReqBlock(this, getInvokeId(), RequestReportBCSMEventReqBlock::O_DISCONNECT_1 | RequestReportBCSMEventReqBlock::O_ABANDON_1));
                                else putBlockQueue->push(new RequestReportBCSMEventReqBlock(this, getInvokeId(), RequestReportBCSMEventReqBlock::T_DISCONNECT_1 | RequestReportBCSMEventReqBlock::T_ABANDON_1));
                                incrementInvokeId();
                                putBlockQueue->push(new EstablishTemporaryConnectionReqBlock(this, getInvokeId(), &genericNumber));
                                incrementInvokeId();
                                putBlockQueue->push(new DelimiterReqBlock(this, 0));
                            }
                        }
                    }
                    setState(W_INVOKE);
                }
                // ACCEPT
                else if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::ACCEPT.getAction()) {
                    // watchArg
                    CompositeIE *watchArgIE=ret->getComposite(CFIE::WATCH_ARG_IE);
                    // DATA
                    if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::DATA.getType()) {
                        if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::PDPCE.getType()) {
                            putBlockQueue->push(new RequestReportGPRSEventReqBlock(this, getInvokeId(), RequestReportGPRSEventReqBlock::ALL));
                            incrementInvokeId();
                        }
                        else {
                            putBlockQueue->push(new RequestReportGPRSEventReqBlock(this, getInvokeId(), RequestReportGPRSEventReqBlock::DISCONNECT));
                            incrementInvokeId();
                            if(watchArgIE!=NULL) {
                                WatchArg watchArg(watchArgIE);
                                if(watchArg.getWatchArg0() > 0) {
                                    putBlockQueue->push(new ApplyChargingGPRSReqBlock(this, getInvokeId(), 0, watchArg.getWatchArg0()));
                                    incrementInvokeId();
                                }
                                if(watchArg.getWatchArg1() > 0) {
                                    putBlockQueue->push(new ApplyChargingGPRSReqBlock(this, getInvokeId(), watchArg.getWatchArg1(), 0));
                                    incrementInvokeId();
                                }
                            }
                        }
                        putBlockQueue->push(new ContinueGPRSReqBlock(this, getInvokeId()));
                        incrementInvokeId();
                        putBlockQueue->push(new DelimiterReqBlock(this, 0));
                        setState(W_CLOSE_0);
                    }
                    // SPEECH VIDEO
                    else if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::SPEECH.getType() || SERVICE_TYPE::getType(type)==SERVICE_TYPE::VIDEO.getType()) {
                        if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::O.getType() || REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWO.getType())
                            putBlockQueue->push(new RequestReportBCSMEventReqBlock(this, getInvokeId(), RequestReportBCSMEventReqBlock::O_ALL));
                        else putBlockQueue->push(new RequestReportBCSMEventReqBlock(this, getInvokeId(), RequestReportBCSMEventReqBlock::T_ALL));
                        incrementInvokeId();
                        putBlockQueue->push(new ContinueReqBlock(this, getInvokeId()));
                        incrementInvokeId();
                        putBlockQueue->push(new DelimiterReqBlock(this, 0));
                        setState(W_INVOKE);
                    }
                }
                // MODIFY
                else if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::MODIFY.getAction()) {
                    // SPEECH VIDEO
                    if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::SPEECH.getType() || SERVICE_TYPE::getType(type)==SERVICE_TYPE::VIDEO.getType()) {
                        // sessionArg
                        CompositeIE *sessionArgIE=ret->getComposite(CFIE::SESSION_ARG_IE);
                        if(sessionArgIE!=NULL) {
                            SessionArg sessionArg(sessionArgIE);
                            if(sessionArg.getToName()!=NULL) {
                                CalledPartyNumber calledPartyNumber(sessionArg.getToName()->getType(), CalledPartyNumber::NPI_ISDN, sessionArg.getToName()->getName(), CalledPartyNumber::INN_RINNA);
                                GenericNumber *genericNumber=NULL;
                                if(sessionArg.getFromName()!=NULL) genericNumber=new GenericNumber(sessionArg.getFromName()->getType(), GenericNumber::NPI_ISDN, sessionArg.getFromName()->getName(), GenericNumber::NI_COMPLETE, GenericNumber::APRI_PRESENTATION_ALLOWED, GenericNumber::SI_USER_PROVIDED_NV, GenericNumber::NQI_ADDITIONAL_CALLING_PARTY_NUMBER);

                                if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::O.getType() || REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWO.getType())
                                    putBlockQueue->push(new RequestReportBCSMEventReqBlock(this, getInvokeId(), RequestReportBCSMEventReqBlock::O_ALL));
                                else putBlockQueue->push(new RequestReportBCSMEventReqBlock(this, getInvokeId(), RequestReportBCSMEventReqBlock::T_ALL));
                                incrementInvokeId();
                                putBlockQueue->push(new ConnectReqBlock(this, getInvokeId(), &calledPartyNumber, genericNumber));
                                incrementInvokeId();
                                putBlockQueue->push(new DelimiterReqBlock(this, 0));
                                if(genericNumber!=NULL) delete genericNumber;
                            }
                        }
                    }
                    setState(W_INVOKE);
                }
                // END
                else if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::END.getAction()) {
                    // DATA
                    if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::DATA.getType()) {
                        putBlockQueue->push(new ReleaseGPRSReqBlock(this, getInvokeId(), 0));
                        incrementInvokeId();
                        putBlockQueue->push(new CloseReqBlock(this));
                    }
                    // SPEECH VIDEO
                    else if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::SPEECH.getType() || SERVICE_TYPE::getType(type)==SERVICE_TYPE::VIDEO.getType()) {
                        Cause cause(Cause::CS_ITU_T, Cause::L_USER, Cause::CAUSE_NORMAL_CALL_CLEARING);
                        putBlockQueue->push(new ReleaseCallReqBlock(this, getInvokeId(), &cause));
                        putBlockQueue->push(new CloseReqBlock(this));
                    }
                }
            } // ret NULL
            else {
                // DATA
                if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::DATA.getType()) {
                    putBlockQueue->push(new ReleaseGPRSReqBlock(this, getInvokeId(), 0));
                    incrementInvokeId();
                    putBlockQueue->push(new CloseReqBlock(this));
                }
                // SPEECH VIDEO
                else if(SERVICE_TYPE::getType(type)==SERVICE_TYPE::SPEECH.getType() || SERVICE_TYPE::getType(type)==SERVICE_TYPE::VIDEO.getType()) {
                    Cause cause(Cause::CS_ITU_T, Cause::L_USER, Cause::CAUSE_NORMAL_CALL_CLEARING);
                    putBlockQueue->push(new ReleaseCallReqBlock(this, getInvokeId(), &cause));
                    putBlockQueue->push(new CloseReqBlock(this));
                }
            }
        }
        // EVENT_REPORT_CALL
        else if(msg->getCode()==CFMessage::EVENT_REPORT_CALL) {
            // type
            char type=msg->getByte(CFIE::TYPE_IE);
            // eventType
            char eventType=msg->getByte(CFIE::EVENT_TYPE_IE);

            if(ret!=NULL) {
                // action
                char action=ret->getByte(CFIE::ACTION_IE);

                // NEW_MEDIA
                if(MEDIA_ACTION::getMediaAction(action)==MEDIA_ACTION::NEW_MEDIA.getAction()) {
                    // SPEECH, VIDEO
                    if(eventType==EVENT_TYPE::O_CALLED_PARTY_BUSY_2.getType() ||
                       eventType==EVENT_TYPE::O_NO_ANSWER_2.getType() ||
                       eventType==EVENT_TYPE::O_DISCONNECT_2.getType() ||
                       eventType==EVENT_TYPE::T_BUSY_2.getType() ||
                       eventType==EVENT_TYPE::T_NO_ANSWER_2.getType() ||
                       eventType==EVENT_TYPE::T_DISCONNECT_2.getType()) {
                        // mediaArg
                        CompositeIE *mediaArgIE=ret->getComposite(CFIE::MEDIA_ARG_IE);
                        if(mediaArgIE!=NULL) {
                            MediaArg mediaArg(mediaArgIE);
                            if(mediaArg.getMediaArg0().length() > 0) {
                                GenericNumber genericNumber(mediaArg.getMediaArg1(), GenericNumber::NPI_ISDN, mediaArg.getMediaArg0(), GenericNumber::NI_COMPLETE, GenericNumber::APRI_ADDRESS_NA, GenericNumber::SI_USER_PROVIDED_NV, GenericNumber::NQI_RESERVED_DIALED_DIGITS);

                                if(REQUEST_TYPE::getType(type)==REQUEST_TYPE::O.getType() || REQUEST_TYPE::getType(type)==REQUEST_TYPE::FWO.getType())
                                    putBlockQueue->push(new RequestReportBCSMEventReqBlock(this, getInvokeId(), RequestReportBCSMEventReqBlock::O_DISCONNECT_1 | RequestReportBCSMEventReqBlock::O_ABANDON_1));
                                else putBlockQueue->push(new RequestReportBCSMEventReqBlock(this, getInvokeId(), RequestReportBCSMEventReqBlock::T_DISCONNECT_1 | RequestReportBCSMEventReqBlock::T_ABANDON_1));
                                incrementInvokeId();
                                putBlockQueue->push(new EstablishTemporaryConnectionReqBlock(this, getInvokeId(), &genericNumber));
                                incrementInvokeId();
                                putBlockQueue->push(new DelimiterReqBlock(this, 0));
                            }
                        }
                    }
                    setState(W_INVOKE);
                }
                // ACCEPT
                else if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::ACCEPT.getAction()) {
                    // watchArg
                    CompositeIE *watchArgIE=ret->getComposite(CFIE::WATCH_ARG_IE);
                    // DATA
                    if(eventType==EVENT_TYPE::PDP_CONTEXT_ESTABLISHMENT_ACK.getType()) {
                        if(watchArgIE!=NULL) {
                            WatchArg watchArg(watchArgIE);
                            if(watchArg.getWatchArg0() > 0) {
                                putBlockQueue->push(new ApplyChargingGPRSReqBlock(this, getInvokeId(), 0, watchArg.getWatchArg0()));
                                incrementInvokeId();
                            }
                            if(watchArg.getWatchArg1() > 0) {
                                putBlockQueue->push(new ApplyChargingGPRSReqBlock(this, getInvokeId(), watchArg.getWatchArg1(), 0));
                                incrementInvokeId();
                            }
                        }
                        putBlockQueue->push(new ContinueGPRSReqBlock(this, getInvokeId()));
                        incrementInvokeId();
                        putBlockQueue->push(new DelimiterReqBlock(this, 0));
                        setState(W_CLOSE_0);
                    }
                    // SPEECH, VIDEO
                    else if(eventType==EVENT_TYPE::O_ANSWER_2.getType() || eventType==EVENT_TYPE::T_ANSWER_2.getType()) {
                        if(watchArgIE!=NULL) {
                            WatchArg watchArg(watchArgIE);
                            unsigned int maxCallPeriodDuration=0;
                            bool releaseIfdurationExceeded=false;
                            if(watchArg.getWatchArg0() > 0) maxCallPeriodDuration=watchArg.getWatchArg0();
                            else if(watchArg.getWatchArg5() > 0) {
                                maxCallPeriodDuration=watchArg.getWatchArg5();
                                releaseIfdurationExceeded=true;
                            }
                            if(maxCallPeriodDuration > 0) {
                                if(eventType==EVENT_TYPE::O_ANSWER_2.getType())
                                    putBlockQueue->push(new ApplyChargingReqBlock(this, getInvokeId(), 1, maxCallPeriodDuration, releaseIfdurationExceeded));
                                else putBlockQueue->push(new ApplyChargingReqBlock(this, getInvokeId(), 2, maxCallPeriodDuration, releaseIfdurationExceeded));
                                incrementInvokeId();
                                putBlockQueue->push(new DelimiterReqBlock(this, 0));
                            }
                        }
                        setState(W_INVOKE);
                    }
                    else if(eventType==EVENT_TYPE::O_CALLED_PARTY_BUSY_2.getType() ||
                            eventType==EVENT_TYPE::O_NO_ANSWER_2.getType() ||
                            eventType==EVENT_TYPE::O_DISCONNECT_2.getType() ||
                            eventType==EVENT_TYPE::T_BUSY_2.getType() ||
                            eventType==EVENT_TYPE::T_NO_ANSWER_2.getType() ||
                            eventType==EVENT_TYPE::T_DISCONNECT_2.getType()) {
                        putBlockQueue->push(new ContinueReqBlock(this, getInvokeId()));
                        incrementInvokeId();
                        putBlockQueue->push(new DelimiterReqBlock(this, 0));
                        setState(W_CLOSE_0);
                    }
                    // ABORT
                    else if(eventType==EVENT_TYPE::U_ABORT.getType() || eventType==EVENT_TYPE::P_ABORT.getType()) {
                        setState(ABORT_0);
                    }
                    else setState(W_CLOSE_0);
                }
                // END
                else if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::END.getAction()) {
                    // DATA
                    if(eventType==EVENT_TYPE::PDP_CONTEXT_ESTABLISHMENT_ACK.getType()) {
                        putBlockQueue->push(new ReleaseGPRSReqBlock(this, getInvokeId(), 0));
                        incrementInvokeId();
                        putBlockQueue->push(new CloseReqBlock(this));
                    }
                    // SPEECH, VIDEO
                    else if(eventType==EVENT_TYPE::O_ANSWER_2.getType() || eventType==EVENT_TYPE::T_ANSWER_2.getType()) {
                        Cause cause(Cause::CS_ITU_T, Cause::L_USER, Cause::CAUSE_NORMAL_CALL_CLEARING);
                        putBlockQueue->push(new ReleaseCallReqBlock(this, getInvokeId(), &cause));
                        putBlockQueue->push(new CloseReqBlock(this));
                    }
                    else if(eventType==EVENT_TYPE::O_CALLED_PARTY_BUSY_2.getType() ||
                            eventType==EVENT_TYPE::O_NO_ANSWER_2.getType() ||
                            eventType==EVENT_TYPE::O_DISCONNECT_2.getType() ||
                            eventType==EVENT_TYPE::T_BUSY_2.getType() ||
                            eventType==EVENT_TYPE::T_NO_ANSWER_2.getType() ||
                            eventType==EVENT_TYPE::T_DISCONNECT_2.getType()) {
                        putBlockQueue->push(new ContinueReqBlock(this, getInvokeId()));
                        incrementInvokeId();
                        putBlockQueue->push(new DelimiterReqBlock(this, 0));
                        setState(W_CLOSE_0);
                    }
                    // ABORT
                    else if(eventType==EVENT_TYPE::U_ABORT.getType() || eventType==EVENT_TYPE::P_ABORT.getType()) {
                        setState(ABORT_0);
                    }
                    else setState(W_CLOSE_0);
                }
            } // ret NULL
            else {
                // DATA
                if(eventType==EVENT_TYPE::PDP_CONTEXT_ESTABLISHMENT_ACK.getType()) {
                    putBlockQueue->push(new ReleaseGPRSReqBlock(this, getInvokeId(), 0));
                    incrementInvokeId();
                    putBlockQueue->push(new CloseReqBlock(this));
                }
                // SPEECH, VIDEO
                else if(eventType==EVENT_TYPE::O_ANSWER_2.getType() || eventType==EVENT_TYPE::T_ANSWER_2.getType()) {
                    Cause cause(Cause::CS_ITU_T, Cause::L_USER, Cause::CAUSE_NORMAL_CALL_CLEARING);
                    putBlockQueue->push(new ReleaseCallReqBlock(this, getInvokeId(), &cause));
                    putBlockQueue->push(new CloseReqBlock(this));
                }
                else if(eventType==EVENT_TYPE::O_CALLED_PARTY_BUSY_2.getType() ||
                        eventType==EVENT_TYPE::O_NO_ANSWER_2.getType() ||
                        eventType==EVENT_TYPE::O_DISCONNECT_2.getType() ||
                        eventType==EVENT_TYPE::T_BUSY_2.getType() ||
                        eventType==EVENT_TYPE::T_NO_ANSWER_2.getType() ||
                        eventType==EVENT_TYPE::T_DISCONNECT_2.getType()) {
                    putBlockQueue->push(new ContinueReqBlock(this, getInvokeId()));
                    incrementInvokeId();
                    putBlockQueue->push(new DelimiterReqBlock(this, 0));
                    setState(W_CLOSE_0);
                }
                // ABORT
                else if(eventType==EVENT_TYPE::U_ABORT.getType() || eventType==EVENT_TYPE::P_ABORT.getType()) {
                    setState(ABORT_0);
                }
                else setState(W_CLOSE_0);
            }
        }
        // WATCH_REPORT_CALL
        else if(msg->getCode()==CFMessage::WATCH_REPORT_CALL) {
            // watchType
            char watchType=msg->getByte(CFIE::WATCH_TYPE_IE);
            // watchFlags
            char watchFlags=msg->getByte(CFIE::WATCH_FLAGS_IE);

            if(ret!=NULL) {
                // action
                char action=ret->getByte(CFIE::ACTION_IE);

                // ACCEPT
                if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::ACCEPT.getAction()) {
                    // watchArg
                    CompositeIE *watchArgIE=ret->getComposite(CFIE::WATCH_ARG_IE);
                    // DATA
                    if(watchType==WATCH_TYPE::D_A_TIME_WATCH.getType()) {
                        if(watchArgIE!=NULL) {
                            WatchArg watchArg(watchArgIE);
                            if(watchArg.getWatchArg0() > 0) {
                                putBlockQueue->push(new ApplyChargingGPRSReqBlock(this, getInvokeId(), 0, watchArg.getWatchArg0()));
                                incrementInvokeId();
                                putBlockQueue->push(new DelimiterReqBlock(this, 0));
                            }
                        }
                        setState(W_CLOSE_0);
                    }
                    else if(watchType==WATCH_TYPE::D_A_UNIT_WATCH.getType()) {
                        if(watchArgIE!=NULL) {
                            WatchArg watchArg(watchArgIE);
                            if(watchArg.getWatchArg1() > 0) {
                                putBlockQueue->push(new ApplyChargingGPRSReqBlock(this, getInvokeId(), watchArg.getWatchArg1(), 0));
                                incrementInvokeId();
                                putBlockQueue->push(new DelimiterReqBlock(this, 0));
                            }
                        }
                        setState(W_CLOSE_0);
                    }
                    // SPEECH, VIDEO
                    else if(watchType==WATCH_TYPE::A_TIME_WATCH.getType()) {
                        if(watchArgIE!=NULL) {
                            WatchArg watchArg(watchArgIE);
                            unsigned int maxCallPeriodDuration=0;
                            bool releaseIfdurationExceeded=false;
                            if(watchArg.getWatchArg0() > 0) maxCallPeriodDuration=watchArg.getWatchArg0();
                            else if(watchArg.getWatchArg5() > 0) {
                                maxCallPeriodDuration=watchArg.getWatchArg5();
                                releaseIfdurationExceeded=true;
                            }
                            if(maxCallPeriodDuration > 0) {
                                putBlockQueue->push(new ApplyChargingReqBlock(this, getInvokeId(), watchFlags, maxCallPeriodDuration, releaseIfdurationExceeded));
                                incrementInvokeId();
                                putBlockQueue->push(new DelimiterReqBlock(this, 0));
                            }
                        }
                        setState(W_INVOKE);
                    }
                    else if(watchType==WATCH_TYPE::TIME_WATCH.getType()) {
                        setState(W_CLOSE_0);
                    }
                    else setState(W_CLOSE_0);
                }
                // END
                else if(SESSION_ACTION::getSessionAction(action)==SESSION_ACTION::END.getAction()) {
                    // DATA
                    if(watchType==WATCH_TYPE::D_A_TIME_WATCH.getType() ||
                       watchType==WATCH_TYPE::D_TIME_WATCH.getType() ||
                       watchType==WATCH_TYPE::D_A_UNIT_WATCH.getType() ||
                       watchType==WATCH_TYPE::D_UNIT_WATCH.getType()) {
                        putBlockQueue->push(new ReleaseGPRSReqBlock(this, getInvokeId(), 0));
                        incrementInvokeId();
                        putBlockQueue->push(new CloseReqBlock(this));
                    }
                    // SPEECH, VIDEO
                    else if(watchType==WATCH_TYPE::TIME_WATCH.getType()) {
                        Cause cause(Cause::CS_ITU_T, Cause::L_USER, Cause::CAUSE_NORMAL_CALL_CLEARING);
                        putBlockQueue->push(new ReleaseCallReqBlock(this, getInvokeId(), &cause));
                        putBlockQueue->push(new CloseReqBlock(this));
                    }
                }
            } // ret NULL
            else {
                // DATA
                if(watchType==WATCH_TYPE::D_A_TIME_WATCH.getType() ||
                    watchType==WATCH_TYPE::D_TIME_WATCH.getType() ||
                    watchType==WATCH_TYPE::D_A_UNIT_WATCH.getType() ||
                    watchType==WATCH_TYPE::D_UNIT_WATCH.getType()) {
                    putBlockQueue->push(new ReleaseGPRSReqBlock(this, getInvokeId(), 0));
                    incrementInvokeId();
                    putBlockQueue->push(new CloseReqBlock(this));
                }
                // SPEECH, VIDEO
                else if(watchType==WATCH_TYPE::TIME_WATCH.getType()) {
                    setState(W_CLOSE_0);
                }
            }
        }

        delete msg;
        if(ret!=NULL) delete ret;
        delete cfMessage;
    }
};

#endif	/* CAPDIALOG_HPP */
