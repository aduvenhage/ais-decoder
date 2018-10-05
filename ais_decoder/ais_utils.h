
#ifndef AIS_UTILS_H
#define AIS_UTILS_H


#include <vector>
#include <string>


namespace AIS
{
    /// returns full class name for the given code
    const std::string &getAisVesselClassFull(int _iRawAisCode);
    
    /// returns abbreviated class name for the given code
    const std::string &getAisVesselClassShort(int _iRawAisCode);
    
    /// does a reverse lookup on vessel class
    int getAisVesselClassCode(const std::string &_strClass);

    /// returns navigation status string for the given code
    const std::string &getAisNavigationStatus(int _iRawAisCode);

    /// does a lookup of the country/flag details from MMSI
    const std::pair<std::string, std::string> &getAisCountryCodes(const std::string &_strMmsi);
    
    /// formats name and removes illegal characters
    std::string getCleanName(const std::string &_str);

    /// make sure mmsi has 9 digits
    std::string mmsi_to_string(long _iMmsi);

    /// make sure mmsi has 9 digits
    std::string mmsi_to_string(const std::string &_strMmsi);
    
    /// returns nav aid type string
    const std::string &getAisNavAidType(int _iNavAidType);
    
};  // namespace AIS



#endif  // #ifndef AIS_UTILS_H
