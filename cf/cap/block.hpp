/*
 * block.hpp
 */
#ifndef BLOCK_HPP
#define	BLOCK_HPP

#include "dialog.hpp"
#include "../../base/appconn/primitiveie.hpp"
#include "../../base/cqueue.hpp"
#include <string>
#include <sstream>

#define CL_SVC_CLASS_0 0

/**
 * <p>The Address class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Address {

public:

    /* constants */
    static char TON;
    /* ton 3 bit 4, 5 and 6 */
    static char TON_UNKNOWN;
    static char TON_INTERNATIONAL;
    static char TON_NATIONAL;
    static char TON_NETWORK_SPECIFIC;
    static char TON_DEDICATED_ACCESS;

    static char NPI;
    /* npi 4 bit 0 to 3 */
    static char NPI_UNKNOWN;
    static char NPI_ISDN;
    static char NPI_DNP;
    static char NPI_TNP;
    static char NPI_NATIONAL;
    static char NPI_PRIVATE;

private:

    /* private variables */
    char ton;
    char npi;
    std::string addr;

public:

    /**
     * Creates a new instance of Address.
     * @param &ton const char
     * @param &npi const char
     * @param &addr const std::string
     */
    Address(const char &ton, const char &npi, const std::string &addr) {
        if(addr.length() > 0) {
            this->ton=ton;
            this->npi=npi;
            this->addr=addr;
        }
        else {
            this->ton=TON_UNKNOWN;
            this->npi=NPI_UNKNOWN;
            this->addr="";
        }
    }

    /**
     * Creates a new instance of Address.
     * @param *addr char
     * @param &len const int
     */
    Address(char *addr, const int &len) {
        if(len > 0) {
            ton=addr[0] & TON;
            npi=addr[0] & NPI;
            this->addr=toSemiOctetString(addr + 1, len - 1);
        }
        else {
            ton=TON_UNKNOWN;
            npi=NPI_UNKNOWN;
            this->addr="";
        }
    }

    virtual ~Address() {}

    /**
     * Returns the address.
     * @param *ch char
     * @param &len int
     */
    void getAddress(char *ch, int &len) {
        ch[0]=0x80;
        ch[0]|=ton & TON;
        ch[0]|=npi & NPI;
        toSemiOctet(addr, ch + 1, len);
        len+=1;
    }

    /**
     * Returns the ton.
     * @return char
     */
    char getTON() {
        return ton;
    }

    /**
     * Returns the npi.
     * @return char
     */
    char getNPI() {
        return npi;
    }

    /**
     * Returns the addr.
     * @return std::string
     */
    std::string getAddr() {
        return addr;
    }

    /**
     * Returns the semi octet.
     * @param &str const std::string
     * @param *so char
     * @param &len int
     */
    static void toSemiOctet(const std::string &s, char *so, int &len) {
        len=s.length() >> 1;
        if((s.length() & 1)==1) len++;
        for(int i=0; i < len; i++) {
            char o=hexToByte(s.at(i << 1));
            if(o!=-1) so[i]=o;
            if(((i << 1) + 1) < (int)s.length()) {
                o=hexToByte(s.at((i << 1) + 1));
                if(o!=-1) so[i]|=o << 4;
            }
            else so[i]|=0xf0;
        }
    }

    /**
     * Return the string for a semi octet.
     * @param *so char
     * @param &len const int
     * @return std::string
     */
    static std::string toSemiOctetString(char *so, const int &len) {
        std::string str="";
        for(int i=0; i < len; i++) {
            int l=so[i] & 0xf;
            int h=(so[i] >> 4) & 0xf;
            if(l < 0xf) str+=hexCh[l];
            if(h < 0xf) str+=hexCh[h];
        }
        return str;
    }
};

char Address::TON=0x70;

char Address::TON_UNKNOWN=0x0;
char Address::TON_INTERNATIONAL=0x10;
char Address::TON_NATIONAL=0x20;
char Address::TON_NETWORK_SPECIFIC=0x30;
char Address::TON_DEDICATED_ACCESS=0x40;

char Address::NPI=0xf;

char Address::NPI_UNKNOWN=0x0;
char Address::NPI_ISDN=0x1;
char Address::NPI_DNP=0x3;
char Address::NPI_TNP=0x4;
char Address::NPI_NATIONAL=0x8;
char Address::NPI_PRIVATE=0x9;

/**
 * <p>The Number class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Number {

public:

    /* constants */
    static char OEI;
    /* oei 1 bit 7 */
    static char OEI_EVEN;
    static char OEI_ODD;

    static char NAI;
    /* nai 7 bit 0 to 6 */
    static char NAI_SUBSCRIBER_NUMBER;
    static char NAI_UNKNOWN;
    static char NAI_NATIONAL;
    static char NAI_INTERNATIONAL;

    static char NPI;
    /* npi 3 bit 4 to 6 */
    static char NPI_ISDN;
    static char NPI_DNP;
    static char NPI_TNP;

private:

    /* private variables */
    char oei;
    char nai;
    char npi;
    std::string addr;

public:

    /**
     * Creates a new instance of Number.
     * @param &nai const char
     * @param &npi const char
     * @param &addr const std::string
     */
    Number(const char &nai, const char &npi, const std::string &addr) {
        if((addr.length() & 1)==0) oei=OEI_EVEN;
        else oei=OEI_ODD;
        this->nai=nai;
        this->npi=npi;
        this->addr=addr;
    }

    /**
     * Creates a new instance of Number.
     * @param *nbr char
     * @param &len const int
     */
    Number(char *nbr, const int &len) {
        if(len > 1) {
            oei=nbr[0] & OEI;
            nai=nbr[0] & NAI;
            npi=nbr[1] & NPI;
            addr=toSemiOctetString(oei, nbr + 2, len - 2);
        }
        else {
            oei=OEI_EVEN;
            nai=0;
            npi=0;
            addr="";
        }
    }

    virtual ~Number() {}

protected:

    /**
     * Returns the number.
     * @param *ch char
     * @param &len int
     */
    void getNumber(char *ch, int &len) {
        ch[0]=0;
        ch[0]|=oei & OEI;
        ch[0]|=nai & NAI;
        ch[1]=npi & NPI;
        toSemiOctet(addr, ch + 2, len);
        len+=2;
    }

public:

    /**
     * Returns the nai.
     * @return char
     */
    char getNAI() {
        return nai;
    }

    /**
     * Returns the npi.
     * @return char
     */
    char getNPI() {
        return npi;
    }

    /**
     * Returns the addr.
     * @return std::string
     */
    std::string getAddr() {
        return addr;
    }

    /**
     * Returns the semi octet.
     * @param &str const std::string
     * @param *so char
     * @param &len int
     */
    static void toSemiOctet(const std::string &s, char *so, int &len) {
        int slen=s.size();
        len=slen >> 1;
        if((slen & 1)==1) len++;
        int soi=0;
        for(int si=0; si < slen; si++) {
            char o=hexToByte(s.at(si));
            if(o!=-1) {
                if((si & 1)==0) so[soi]=o;
                else {
                    so[soi]|=o << 4;
                    soi++;
                }
            }
        }
    }

    /**
     * Return the string for the semi octet.
     * @param &oei const char
     * @param *so char
     * @param &len const int
     * @return std::string
     */
    static std::string toSemiOctetString(const char &oei, char *so, const int &len) {
        std::string s="";
        int tmpLen=len << 1;
        if((oei & OEI)==OEI_ODD) tmpLen-=1;
        int soi=0;
        for(int si=0; si < tmpLen; si++) {
            if((si & 1)==0)
                s=s + hexCh[so[soi] & 0xf];
            else {
                s=s + hexCh[(so[soi] >> 4) & 0xf];
                soi++;
            }
        }
        return s;
    }
};

char Number::OEI=0x80;

char Number::OEI_EVEN=0x0;
char Number::OEI_ODD=0x80;

char Number::NAI=0x7f;

char Number::NAI_SUBSCRIBER_NUMBER=0x1;
char Number::NAI_UNKNOWN=0x2;
char Number::NAI_NATIONAL=0x3;
char Number::NAI_INTERNATIONAL=0x4;

char Number::NPI=0x70;

char Number::NPI_ISDN=0x10;
char Number::NPI_DNP=0x30;
char Number::NPI_TNP=0x40;

/**
 * <p>The CallingPartyNumber class. See Q.763</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CallingPartyNumber : public Number {

public:

    /* constants */
    static char NI;
    /* ni 1 bit 7 */
    static char NI_COMPLETE;
    static char NI_INCOMPLETE;

    static char APRI;
    /* apri 2 bit 2 and 3 */
    static char APRI_PRESENTATION_ALLOWED;
    static char APRI_PRESENTATION_RESTRICTED;
    static char APRI_ADDRESS_NA;

    static char SI;
    /* apri 2 bit 0 and 1 */
    static char SI_USER_PROVIDED_NV;
    static char SI_USER_PROVIDED_VP;
    static char SI_USER_PROVIDED_VF;
    static char SI_NETWORK_PROVIDED;

private:

    /* private variables */
    char ni;
    char apri;
    char si;

public:

    /**
     * Creates a new instance of CallingPartyNumber.
     * @param &nai const char
     * @param &npi const char
     * @param &addr const std::string
     * @param &ni const char
     * @param &apri const char
     * @param &si const char
     */
    CallingPartyNumber(const char &nai, const char &npi, const std::string &addr, const char &ni, const char &apri, const char &si) : Number(nai, npi, addr) {
        this->ni=ni;
        this->apri=apri;
        this->si=si;
    }

    /**
     * Creates a new instance of CallingPartyNumber.
     * @param *nbr char
     * @param &len const int
     */
    CallingPartyNumber(char *nbr, const int &len) : Number(nbr, len) {
        if(len > 1) {
            ni=nbr[1] & NI;
            apri=nbr[1] & APRI;
            si=nbr[1] & SI;
        }
        else {
            ni=0;
            apri=0;
            si=0;
        }
    }

    virtual ~CallingPartyNumber() {}

    /**
     * Returns the callingPartyNumber.
     * @param *ch char
     * @param &len int
     */
    void getCallingPartyNumber(char *ch, int &len) {
        Number::getNumber(ch, len);
        ch[1]|=ni & NI;
        ch[1]|=apri & APRI;
        ch[1]|=si & SI;
    }

    /**
     * Returns the ni.
     * @return char
     */
    char getNI() {
        return ni;
    }

    /**
     * Returns the apri.
     * @return char
     */
    char getAPRI() {
        return apri;
    }

    /**
     * Returns the si.
     * @return char
     */
    char getSI() {
        return si;
    }
};

char CallingPartyNumber::NI=0x80;

char CallingPartyNumber::NI_COMPLETE=0x0;
char CallingPartyNumber::NI_INCOMPLETE=0x80;

char CallingPartyNumber::APRI=0xc;

char CallingPartyNumber::APRI_PRESENTATION_ALLOWED=0x0;
char CallingPartyNumber::APRI_PRESENTATION_RESTRICTED=0x4;
char CallingPartyNumber::APRI_ADDRESS_NA=0x8;

char CallingPartyNumber::SI=0x3;

char CallingPartyNumber::SI_USER_PROVIDED_NV=0x0;
char CallingPartyNumber::SI_USER_PROVIDED_VP=0x1;
char CallingPartyNumber::SI_USER_PROVIDED_VF=0x2;
char CallingPartyNumber::SI_NETWORK_PROVIDED=0x3;

/**
 * <p>The CalledPartyNumber class. See Q.763</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CalledPartyNumber : public Number {

public:

    /* constants */
    static char INN;
    /* inn 1 bit 7 */
    static char INN_RINNA;
    static char INN_RINNNA;

private:

    /* private variables */
    char inn;

public:

    /**
     * Creates a new instance of CalledPartyNumber.
     * @param &nai const char
     * @param &npi const char
     * @param &addr const std::string
     * @param &inn const char
     */
    CalledPartyNumber(const char &nai, const char &npi, const std::string &addr, const char &inn) : Number(nai, npi, addr) {
        this->inn=inn;
    }

    /**
     * Creates a new instance of CalledPartyNumber.
     * @param *nbr char
     * @param &len const int
     */
    CalledPartyNumber(char *nbr, const int &len) : Number(nbr, len) {
        if(len > 1) inn=nbr[1] & INN;
        else inn=0;
    }

    /**
     * Returns the calledPartyNumber.
     * @param *ch char
     * @param &len int
     */
    void getCalledPartyNumber(char *ch, int &len) {
        Number::getNumber(ch, len);
        ch[1]|=inn & INN;
    }

    /**
     * Returns the inn.
     * @return char
     */
    char getINN() {
        return inn;
    }
};

char CalledPartyNumber::INN=0x80;

char CalledPartyNumber::INN_RINNA=0x0;
char CalledPartyNumber::INN_RINNNA=0x80;

/**
 * <p>The GenericNumber class. See Q.763</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class GenericNumber : public CallingPartyNumber {

public:

    /* constants */
    static char NQI_RESERVED_DIALED_DIGITS;
    static char NQI_ADDITIONAL_CALLED_NUMBER;
    static char NQI_ADDITIONAL_CONNECTED_NUMBER;
    static char NQI_ADDITIONAL_CALLING_PARTY_NUMBER;

private:

    /* private variables */
    char nqi;

public:

    /**
     * Creates a new instance of GenericNumber.
     * @param &nai const char
     * @param &npi const char
     * @param &addr const std::string
     * @param &ni const char
     * @param &apri const char
     * @param &si const char
     * @param &nqi const char
     */
    GenericNumber(const char &nai, const char &npi, const std::string &addr, const char &ni, const char &apri, const char &si, const char &nqi) : CallingPartyNumber(nai, npi, addr, ni, apri, si) {
        this->nqi=nqi;
    }

    /**
     * Creates a new instance of GenericNumber.
     * @param *nbr char
     * @param &len const int
     */
    GenericNumber(char *nbr, const int &len) : CallingPartyNumber(nbr + 1, len - 1) {
        if(len > 0) nqi=nbr[0];
        else nqi=0;
    }

    /**
     * Returns the genericNumber.
     * @param *ch char
     * @param &len int
     */
    void getGenericNumber(char *ch, int &len) {
        ch[0]=nqi;
        CallingPartyNumber::getCallingPartyNumber(ch + 1, len);
        len++;
    }

    /**
     * Returns the nqi.
     * @return char
     */
    char getNQI() {
        return nqi;
    }
};

char GenericNumber::NQI_RESERVED_DIALED_DIGITS=0x0;
char GenericNumber::NQI_ADDITIONAL_CALLED_NUMBER=0x1;
char GenericNumber::NQI_ADDITIONAL_CONNECTED_NUMBER=0x5;
char GenericNumber::NQI_ADDITIONAL_CALLING_PARTY_NUMBER=0x6;

/**
 * <p>The GenericNumber class. See Q.850</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Cause {

public:

    /* constants */
    static char CODING_STANDARD;
    /* codingStandard 2 bit 5 and 6 */
    static char CS_ITU_T;
    static char CS_ISO_IEC;

    static char LOCATION;
    /* location 4 bit 0 to 3 */
    static char L_USER;

    static char CAUSE;
    /* cause 7 bit 0 to 6 */
    static char CAUSE_NORMAL_CALL_CLEARING;

private:

    /* private variables */
    char codingStandard;
    char location;
    char cause;

public:

    /**
     * Creates a new instance of Cause.
     * @param &codingStandard const char
     * @param &location const char
     * @param &cause const char
     */
    Cause(const char &codingStandard, const char &location, const char &cause) {
        this->codingStandard=codingStandard;
        this->location=location;
        this->cause=cause;
    }

    /**
     * Creates a new instance of Cause.
     * @param *ch char
     * @param &len const int
     */
    Cause(char *ch, const int &len) {
        if(len==2) {
            codingStandard=ch[0] & CODING_STANDARD;
            location=ch[0] & LOCATION;
            cause=ch[1] & CAUSE;
        }
        else {
            codingStandard=0;
            location=0;
            cause=0;
        }
    }

    virtual ~Cause() {}

    /**
     * Returns the cause.
     * @param *ch char
     * @param &len int
     */
    void getCause(char *ch, int &len) {
        ch[0]=0x80 | codingStandard | location;
        ch[1]=0x80 | cause;
        len=2;
    }

    /**
     * Returns the codingStandard.
     * @return char
     */
    char getCodingStandard() {
        return codingStandard;
    }

    /**
     * Returns the location.
     * @return char
     */
    char getLocation() {
        return location;
    }

    /**
     * Returns the cause.
     * @return char
     */
    char getCause() {
        return cause;
    }

};

char Cause::CODING_STANDARD=0x60;

char Cause::CS_ITU_T=0x00;
char Cause::CS_ISO_IEC=0x20;

char Cause::LOCATION=0xf;

char Cause::L_USER=0x0;

char Cause::CAUSE=0x7f;

char Cause::CAUSE_NORMAL_CALL_CLEARING=0x10;

/**
 * <p>The CellGlobalId class. See 3GPP TS 23.003 V4.8.0, 3GPP TS 24.008 V4.8.0</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class CellGlobalId {

    /* private variables */
    std::string mcc;
    std::string mnc;
    int lac;
    int ci;

public:

    /**
     * Creates a new instance of CellGlobalId.
     * @param *ch char
     * @param &len const int
     */
    CellGlobalId(char *ch, const int &len) {
        if(len==7) {
            mcc=Number::toSemiOctetString(Number::OEI_ODD, ch, 2);
            mnc=Number::toSemiOctetString(Number::OEI_EVEN, (ch + 2), 1);
            lac=((ch[3] & 0xff) << 8) | (ch[4] & 0xff);
            ci=((ch[5] & 0xff) << 8) | (ch[6] & 0xff);
        }
        else {
            mcc="";
            mnc="";
            lac=0;
            ci=0;
        }
    }

    virtual ~CellGlobalId() {}

    /**
     * Returns the cellGlobalId.
     * @return std::string
     */
    std::string getCellGlobalId() {
        std::stringstream ss;
        ss << mcc << mnc;
        std::stringstream tmpLAC;
        tmpLAC << lac;
        for(int i=tmpLAC.str().length(); i < 5; i++) ss << '0';
        ss << tmpLAC.str();
        std::stringstream tmpCI;
        tmpCI << ci;
        for(int i=tmpCI.str().length(); i < 5; i++) ss << '0';
        ss << tmpCI.str();
        return ss.str();
    }

    /**
     * Returns the mcc.
     * @return std::string
     */
    std::string getMCC() {
        return mcc;
    }

    /**
     * Returns the mnc.
     * @return std::string
     */
    std::string getMNC() {
        return mnc;
    }

    /**
     * Returns the lac.
     * @return int
     */
    int getLAC() {
        return lac;
    }

    /**
     * Returns the ci.
     * @return int
     */
    int getCI() {
        return ci;
    }
};

/**
 * <p>The AccessPointName class. See 3GPP TS 23.003 V4.8.0, 3GPP TS 24.008 V4.8.0</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class AccessPointName {

    /* private variables */
    std::string accessPointName;

public:

    /**
     * Creates a new instance of AccessPointName.
     * @param *ch char
     * @param &len const int
     */
    AccessPointName(char *ch, const int &len) {
        if(len > 0) {
            int chi=0;
            while(chi < len) {
                int ll=ch[chi];
                for(int i=0; i < ll; i++)
                    accessPointName+=ch[chi + i + 1];
                chi+=ll + 1;
                if(chi < len) accessPointName+='.';
            }
        }
        else accessPointName="";
    }

    virtual ~AccessPointName() {}

    /**
     * Returns the accessPointName.
     * @return std::string
     */
    std::string getAccessPointName() {
        return accessPointName;
    }
};

/**
 * Returns the natureOfAddres for a given type.
 * @param &ch const char
 * @return NatureOfAddress
 */
static U8 getNatureOfAddress(const char &ch) {
    char ton=ch & Address::TON;
    if(ton==Address::TON_INTERNATIONAL) return NA_INTERNATIONAL_NUM;
    else if(ton==Address::TON_NATIONAL) return NA_NATIONAL_SIGNIFANT_NUM;
    else if(ton==Address::TON_DEDICATED_ACCESS) return NA_SUSCRIBER_NUM;
    else return NA_UNKNOWN;
}

/**
 * Returns the numberingPlan for a given type.
 * @param &ch const char
 * @return NumberingPlan
 */
static SccpNumberingPlan getNumberingPlan(const char &ch) {
    char npi=ch & Address::NPI;
    if(npi==Address::NPI_ISDN) return NP_ISDN;
    return NP_UNKNOWN;
}

/**
 * <p>The Block class.</p>
 * <p>Nuevatel PCS de Bolivia S.A. (c) 2013</p>
 *
 * @author Eduardo Marin
 * @version 2.0
 */
class Block {

    /* private variables */
    Dialog *dialog;

protected:

    /** The capBlock. */
    CapBlock capBlock;

    /**
     * Creates a new instance of Block.
     * @param *dialog Dialog
     * @param &bitMask const unsigned char
     * @param &serviceType const CapServiceType
     * @param &serviceMsg const int
     * @param &applicationId const unsigned short
     * @param &invokeId const short
     * @param &linkedId const short
     */
    Block(Dialog *dialog,
          const unsigned char &bitMask,
          const CapServiceType &serviceType,
          const int &serviceMsg,
          const unsigned short &applicationId,
          const short &invokeId,
          const short &linkedId) {
        this->dialog=dialog;
        capBlock.bit_mask=bitMask;
        capBlock.serviceType=serviceType;
        capBlock.serviceMsg=serviceMsg;
        capBlock.applicationId=applicationId;
        capBlock.invokeId=invokeId;
        capBlock.linkedId=linkedId;
    }

public:

    virtual ~Block() {}

    /**
     * Returns the capBlock.
     * @return *CapBlock
     */
    CapBlock* getCapBlock() {
        capBlock.dialogId=dialog->getDialogId();
        return &capBlock;
    }

    /**
     * Returns the dialog.
     * @return *Dialog
     */
    Dialog* getDialog() {
        return dialog;
    }
};

/**
 * The PutBlockQueue abstract class.
 */
class PutBlockQueue : public Thread {

protected:

    /** The blockQueue. */
    Queue<Block> blockQueue;

public:

    /**
     * Creates a new instance of PutBlockQueue.
     */
    PutBlockQueue() {}

    virtual ~PutBlockQueue() {}

    /**
     * Pushes a block.
     * @param *block Block
     */
    void push(Block *block) {
        blockQueue.push(block);
    }
};

/**
 * The ActivityTestReqBlock class.
 */
class ActivityTestReqBlock : public Block {

public:

    /**
     * Creates a new instance of ActivityTestReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     */
    ActivityTestReqBlock(Dialog *dialog, const short &invokeId) : Block(dialog, 0, capReq, CAP_ACTIVITY_TEST, 0, invokeId, -1) {}
};

/**
 * The ApplyChargingGPRSReqBlock class.
 */
class ApplyChargingGPRSReqBlock : public Block {

public:

    /**
     * Creates a new instance of ApplyChargingGPRSReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     * @param &maxTransferredVolume const unsigned int
     * @param &maxElapsedTime const unsigned int
     */
    ApplyChargingGPRSReqBlock(Dialog *dialog, const short &invokeId, const unsigned int &maxTransferredVolume, const unsigned int &maxElapsedTime) : Block(dialog, CapBlock_parameter_present, capReq, CAP_APPLY_CHARGING_GPRS, 0, invokeId, -1) {
        capBlock.parameter.applyChargingGPRSArg_v3.bit_mask=0;

        if(maxTransferredVolume > 0) {
            capBlock.parameter.applyChargingGPRSArg_v3.chargingCharacteristics.choice=CapChargingCharacteristics_maxTransferredVolume_chosen;
            capBlock.parameter.applyChargingGPRSArg_v3.chargingCharacteristics.u.maxTransferredVolume=maxTransferredVolume;
        }
        else if(maxElapsedTime > 0) {
            capBlock.parameter.applyChargingGPRSArg_v3.chargingCharacteristics.choice=CapChargingCharacteristics_maxElapsedTime_chosen;
            capBlock.parameter.applyChargingGPRSArg_v3.chargingCharacteristics.u.maxElapsedTime=maxElapsedTime;
        }
    }
};

/**
 * The ApplyChargingReportGPRSResBlock class.
 */
class ApplyChargingReportGPRSResBlock : public Block {

public:

    /**
     * Creates a new instance of ApplyChargingReportGPRSResBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     */
    ApplyChargingReportGPRSResBlock(Dialog *dialog, const short &invokeId) : Block(dialog, 0, capRsp, CAP_APPLY_CHARGING_REPORT_GPRS, 0, invokeId, -1) {}
};

/**
 * The ApplyChargingReqBlock class.
 */
class ApplyChargingReqBlock : public Block {

public:

    /**
     * Creates a new instance of ApplyChargingReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     * @param &sendingSideId const unsigned char
     * @param &maxCallPeriodDuration const unsigned int
     * @param &releaseIfdurationExceeded const bool
     */
    ApplyChargingReqBlock(Dialog *dialog, const short &invokeId, const unsigned char &sendingSideId, const unsigned int &maxCallPeriodDuration, const bool &releaseIfdurationExceeded) : Block(dialog, CapBlock_parameter_present | CapBlock_subParameter_present, capReq, CAP_APPLY_CHARGING, 0, invokeId, -1) {
        capBlock.parameter.applyChargingArg_v2.bit_mask=CapApplyChargingArg_v2_partyToCharge_present;
        capBlock.parameter.applyChargingArg_v2.partyToCharge.choice=CapSendingSideID_sendingSideID_chosen;
        capBlock.parameter.applyChargingArg_v2.partyToCharge.u.sendingSideID.length=1;
        capBlock.parameter.applyChargingArg_v2.partyToCharge.u.sendingSideID.value[0]=sendingSideId;
        capBlock.parameter.applyChargingArg_v2.aChBillingChargingCharacteristics.length=0;

        capBlock.subParameter.cAMEL_AChBillingChargingCharacteristics_v2.choice=CapCAMEL_AChBillingChargingCharacteristics_v2_timeDurationCharging_chosen;
        capBlock.subParameter.cAMEL_AChBillingChargingCharacteristics_v2.u.timeDurationCharging.bit_mask=0;
        capBlock.subParameter.cAMEL_AChBillingChargingCharacteristics_v2.u.timeDurationCharging.maxCallPeriodDuration=maxCallPeriodDuration;
        if(releaseIfdurationExceeded) {
            capBlock.subParameter.cAMEL_AChBillingChargingCharacteristics_v2.u.timeDurationCharging.bit_mask=CapCAMEL_AChBillingChargingCharacteristics_v2_timeDurationCharging_releaseIfdurationExceeded_present;
            capBlock.subParameter.cAMEL_AChBillingChargingCharacteristics_v2.u.timeDurationCharging.releaseIfdurationExceeded.bit_mask=CapReleaseIfDurationExceeded_tone_present;
            capBlock.subParameter.cAMEL_AChBillingChargingCharacteristics_v2.u.timeDurationCharging.releaseIfdurationExceeded.tone=true;
        }
    }
};

/**
 * The CloseReqBlock class.
 */
class CloseReqBlock : public Block {

public:

    /**
     * Creates a new instance of CloseReqBlock.
     * @param *dialog Dialog
     */
    CloseReqBlock(Dialog *dialog) : Block(dialog, CapBlock_parameter_present, capReq, CAP_CLOSE, 0, 0, -1) {
        capBlock.parameter.closeArg.releaseMethod=capNormalRelease;
    }
};

/**
 * The ConnectReqBlock class.
 */
class ConnectReqBlock : public Block {

public:

    /**
     * Creates a new instance of ConnectReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     * @param *calledPartyNumber CalledPartyNumber
     * @param *genericNumber GenericNumber
     */
    ConnectReqBlock(Dialog *dialog, const short &invokeId, CalledPartyNumber *calledPartyNumber, GenericNumber *genericNumber) : Block(dialog, CapBlock_parameter_present, capReq, CAP_CONNECT, 0, invokeId, -1) {
        int len=32;
        char *ch=new char[len];
        calledPartyNumber->getCalledPartyNumber(ch, len);
        capBlock.parameter.connectArg_v2.bit_mask=0;
        capBlock.parameter.connectArg_v2.destinationRoutingAddress.count=1;
        capBlock.parameter.connectArg_v2.destinationRoutingAddress.value[0].length=len;
        memcpy(capBlock.parameter.connectArg_v2.destinationRoutingAddress.value[0].value, ch, len);

        if(genericNumber!=NULL) {
            genericNumber->getGenericNumber(ch, len);
            capBlock.parameter.connectArg_v2.bit_mask=CapConnectArg_v2_genericNumbers_present;
            capBlock.parameter.connectArg_v2.genericNumbers.count=1;
            capBlock.parameter.connectArg_v2.genericNumbers.value[0].length=len;
            memcpy(capBlock.parameter.connectArg_v2.genericNumbers.value[0].value, ch, len);
        }
        delete[] ch;
    }
};

/**
 * The ContinueGPRSReqBlock class.
 */
class ContinueGPRSReqBlock : public Block {

public:

    /**
     * Creates a new instance of ContinueGPRSReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     */
    ContinueGPRSReqBlock(Dialog *dialog, const short &invokeId) : Block(dialog, CapBlock_parameter_present, capReq, CAP_CONTINUE_GPRS, 0, invokeId, -1) {
        capBlock.parameter.continueGPRSArg_v3.bit_mask=0;
    }
};

/**
 * The ContinueReqBlock class.
 */
class ContinueReqBlock : public Block {

public:

    /**
     * Creates a new instance of ContinueReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     */
    ContinueReqBlock(Dialog *dialog, const short &invokeId) : Block(dialog, 0, capReq, CAP_CONTINUE, 0, invokeId, -1) {}
};

/**
 * The DelimiterReqBlock class.
 */
class DelimiterReqBlock : public Block {

public:

    /**
     * Creates a new instance of DelimiterReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     */
    DelimiterReqBlock(Dialog *dialog, const short &invokeId) : Block(dialog, CapBlock_parameter_present, capReq, CAP_DELIMITER, 0, invokeId, -1) {
        capBlock.parameter.delimiter.qualityOfService=CL_SVC_CLASS_0;
    }
};

/**
 * The DisconnectForwardConnectionReqBlock class.
 */
class DisconnectForwardConnectionReqBlock : public Block {

public:

    /**
     * Creates a new instance of DisconnectForwardConnectionReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     */
    DisconnectForwardConnectionReqBlock(Dialog *dialog, const short &invokeId) : Block(dialog, 0, capReq, CAP_DISCONNECT_FORWARD_CONNECTION, 0, invokeId, -1) {}
};

/**
 * The EstablishTemporaryConnectionReqBlock class.
 */
class EstablishTemporaryConnectionReqBlock : public Block {

public:

    /**
     * Creates a new instance of EstablishTemporaryConnectionReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     * @param *genericNumber GenericNumber
     */
    EstablishTemporaryConnectionReqBlock(Dialog *dialog, const short &invokeId, GenericNumber *genericNumber) : Block(dialog, CapBlock_parameter_present, capReq, CAP_ESTABLISH_TEMPORARY_CONNECTION, 0, invokeId, -1) {
        int len=32;
        char *ch=new char[len];
        genericNumber->getGenericNumber(ch, len);
        capBlock.parameter.establishTemporaryConnectionArg_v2.bit_mask=0;
        capBlock.parameter.establishTemporaryConnectionArg_v2.assistingSSPIPRoutingAddress.length=len;
        memcpy(capBlock.parameter.establishTemporaryConnectionArg_v2.assistingSSPIPRoutingAddress.value, ch, len);
        delete[] ch;
    }
};

/**
 * The EventReportGPRSResBlock class.
 */
class EventReportGPRSResBlock : public Block {

public:

    /**
     * Creates a new instance of EventReportGPRSResBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     */
    EventReportGPRSResBlock(Dialog *dialog, const short &invokeId) : Block(dialog, 0, capRsp, CAP_EVENT_REPORT_GPRS, 0, invokeId, -1) {}
};

/**
 * The EntityReleasedGPRSResBlock class.
 */
class EntityReleasedGPRSResBlock : public Block {

public:

    /**
     * Creates a new instance of EntityReleasedGPRSResBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     */
    EntityReleasedGPRSResBlock(Dialog *dialog, const short &invokeId) : Block(dialog, 0, capRsp, CAP_ENTITY_RELEASED_GPRS, 0, invokeId, -1) {}
};

/**
 * The OpenResBlock class.
 */
class OpenResBlock : public Block {

public:

    /* constants for gprsRerence */
    static char GPRS_REFERENCE_NOT_PRESENT;
    static char GPRS_ORIGINATION_REFERENCE_PRESENT;
    static char GPRS_DESTINATION_REFERENCE_PRESENT;

    /**
     * Creates a new instance of OpenResBlock.
     * @param *dialog Dialog
     * @param &result const CapDialogResult
     * @param &gprsReference const char
     * @param &gprsOriginationReference const unsigned int
     * @param &gprsDestinationReference const unsigned int
     */
    OpenResBlock(Dialog *dialog, const CapDialogResult &result, const char &gprsReference, const unsigned int &gprsOriginationReference, const unsigned int &gprsDestinationReference) : Block(dialog, CapBlock_parameter_present, capRsp, CAP_OPEN, 0, 0, -1) {
        capBlock.parameter.openRes.bit_mask=0;
        capBlock.parameter.openRes.result=result;
        capBlock.parameter.openRes.dialogPortion.bit_mask=0;
        capBlock.parameter.openRes.dialogPortion.userInformationLength=0;
        if(result==capDialogAccepted) {
            if(gprsReference!=GPRS_REFERENCE_NOT_PRESENT) {
                capBlock.parameter.openRes.bit_mask=CapOpenRes_dialogPortion_present;
                if((gprsReference & GPRS_ORIGINATION_REFERENCE_PRESENT)==GPRS_ORIGINATION_REFERENCE_PRESENT) {
                    capBlock.parameter.openRes.dialogPortion.bit_mask|=CapDialogPortion_gprsOriginatingReference_present;
                    capBlock.parameter.openRes.dialogPortion.gprsOriginatingReference=gprsOriginationReference;
                }
                if((gprsReference & GPRS_DESTINATION_REFERENCE_PRESENT)==GPRS_DESTINATION_REFERENCE_PRESENT) {
                    capBlock.parameter.openRes.dialogPortion.bit_mask|=CapDialogPortion_gprsDestinationReference_present;
                    capBlock.parameter.openRes.dialogPortion.gprsDestinationReference=gprsDestinationReference;
                }
            }
        }
    }

    /**
     * Creates a new instance of OpenResBlock.
     * @param *dialog Dialog
     * @param &result const CapDialogResult
     * @param &reason const CapOpenRefuseReason
     */
    OpenResBlock(Dialog *dialog, const CapDialogResult &result, const CapOpenRefuseReason &reason) : Block(dialog, CapBlock_parameter_present, capRsp, CAP_OPEN, 0, 0, -1) {
        capBlock.parameter.openRes.bit_mask=CapOpenRes_refuseReason_present;
        capBlock.parameter.openRes.result=result;
        capBlock.parameter.openRes.dialogPortion.bit_mask=0;
        capBlock.parameter.openRes.dialogPortion.userInformationLength=0;
        if(result==capDialogRefused) capBlock.parameter.openRes.refuseReason=reason;
    }
};

char OpenResBlock::GPRS_REFERENCE_NOT_PRESENT=0x0;
char OpenResBlock::GPRS_ORIGINATION_REFERENCE_PRESENT=0x1;
char OpenResBlock::GPRS_DESTINATION_REFERENCE_PRESENT=0x2;

/**
 * The ReleaseCallReqBlock class.
 */
class ReleaseCallReqBlock : public Block {

public:

    /**
     * Creates a new instance of ReleaseCallReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     * @param *cause Cause
     */
    ReleaseCallReqBlock(Dialog *dialog, const short &invokeId, Cause *cause) : Block(dialog, CapBlock_parameter_present, capReq, CAP_RELEASE_CALL, 0, invokeId, -1) {
        int len=2;
        char *ch=new char[len];
        cause->getCause(ch, len);
        capBlock.parameter.releaseCallArg_v2.length=len;
        memcpy(capBlock.parameter.releaseCallArg_v2.value, ch, len);
        delete[] ch;
    }
};

/**
 * The ReleaseGPRSReqBlock class.
 */
class ReleaseGPRSReqBlock : public Block {

public:

    /**
     * Creates a new instance of ReleaseGPRSReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     * @param &gprsCause const unsigned char
     */
    ReleaseGPRSReqBlock(Dialog *dialog, const short &invokeId, const unsigned char &gprsCause) : Block(dialog, CapBlock_parameter_present, capReq, CAP_RELEASE_GPRS, 0, invokeId, -1) {
        capBlock.parameter.releaseGPRSArg_v3.bit_mask=0;
        capBlock.parameter.releaseGPRSArg_v3.gprsCause.length=1;
        capBlock.parameter.releaseGPRSArg_v3.gprsCause.value[0]=gprsCause;
    }
};

/**
 * The RequestReportBCSMEventReqBlock class.
 */
class RequestReportBCSMEventReqBlock : public Block {

public:

    /* constants for bcsmEvents */
    static int ROUTE_SELECT_FAILURE_2;
    static int O_CALLED_PARTY_BUSY_2;
    static int O_NO_ANSWER_2;
    static int O_ANSWER_2;
    static int O_DISCONNECT_1;
    static int O_DISCONNECT_2;
    static int O_ABANDON_1;
    static int O_ALL;
    static int T_BUSY_2;
    static int T_NO_ANSWER_2;
    static int T_ANSWER_2;
    static int T_DISCONNECT_1;
    static int T_DISCONNECT_2;
    static int T_ABANDON_1;
    static int T_ALL;

    /**
     * Creates a new instance of RequestReportBCSMEventReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     * @param &bcsmEvents const int
     */
    RequestReportBCSMEventReqBlock(Dialog *dialog, const short &invokeId, const int &bcsmEvents) : Block(dialog, CapBlock_parameter_present, capReq, CAP_REQUEST_REPORT_BCSM_EVENT, 0, invokeId, -1) {
        capBlock.parameter.requestReportBCSMEventArg_v2.bit_mask=0;
        unsigned short count=0;
        if(bcsmEvents<=O_ALL) {
            if((bcsmEvents & ROUTE_SELECT_FAILURE_2)==ROUTE_SELECT_FAILURE_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_routeSelectFailure, capMonitorMode_t2_notifyAndContinue, 2, 0);
            if((bcsmEvents & O_CALLED_PARTY_BUSY_2)==O_CALLED_PARTY_BUSY_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_oCalledPartyBusy, capMonitorMode_t2_interrupted, 2, 0);
            if((bcsmEvents & O_NO_ANSWER_2)==O_NO_ANSWER_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_oNoAnswer, capMonitorMode_t2_interrupted, 2, 40);
            if((bcsmEvents & O_ANSWER_2)==O_ANSWER_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_oAnswer, capMonitorMode_t2_notifyAndContinue, 2, 0);
            if((bcsmEvents & O_DISCONNECT_1)==O_DISCONNECT_1)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_oDisconnect, capMonitorMode_t2_notifyAndContinue, 1, 0);
            if((bcsmEvents & O_DISCONNECT_2)==O_DISCONNECT_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_oDisconnect, capMonitorMode_t2_interrupted, 2, 0);
            if((bcsmEvents & O_ABANDON_1)==O_ABANDON_1)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_oAbandon, capMonitorMode_t2_notifyAndContinue, 1, 0);
        }
        else {
            if((bcsmEvents & T_BUSY_2)==T_BUSY_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_tBusy, capMonitorMode_t2_interrupted, 2, 0);
            if((bcsmEvents & T_NO_ANSWER_2)==T_NO_ANSWER_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_tNoAnswer, capMonitorMode_t2_interrupted, 2, 0);
            if((bcsmEvents & T_ANSWER_2)==T_ANSWER_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_tAnswer, capMonitorMode_t2_notifyAndContinue, 2, 0);
            if((bcsmEvents & T_DISCONNECT_1)==T_DISCONNECT_1)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_tDisconnect, capMonitorMode_t2_notifyAndContinue, 1, 0);
            if((bcsmEvents & T_DISCONNECT_2)==T_DISCONNECT_2)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_tDisconnect, capMonitorMode_t2_interrupted, 2, 0);
            if((bcsmEvents & T_ABANDON_1)==T_ABANDON_1)
                capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.value[count++]=getCapBCSMEvent(capEventTypeBCSM_t2_tAbandon, capMonitorMode_t2_notifyAndContinue, 1, 0);
        }
        capBlock.parameter.requestReportBCSMEventArg_v2.bcsmEvents.count=count;
    }

private:

    /**
     * Returns the capBCSMEvent.
     * @param &eventTypeBCSM const CapEventTypeBCSM_t2
     * @param &monitorMode const CapMonitorMode_t2
     * @param &sendingSideId const unsigned char
     * @param &applicationTimer const unsigned short
     * @return CapBCSMEvent_t2
     */
    CapBCSMEvent_t2 getCapBCSMEvent(const CapEventTypeBCSM_t2 &eventTypeBCSM,
                                    const CapMonitorMode_t2 &monitorMode,
                                    const unsigned char &sendingSideId,
                                    const unsigned short &applicationTimer) {
        CapBCSMEvent_t2 capBCSMEvent;
        capBCSMEvent.bit_mask=CapBCSMEvent_t2_legID_present;
        capBCSMEvent.eventTypeBCSM=eventTypeBCSM;
        capBCSMEvent.monitorMode=monitorMode;
        capBCSMEvent.legID.choice=CapLegID_sendingSideID_chosen;
        capBCSMEvent.legID.u.sendingSideID.length=1;
        capBCSMEvent.legID.u.sendingSideID.value[0]=sendingSideId;
        if(applicationTimer > 0) {
            capBCSMEvent.bit_mask|=CapBCSMEvent_t2_dPSpecificCriteria_present;
            capBCSMEvent.dPSpecificCriteria.choice=CapDPSpecificCriteria_applicationTimer_chosen;
            capBCSMEvent.dPSpecificCriteria.u.applicationTimer=applicationTimer;
        }
        return capBCSMEvent;
    }
};

int RequestReportBCSMEventReqBlock::ROUTE_SELECT_FAILURE_2=0x1;
int RequestReportBCSMEventReqBlock::O_CALLED_PARTY_BUSY_2=0x2;
int RequestReportBCSMEventReqBlock::O_NO_ANSWER_2=0x4;
int RequestReportBCSMEventReqBlock::O_ANSWER_2=0x8;
int RequestReportBCSMEventReqBlock::O_DISCONNECT_1=0x10;
int RequestReportBCSMEventReqBlock::O_DISCONNECT_2=0x20;
int RequestReportBCSMEventReqBlock::O_ABANDON_1=0x40;
int RequestReportBCSMEventReqBlock::O_ALL=0x7f;
int RequestReportBCSMEventReqBlock::T_BUSY_2=0x200;
int RequestReportBCSMEventReqBlock::T_NO_ANSWER_2=0x400;
int RequestReportBCSMEventReqBlock::T_ANSWER_2=0x800;
int RequestReportBCSMEventReqBlock::T_DISCONNECT_1=0x1000;
int RequestReportBCSMEventReqBlock::T_DISCONNECT_2=0x2000;
int RequestReportBCSMEventReqBlock::T_ABANDON_1=0x4000;
int RequestReportBCSMEventReqBlock::T_ALL=0x7f00;

/**
 * The RequestReportGPRSEventReqBlock class.
 */
class RequestReportGPRSEventReqBlock : public Block {

public:

    /* constants for gPRSEvent */
    static char PDP_CONTEXT_ESTABLISHMENT_ACK;
    static char DISCONNECT;
    static char ALL;

    /**
     * Creates a new instance of RequestReportBCSMReqBlock.
     * @param *dialog Dialog
     * @param &invokeId const short
     * @param &gprsEvents const char
     */
    RequestReportGPRSEventReqBlock(Dialog *dialog, const short &invokeId, const char &gprsEvents) : Block(dialog, CapBlock_parameter_present, capReq, CAP_REQUEST_REPORT_GPRS_EVENT, 0, invokeId, -1) {
        capBlock.parameter.requestReportGPRSEventArg_v3.bit_mask=0;
        unsigned short count=0;
        if((gprsEvents & PDP_CONTEXT_ESTABLISHMENT_ACK)==PDP_CONTEXT_ESTABLISHMENT_ACK) {
            capBlock.parameter.requestReportGPRSEventArg_v3.gPRSEvent.value[count].gPRSEventType=capGPRSEventType_pdp_ContextEstablishmentAcknowledgement;
            capBlock.parameter.requestReportGPRSEventArg_v3.gPRSEvent.value[count].monitorMode=capMonitorMode_interrupted;
            count++;
        }
        if((gprsEvents & DISCONNECT)==DISCONNECT) {
            capBlock.parameter.requestReportGPRSEventArg_v3.gPRSEvent.value[count].gPRSEventType=capGPRSEventType_disconnect;
            capBlock.parameter.requestReportGPRSEventArg_v3.gPRSEvent.value[count].monitorMode=capMonitorMode_interrupted;
            count++;
        }
        capBlock.parameter.requestReportGPRSEventArg_v3.gPRSEvent.count=count;
    }
};

char RequestReportGPRSEventReqBlock::PDP_CONTEXT_ESTABLISHMENT_ACK=0x1;
char RequestReportGPRSEventReqBlock::DISCONNECT=0x2;
char RequestReportGPRSEventReqBlock::ALL=0x3;

#endif	/* BLOCK_HPP */
