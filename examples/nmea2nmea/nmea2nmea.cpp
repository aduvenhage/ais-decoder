
#include "../../ais_decoder/ais_decoder.h"
#include "../../ais_decoder/ais_file.h"
#include "../../ais_decoder/ais_utils.h"
#include "../../ais_decoder/strutils.h"
#include "../../ais_decoder/default_sentence_parser.h"
#include "../example_utils.h"

#include <string>
#include <vector>
#include <stdio.h>
#include <map>
#include <set>


/*
 
 
 
 This is just a very simple NMEA to NMEA file writing example.  NMEA strings are decoded, filtered and then writen to a new file.
 
 
 Current limitations:
 - the VesselDB container is a standard map and set combination that could be improved on
 - the filtering method 'allowMessage(...)' and this class only allows for filtering of single values
   (typically you would like to check for sets of values)

 
 
 */




/// output file with buffering
class BufferedFileOut
{
 public:
    BufferedFileOut(const std::string &_strPath, size_t _uBlockSize)
        :m_pFileOut(nullptr),
         m_buffer(_uBlockSize),
         m_uBlockSize(_uBlockSize)
    {
        m_pFileOut = fopen(_strPath.c_str(), "wb");
    }
    
    ~BufferedFileOut()
    {
        flush();
        fclose(m_pFileOut);
    }
    
    void append(const AIS::StringRef &_str)
    {
        m_buffer.append(_str.data(), _str.size());
        
        if (m_buffer.size() >= m_uBlockSize)
        {
            flush();
        }
    }
    
    void flush()
    {
        if (m_pFileOut != nullptr)
        {
            fwrite(m_buffer.data(), 1, m_buffer.size(), m_pFileOut);
            m_buffer.clear();
        }
    }
    
 private:
    FILE            *m_pFileOut;
    AIS::Buffer     m_buffer;
    size_t          m_uBlockSize;
};



/**
 Vessel info lookup (DB indexed by MMSI).
 
 TODO: look at a more efficient way (faster lookup) to store vessel info.
 
 */
struct VesselDb
{
    void updateVesselType(unsigned int _uMmsi, unsigned int _uType)
    {
        m_mapMmsi2Type[_uMmsi].insert(_uType);
    }
    
    bool checkVesselType(unsigned int _uMmsi, unsigned int _uType)
    {
        return m_mapMmsi2Type[_uMmsi].count(_uType) > 0;
    }
    
    std::map<unsigned int, std::set<unsigned int>>       m_mapMmsi2Type;
};


/**
 
 This decoder looks at type 5 and indexes some vessel properties on MMSI.
 Use it to build up a vessel DB from some data.
 
 NOTE: 'enableMsgTypes({5, 19, 24})' is called in decoder constructor to limit decoding to types 5, 19 & 24 only
 
 */
class AisType5Db : public AIS::AisDecoder
{
public:
    AisType5Db(VesselDb &_db, const AIS::SentenceParser &_parser)
        :m_parser(_parser),
         m_db(_db)
    {
        // set decoder to decode only message types that provide vessel type code
        enableMsgTypes({5, 19, 24});
    }
    
protected:
    virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int /*_iRot*/, unsigned int _uSog, bool /*_bPosAccuracy*/, int _iPosLon, int _iPosLat, int _iCog, int /*_iHeading*/) override {}
    
    virtual void onType411(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uYear, unsigned int _uMonth, unsigned int _uDay, unsigned int _uHour, unsigned int _uMinute, unsigned int _uSecond,
                           bool _bPosAccuracy, int _iPosLon, int _iPosLat) override {}
    
    virtual void onType5(unsigned int _uMmsi, unsigned int _uImo, const std::string &_strCallsign, const std::string &_strName,
                         unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard, unsigned int _uFixType,
                         unsigned int _uEtaMonth, unsigned int _uEtaDay, unsigned int _uEtaHour, unsigned int _uEtaMinute, unsigned int _uDraught,
                         const std::string &_strDestination)  override
    {
        m_db.updateVesselType(_uMmsi, _uType);
    }
    
    virtual void onType9(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, unsigned int _iAltitude) override {}
    
    virtual void onType18(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override {}
    
    virtual void onType19(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading,
                          const std::string &_strName, unsigned int _uType,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override
    {
        m_db.updateVesselType(_uMmsi, _uType);
    }

    virtual void onType21(unsigned int _uMmsi, unsigned int _uAidType, const std::string &_strName, bool _bPosAccuracy, int _iPosLon, int _iPosLat,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {}
    
    virtual void onType24A(unsigned int _uMmsi, const std::string &_strName) override {}
    
    virtual void onType24B(unsigned int _uMmsi, const std::string &_strCallsign, unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override
    {
        m_db.updateVesselType(_uMmsi, _uType);
    }

    virtual void onType27(unsigned int _uMmsi, unsigned int _uNavstatus, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog) override {}
    
    /// called on every sentence (raw data) received (includes all characters, including NL, CR, etc.; called before any validation or CRCs checks are performed)
    virtual void onSentence(const AIS::StringRef &_strSentence) override {}
    
    /// called on every full message, before 'onTypeXX(...)'
    virtual void onMessage(const AIS::StringRef &_strPayload, const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) override {}
    
    /// called when message type is not supported (i.e. 'onTypeXX(...)' not implemented)
    virtual void onNotDecoded(const AIS::StringRef &_strPayload, int _iMsgType) override {}
    
    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {}
    
private:
    const AIS::SentenceParser   &m_parser;          ///< sentence parser being used
    VesselDb                    &m_db;
};



/**
 
 Picks out specific AIS messages and writes original sentences to a new file
 
 NOTE: this is just a very simple example.
 - the VesselDB container is a standard map and set combination that could be improved on
 - the filtering method 'allowMessage(...)' and this class only allows for filtering of single values
   (typically you would like to check for sets of values)
 
 NOTE: NMEA is output if any of the rules fire.
 
 */
class AisNmeaFilter : public AIS::AisDecoder
{
 public:
    AisNmeaFilter(const std::string &_strOutputPath, const AIS::SentenceParser &_parser, VesselDb &_db,
                  size_t _uFileOutChunkSize,
                  unsigned int _uTargetMsgType,
                  unsigned int _uTargetCountryCode,
                  unsigned int _uType)
        :m_fout(_strOutputPath, _uFileOutChunkSize),
         m_parser(_parser),
         m_db(_db),
         m_uTargetMsgType(_uTargetMsgType),
         m_uTargetCountryCode(_uTargetCountryCode),
         m_uTargetType(_uType)
    {
        if (_uTargetMsgType > 0)
        {
            // set decoder to decode only specified type
            enableMsgTypes({(int)_uTargetMsgType});
        }
    }
    
 protected:
    bool allowMessage(unsigned int _uMsgType, unsigned int _uMmsi)
    {
        // NOTE: the message is allowed if any of the rules fire (i.e. OR)

        // allow message if type equals target type
        if  (_uMsgType == m_uTargetMsgType)
        {
            return true;
        }
        
        // or, allow message if MDI equals target country code
        else if ( (m_uTargetCountryCode > 0) &&
                  (m_uTargetCountryCode == AIS::mmsi_to_mdi(_uMmsi)) )
        {
            return true;
        }
        
        // or, allow message if vessel type matches target vessel type
        else if ( (m_uTargetType > 0) &&
                  (m_db.checkVesselType(_uMmsi, m_uTargetType) == true) )
        {
            return true;
        }
        
        else
        {
            return false;
        }
    }
    
    virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int /*_iRot*/, unsigned int _uSog, bool /*_bPosAccuracy*/, int _iPosLon, int _iPosLat, int _iCog, int /*_iHeading*/) override
    {
        if (allowMessage(_uMsgType, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }
    
    virtual void onType411(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uYear, unsigned int _uMonth, unsigned int _uDay, unsigned int _uHour, unsigned int _uMinute, unsigned int _uSecond,
                           bool _bPosAccuracy, int _iPosLon, int _iPosLat) override
    {
        if (allowMessage(_uMsgType, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }
    
    virtual void onType5(unsigned int _uMmsi, unsigned int _uImo, const std::string &_strCallsign, const std::string &_strName,
                         unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard, unsigned int _uFixType,
                         unsigned int _uEtaMonth, unsigned int _uEtaDay, unsigned int _uEtaHour, unsigned int _uEtaMinute, unsigned int _uDraught,
                         const std::string &_strDestination)  override
    {
        if (allowMessage(5, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }

    virtual void onType9(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, unsigned int _iAltitude) override
    {
        if (allowMessage(9, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }

    virtual void onType18(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override
    {
        if (allowMessage(18, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }

    virtual void onType19(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading,
                          const std::string &_strName, unsigned int _uType,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override
    {
        if (allowMessage(19, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }

    virtual void onType21(unsigned int _uMmsi, unsigned int _uAidType, const std::string &_strName, bool _bPosAccuracy, int _iPosLon, int _iPosLat,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override
    {
        if (allowMessage(21, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }

    virtual void onType24A(unsigned int _uMmsi, const std::string &_strName) override
    {
        if (allowMessage(24, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }

    virtual void onType24B(unsigned int _uMmsi, const std::string &_strCallsign, unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override
    {
        if (allowMessage(24, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }

    virtual void onType27(unsigned int _uMmsi, unsigned int _uNavstatus, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog) override
    {
        if (allowMessage(27, _uMmsi) == true)
        {
            for (const auto &str : sentences())
            {
                m_fout.append(str);
            }
        }
    }

    /// called on every sentence (raw data) received (includes all characters, including NL, CR, etc.; called before any validation or CRCs checks are performed)
    virtual void onSentence(const AIS::StringRef &_strSentence) override {}
    
    /// called on every full message, before 'onTypeXX(...)'
    virtual void onMessage(const AIS::StringRef &_strPayload, const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) override {}
    
    /// called when message type is not supported (i.e. 'onTypeXX(...)' not implemented)
    virtual void onNotDecoded(const AIS::StringRef &_strPayload, int _iMsgType) override {}
    
    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {
        std::string msg = _strMessage;
        printf("%s [%s]\n", _strError.c_str(), msg.c_str());
    }
    
 private:
    BufferedFileOut             m_fout;             ///< NMEA output file
    const AIS::SentenceParser   &m_parser;          ///< sentence parser being used
    VesselDb                    &m_db;
    unsigned int                m_uTargetMsgType;
    unsigned int                m_uTargetCountryCode;
    unsigned int                m_uTargetType;
};


/// decoder callback that just prints progress/stats to console
void progressCb(size_t _uTotalBytes, const AIS::AisDecoder &_decoder)
{
    printf("bytes = %lu, messages = %lu, errors = %lu\n", (unsigned long)_uTotalBytes, (unsigned long)_decoder.getTotalMessageCount(), (unsigned long)_decoder.getDecodingErrorCount());
}


/**
 Run decoder through one file.
 'AisNmeaFilter::allowMessage(...)' determines how messages are filtered.
 
 */
void createFilteredFile(const std::string &_strLogPath, const std::string &_strOutputPath, VesselDb &_db,
                        unsigned int _uTargetMsgType,
                        unsigned int _uTargetCountryCode,
                        unsigned int _uType)
{
    // NOTE: EXAMPLE_DATA_PATH is defined by cmake script to be absolute path to source/data folder
    auto strInputFilePath = std::string(EXAMPLE_DATA_PATH) + "/" + _strLogPath;
    const size_t BLOCK_SIZE = 1024 * 1024 * 32;
    auto tsInit = UTILS::CLOCK::getClockNow();

    // create decoder instance
    AIS::DefaultSentenceParser parser;
    AisNmeaFilter decoder(std::string(EXAMPLE_DATA_PATH) + "/" + _strOutputPath, parser, _db,
                          BLOCK_SIZE,
                          _uTargetMsgType,
                          _uTargetCountryCode,
                          _uType);
    
    AIS::processAisFile(strInputFilePath, decoder, parser, BLOCK_SIZE, progressCb);
    
    auto td = UTILS::CLOCK::getClockNow() - tsInit;
    double dTd = UTILS::CLOCK::getClockDurationS(td);
    
    printf("rate = %.2f, count = %lu, time = %.2f\n\n\n\n", (float)(decoder.getTotalMessageCount() / dTd), (unsigned long)decoder.getTotalMessageCount(), dTd);
}


/**
 Run through file to build up vessel DB.
 */
void buildVesselDb(VesselDb &_db, const std::string &_strLogPath)
{
    // NOTE: EXAMPLE_DATA_PATH is defined by cmake script to be absolute path to source/data folder
    auto strInputFilePath = std::string(EXAMPLE_DATA_PATH) + "/" + _strLogPath;
    const size_t BLOCK_SIZE = 1024 * 1024 * 32;
    auto tsInit = UTILS::CLOCK::getClockNow();
    
    // create decoder instance
    AIS::DefaultSentenceParser parser;
    AisType5Db decoder(_db, parser);
    
    AIS::processAisFile(strInputFilePath, decoder, parser, BLOCK_SIZE, progressCb);
    
    auto td = UTILS::CLOCK::getClockNow() - tsInit;
    double dTd = UTILS::CLOCK::getClockDurationS(td);
    
    printf("rate = %.2f, count = %lu, time = %.2f\n\n\n\n", (float)(decoder.getTotalMessageCount() / dTd), (unsigned long)decoder.getTotalMessageCount(), dTd);
}


/**
 
 */
int main()
{
    VesselDb db;
    printf("Building vessel DB... \n");
    buildVesselDb(db, "5.txt");
    buildVesselDb(db, "19.txt");
    buildVesselDb(db, "24.txt");

    // NOTE: NMEA is output if any of the rules fire.
    printf("Creating filtered file... \n");
    createFilteredFile("123.txt", "filtered_nmea.txt", db,
                       0,       // msg type: 0 = ignore
                       0,       // country MDI: 0 = ignore
                       70);     // class code: 0 = ignore
    
    return 0;
}
