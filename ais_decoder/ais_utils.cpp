
#include "ais_utils.h"
#include "strutils.h"



namespace
{
    /// returns sorted list of AIS vessel class names
    std::vector<std::string> loadAisVesselShortStrings()
    {
        std::vector<std::string> vecVesselTypes(100, "UNS");
        
        vecVesselTypes[20] = "WIG";//Wing in ground (WIG)";
        vecVesselTypes[21] = "WIG";//"Wing in ground (WIG), Hazardous category A";
        vecVesselTypes[22] = "WIG";//"Wing in ground (WIG), Hazardous category B";
        vecVesselTypes[23] = "WIG";//"Wing in ground (WIG), Hazardous category C";
        vecVesselTypes[24] = "WIG";//"Wing in ground (WIG), Hazardous category D";
        vecVesselTypes[25] = "WIG";//"Wing in ground (WIG), Unknown category";
        vecVesselTypes[26] = "WIG";//"Wing in ground (WIG), Unknown category";
        vecVesselTypes[27] = "WIG";//"Wing in ground (WIG), Unknown category";
        vecVesselTypes[28] = "WIG";//"Wing in ground (WIG), Unknown category";
        vecVesselTypes[29] = "WIG";//"Wing in ground (WIG), Unknown category";
        vecVesselTypes[30] = "FSH";//"Fishing";
        vecVesselTypes[31] = "TUG";//"Towing";
        vecVesselTypes[32] = "TUG";//"Towing: length exceeds 200m or breadth exceeds 25m";
        vecVesselTypes[33] = "OPS";//"Dredging or underwater ops";
        vecVesselTypes[34] = "OPS";//"Diving ops";
        vecVesselTypes[35] = "MIL";//"Military ops";
        vecVesselTypes[36] = "PLS";//"Sailing";
        vecVesselTypes[37] = "PLS";//"Pleasure Craft";
        vecVesselTypes[40] = "HSD";//"High speed craft (HSC)";
        vecVesselTypes[41] = "HSD";//"High speed craft (HSC), Hazardous category A";
        vecVesselTypes[42] = "HSD";//"High speed craft (HSC), Hazardous category B";
        vecVesselTypes[43] = "HSD";//"High speed craft (HSC), Hazardous category C";
        vecVesselTypes[44] = "HSD";//"High speed craft (HSC), Hazardous category D";
        vecVesselTypes[45] = "HSD";//"High speed craft (HSC), Unknown category";
        vecVesselTypes[46] = "HSD";//"High speed craft (HSC), Unknown category";
        vecVesselTypes[47] = "HSD";//"High speed craft (HSC), Unknown category";
        vecVesselTypes[48] = "HSD";//"High speed craft (HSC), Unknown category";
        vecVesselTypes[49] = "HSD";//"High speed craft (HSC), Unknown category";
        vecVesselTypes[50] = "PAS";//"Pilot Vessel";
        vecVesselTypes[51] = "OPS";//"Search and Rescue vessel";
        vecVesselTypes[52] = "TUG";//"Tug";
        vecVesselTypes[53] = "UNS";//"Port Tender";
        vecVesselTypes[54] = "UNS";//"Anti-pollution equipment";
        vecVesselTypes[55] = "OPS";//"Law Enforcement";
        vecVesselTypes[56] = "UNS";//"Local Vessel";
        vecVesselTypes[57] = "UNS";//"Local Vessel";
        vecVesselTypes[58] = "MED";//"Medical Transport";
        vecVesselTypes[59] = "MIL";//"Noncombatant ship";
        vecVesselTypes[60] = "PAS";//"Passenger";
        vecVesselTypes[61] = "PAS";//"Passenger, Hazardous category A";
        vecVesselTypes[62] = "PAS";//"Passenger, Hazardous category B";
        vecVesselTypes[63] = "PAS";//"Passenger, Hazardous category C";
        vecVesselTypes[64] = "PAS";//"Passenger, Hazardous category D";
        vecVesselTypes[65] = "PAS";//"Passenger, Unknown category";
        vecVesselTypes[66] = "PAS";//"Passenger, Unknown category";
        vecVesselTypes[67] = "PAS";//"Passenger, Unknown category";
        vecVesselTypes[68] = "PAS";//"Passenger, Unknown category";
        vecVesselTypes[69] = "PAS";//"Passenger, Unknown category";
        vecVesselTypes[70] = "CAR";//"Cargo";
        vecVesselTypes[71] = "CAR";//"Cargo, Hazardous category A";
        vecVesselTypes[72] = "CAR";//"Cargo, Hazardous category B";
        vecVesselTypes[73] = "CAR";//"Cargo, Hazardous category C";
        vecVesselTypes[74] = "CAR";//"Cargo, Hazardous category D";
        vecVesselTypes[75] = "CAR";//"Cargo, Unknown category";
        vecVesselTypes[76] = "CAR";//"Cargo, Unknown category";
        vecVesselTypes[77] = "CAR";//"Cargo, Unknown category";
        vecVesselTypes[78] = "CAR";//"Cargo, Unknown category";
        vecVesselTypes[79] = "CAR";//"Cargo, Unknown category";
        vecVesselTypes[80] = "TNK";//"Tanker";
        vecVesselTypes[81] = "TNK";//"Tanker, Hazardous category A";
        vecVesselTypes[82] = "TNK";//"Tanker, Hazardous category B";
        vecVesselTypes[83] = "TNK";//"Tanker, Hazardous category C";
        vecVesselTypes[84] = "TNK";//"Tanker, Hazardous category D";
        vecVesselTypes[85] = "TNK";//"Tanker, Unknown category";
        vecVesselTypes[86] = "TNK";//"Tanker, Unknown category";
        vecVesselTypes[87] = "TNK";//"Tanker, Unknown category";
        vecVesselTypes[88] = "TNK";//"Tanker, Unknown category";
        vecVesselTypes[89] = "TNK";//"Tanker, Unknown category";
        vecVesselTypes[90] = "UNS";//"Other Type";
        vecVesselTypes[91] = "UNS";//"Other Type, Hazardous category A";
        vecVesselTypes[92] = "UNS";//"Other Type, Hazardous category B";
        vecVesselTypes[93] = "UNS";//"Other Type, Hazardous category C";
        vecVesselTypes[94] = "UNS";//"Other Type, Hazardous category D";
        vecVesselTypes[95] = "UNS";//"Other Type, Unknown category";
        vecVesselTypes[96] = "UNS";//"Other Type, Unknown category";
        vecVesselTypes[97] = "UNS";//"Other Type, Unknown category";
        vecVesselTypes[98] = "UNS";//"Other Type, Unknown category";
        vecVesselTypes[99] = "UNS";//"Other Type, Unknown category";
        
        return vecVesselTypes;
    }


    /// returns sorted list of AIS vessel class names
    std::vector<std::string> loadAisVesselFullStrings()
    {
        std::vector<std::string> vecVesselTypes(100, "");
        
        vecVesselTypes[20] = "Wing in ground (WIG)";
        vecVesselTypes[21] = "Wing in ground (WIG), Hazardous category A";
        vecVesselTypes[22] = "Wing in ground (WIG), Hazardous category B";
        vecVesselTypes[23] = "Wing in ground (WIG), Hazardous category C";
        vecVesselTypes[24] = "Wing in ground (WIG), Hazardous category D";
        vecVesselTypes[25] = "Wing in ground (WIG), Unknown category";
        vecVesselTypes[26] = "Wing in ground (WIG), Unknown category";
        vecVesselTypes[27] = "Wing in ground (WIG), Unknown category";
        vecVesselTypes[28] = "Wing in ground (WIG), Unknown category";
        vecVesselTypes[29] = "Wing in ground (WIG), Unknown category";
        
        vecVesselTypes[30] = "Fishing";
        vecVesselTypes[31] = "Towing";
        vecVesselTypes[32] = "Towing: length exceeds 200m or breadth exceeds 25m";
        vecVesselTypes[33] = "Dredging or underwater ops";
        vecVesselTypes[34] = "Diving ops";
        vecVesselTypes[35] = "Military ops";
        vecVesselTypes[36] = "Sailing";
        vecVesselTypes[37] = "Pleasure Craft";

        vecVesselTypes[40] = "High speed craft (HSC)";
        vecVesselTypes[41] = "High speed craft (HSC), Hazardous category A";
        vecVesselTypes[42] = "High speed craft (HSC), Hazardous category B";
        vecVesselTypes[43] = "High speed craft (HSC), Hazardous category C";
        vecVesselTypes[44] = "High speed craft (HSC), Hazardous category D";
        vecVesselTypes[45] = "High speed craft (HSC), Unknown category";
        vecVesselTypes[46] = "High speed craft (HSC), Unknown category";
        vecVesselTypes[47] = "High speed craft (HSC), Unknown category";
        vecVesselTypes[48] = "High speed craft (HSC), Unknown category";
        vecVesselTypes[49] = "High speed craft (HSC), Unknown category";
        
        vecVesselTypes[50] = "Pilot Vessel";
        vecVesselTypes[51] = "Search and Rescue vessel";
        vecVesselTypes[52] = "Tug";
        vecVesselTypes[53] = "Port Tender";
        vecVesselTypes[54] = "Anti-pollution equipment";
        vecVesselTypes[55] = "Law Enforcement";
        vecVesselTypes[56] = "Local Vessel";
        vecVesselTypes[57] = "Local Vessel";
        vecVesselTypes[58] = "Medical Transport";
        vecVesselTypes[59] = "Noncombatant ship";
        
        vecVesselTypes[60] = "Passenger";
        vecVesselTypes[61] = "Passenger, Hazardous category A";
        vecVesselTypes[62] = "Passenger, Hazardous category B";
        vecVesselTypes[63] = "Passenger, Hazardous category C";
        vecVesselTypes[64] = "Passenger, Hazardous category D";
        vecVesselTypes[65] = "Passenger, Unknown category";
        vecVesselTypes[66] = "Passenger, Unknown category";
        vecVesselTypes[67] = "Passenger, Unknown category";
        vecVesselTypes[68] = "Passenger, Unknown category";
        vecVesselTypes[69] = "Passenger, Unknown category";
        
        vecVesselTypes[70] = "Cargo";
        vecVesselTypes[71] = "Cargo, Hazardous category A";
        vecVesselTypes[72] = "Cargo, Hazardous category B";
        vecVesselTypes[73] = "Cargo, Hazardous category C";
        vecVesselTypes[74] = "Cargo, Hazardous category D";
        vecVesselTypes[75] = "Cargo, Unknown category";
        vecVesselTypes[76] = "Cargo, Unknown category";
        vecVesselTypes[77] = "Cargo, Unknown category";
        vecVesselTypes[78] = "Cargo, Unknown category";
        vecVesselTypes[79] = "Cargo, Unknown category";
        
        vecVesselTypes[80] = "Tanker";
        vecVesselTypes[81] = "Tanker, Hazardous category A";
        vecVesselTypes[82] = "Tanker, Hazardous category B";
        vecVesselTypes[83] = "Tanker, Hazardous category C";
        vecVesselTypes[84] = "Tanker, Hazardous category D";
        vecVesselTypes[85] = "Tanker, Unknown category";
        vecVesselTypes[86] = "Tanker, Unknown category";
        vecVesselTypes[87] = "Tanker, Unknown category";
        vecVesselTypes[88] = "Tanker, Unknown category";
        vecVesselTypes[89] = "Tanker, Unknown category";
        
        vecVesselTypes[90] = "Other Type";
        vecVesselTypes[91] = "Other Type, Hazardous category A";
        vecVesselTypes[92] = "Other Type, Hazardous category B";
        vecVesselTypes[93] = "Other Type, Hazardous category C";
        vecVesselTypes[94] = "Other Type, Hazardous category D";
        vecVesselTypes[95] = "Other Type, Unknown category";
        vecVesselTypes[96] = "Other Type, Unknown category";
        vecVesselTypes[97] = "Other Type, Unknown category";
        vecVesselTypes[98] = "Other Type, Unknown category";
        vecVesselTypes[99] = "Other Type, Unknown category";
        
        return vecVesselTypes;
    }


    /// returns sorted list of AIS vessel navigation strings
    std::vector<std::string> loadAisNavigationalStatusStrings()
    {
        std::vector<std::string> vecNavigationalStatus(16, "");
        
        vecNavigationalStatus[0] = "Under way using engine";
        vecNavigationalStatus[1] = "At anchor";
        vecNavigationalStatus[2] = "Not under command";
        vecNavigationalStatus[3] = "Restricted maneuverability";
        vecNavigationalStatus[4] = "Constrained by her draught";
        vecNavigationalStatus[5] = "Moored";
        vecNavigationalStatus[6] = "Aground";
        vecNavigationalStatus[7] = "Engaged in Fishing";
        vecNavigationalStatus[8] = "Under way sailing";
        vecNavigationalStatus[9] = "Reserved for future amendment of Navigational Status for HSC";
        vecNavigationalStatus[10] = "Reserved for future amendment of Navigational Status for WIG";
        vecNavigationalStatus[11] = "Reserved for future use";
        vecNavigationalStatus[12] = "Reserved for future use";
        vecNavigationalStatus[13] = "Reserved for future use";
        vecNavigationalStatus[14] = "AIS-SART is active";
        
        return vecNavigationalStatus;
    }
    
    
    /// returns list of country and country abbreviation codes
    std::vector<std::pair<std::string, std::string>> loadCountryCodes()
    {
        std::vector<std::pair<std::string, std::string>> vecCountry(800);
        
        vecCountry[201] = {"Albania","AL"};
        vecCountry[202] = {"Andorra","AD"};
        vecCountry[203] = {"Austria","AT"};
        vecCountry[204] = {"Portugal","PT"};
        vecCountry[205] = {"Belgium","BE"};
        vecCountry[206] = {"Belarus","BY"};
        vecCountry[207] = {"Bulgaria","BG"};
        vecCountry[208] = {"Vatican","VA"};
        vecCountry[209] = {"Cyprus","CY"};
        vecCountry[210] = {"Cyprus","CY"};
        vecCountry[211] = {"Germany","DE"};
        vecCountry[212] = {"Cyprus","CY"};
        vecCountry[213] = {"Georgia","GE"};
        vecCountry[214] = {"Moldova","MD"};
        vecCountry[215] = {"Malta","MT"};
        vecCountry[216] = {"Armenia","ZZ"};
        vecCountry[218] = {"Germany","DE"};
        vecCountry[219] = {"Denmark","DK"};
        vecCountry[220] = {"Denmark","DK"};
        vecCountry[224] = {"Spain","ES"};
        vecCountry[225] = {"Spain","ES"};
        vecCountry[226] = {"France","FR"};
        vecCountry[227] = {"France","FR"};
        vecCountry[228] = {"France","FR"};
        vecCountry[229] = {"Malta","MT"};
        vecCountry[230] = {"Finland","FI"};
        vecCountry[231] = {"Faroe Is","FO"};
        vecCountry[232] = {"United Kingdom","GB"};
        vecCountry[233] = {"United Kingdom","GB"};
        vecCountry[234] = {"United Kingdom","GB"};
        vecCountry[235] = {"United Kingdom","GB"};
        vecCountry[236] = {"Gibraltar","GI"};
        vecCountry[237] = {"Greece","GR"};
        vecCountry[238] = {"Croatia","HR"};
        vecCountry[239] = {"Greece","GR"};
        vecCountry[240] = {"Greece","GR"};
        vecCountry[241] = {"Greece","GR"};
        vecCountry[242] = {"Morocco","MA"};
        vecCountry[243] = {"Hungary","HU"};
        vecCountry[244] = {"Netherlands","NL"};
        vecCountry[245] = {"Netherlands","NL"};
        vecCountry[246] = {"Netherlands","NL"};
        vecCountry[247] = {"Italy","IT"};
        vecCountry[248] = {"Malta","MT"};
        vecCountry[249] = {"Malta","MT"};
        vecCountry[250] = {"Ireland","IE"};
        vecCountry[251] = {"Iceland","IS"};
        vecCountry[252] = {"Liechtenstein","LI"};
        vecCountry[253] = {"Luxembourg","LU"};
        vecCountry[254] = {"Monaco","MC"};
        vecCountry[255] = {"Portugal","PT"};
        vecCountry[256] = {"Malta","MT"};
        vecCountry[257] = {"Norway","NO"};
        vecCountry[258] = {"Norway","NO"};
        vecCountry[259] = {"Norway","NO"};
        vecCountry[261] = {"Poland","PL"};
        vecCountry[262] = {"Montenegro","ME"};
        vecCountry[263] = {"Portugal","PT"};
        vecCountry[264] = {"Romania","RO"};
        vecCountry[265] = {"Sweden","SE"};
        vecCountry[266] = {"Sweden","SE"};
        vecCountry[267] = {"Slovakia","SK"};
        vecCountry[268] = {"San Marino","SM"};
        vecCountry[269] = {"Switzerland","CH"};
        vecCountry[270] = {"Czech Republic","CZ"};
        vecCountry[271] = {"Turkey","TR"};
        vecCountry[272] = {"Ukraine","UA"};
        vecCountry[273] = {"Russia","RU"};
        vecCountry[274] = {"FYR Macedonia","MK"};
        vecCountry[275] = {"Latvia","LV"};
        vecCountry[276] = {"Estonia","EE"};
        vecCountry[277] = {"Lithuania","LT"};
        vecCountry[278] = {"Slovenia","SI"};
        vecCountry[279] = {"Serbia","RS"};
        vecCountry[301] = {"Anguilla","AI"};
        vecCountry[303] = {"USA","US"};
        vecCountry[304] = {"Antigua Barbuda","AG"};
        vecCountry[305] = {"Antigua Barbuda","AG"};
        vecCountry[306] = {"Curacao","CW"};
        vecCountry[307] = {"Aruba","AW"};
        vecCountry[308] = {"Bahamas","BS"};
        vecCountry[309] = {"Bahamas","BS"};
        vecCountry[310] = {"Bermuda","BM"};
        vecCountry[311] = {"Bahamas","BS"};
        vecCountry[312] = {"Belize","BZ"};
        vecCountry[314] = {"Barbados","BB"};
        vecCountry[316] = {"Canada","CA"};
        vecCountry[319] = {"Cayman Is","KY"};
        vecCountry[321] = {"Costa Rica","CR"};
        vecCountry[323] = {"Cuba","CU"};
        vecCountry[325] = {"Dominica","DM"};
        vecCountry[327] = {"Dominican Rep","DO"};
        vecCountry[329] = {"Guadeloupe","GP"};
        vecCountry[330] = {"Grenada","GD"};
        vecCountry[331] = {"Greenland","GL"};
        vecCountry[332] = {"Guatemala","GT"};
        vecCountry[334] = {"Honduras","HN"};
        vecCountry[336] = {"Haiti","HT"};
        vecCountry[338] = {"USA","US"};
        vecCountry[339] = {"Jamaica","JM"};
        vecCountry[341] = {"St Kitts Nevis","KN"};
        vecCountry[343] = {"St Lucia","LC"};
        vecCountry[345] = {"Mexico","MX"};
        vecCountry[347] = {"Martinique","MQ"};
        vecCountry[348] = {"Montserrat","MS"};
        vecCountry[350] = {"Nicaragua","NI"};
        vecCountry[351] = {"Panama","PA"};
        vecCountry[352] = {"Panama","PA"};
        vecCountry[353] = {"Panama","PA"};
        vecCountry[354] = {"Panama","PA"};
        vecCountry[355] = {"Panama","PA"};
        vecCountry[356] = {"Panama","PA"};
        vecCountry[357] = {"Panama","PA"};
        vecCountry[358] = {"Puerto Rico","PR"};
        vecCountry[359] = {"El Salvador","SV"};
        vecCountry[361] = {"St Pierre Miquelon","PM"};
        vecCountry[362] = {"Trinidad Tobago","TT"};
        vecCountry[364] = {"Turks Caicos Is","TC"};
        vecCountry[366] = {"USA","US"};
        vecCountry[367] = {"USA","US"};
        vecCountry[368] = {"USA","US"};
        vecCountry[369] = {"USA","US"};
        vecCountry[370] = {"Panama","PA"};
        vecCountry[371] = {"Panama","PA"};
        vecCountry[372] = {"Panama","PA"};
        vecCountry[373] = {"Panama","PA"};
        vecCountry[374] = {"Panama","PA"};
        vecCountry[375] = {"St Vincent Grenadines","VC"};
        vecCountry[376] = {"St Vincent Grenadines","VC"};
        vecCountry[377] = {"St Vincent Grenadines","VC"};
        vecCountry[378] = {"British Virgin Is","VG"};
        vecCountry[379] = {"US Virgin Is","VI"};
        vecCountry[401] = {"Afghanistan","AF"};
        vecCountry[403] = {"Saudi Arabia","SA"};
        vecCountry[405] = {"Bangladesh","BD"};
        vecCountry[408] = {"Bahrain","BH"};
        vecCountry[410] = {"Bhutan","BT"};
        vecCountry[412] = {"China","CN"};
        vecCountry[413] = {"China","CN"};
        vecCountry[414] = {"China","CN"};
        vecCountry[416] = {"Taiwan","TW"};
        vecCountry[417] = {"Sri Lanka","LK"};
        vecCountry[419] = {"India","IN"};
        vecCountry[422] = {"Iran","IR"};
        vecCountry[423] = {"Azerbaijan","AZ"};
        vecCountry[425] = {"Iraq","IQ"};
        vecCountry[428] = {"Israel","IL"};
        vecCountry[431] = {"Japan","JP"};
        vecCountry[432] = {"Japan","JP"};
        vecCountry[434] = {"Turkmenistan","TM"};
        vecCountry[436] = {"Kazakhstan","KZ"};
        vecCountry[437] = {"Uzbekistan","UZ"};
        vecCountry[438] = {"Jordan","JO"};
        vecCountry[440] = {"Korea","KR"};
        vecCountry[441] = {"Korea","KR"};
        vecCountry[443] = {"Palestine","PS"};
        vecCountry[445] = {"DPR Korea","KP"};
        vecCountry[447] = {"Kuwait","KW"};
        vecCountry[450] = {"Lebanon","LB"};
        vecCountry[451] = {"Kyrgyz Republic","ZZ"};
        vecCountry[453] = {"Macao","ZZ"};
        vecCountry[455] = {"Maldives","MV"};
        vecCountry[457] = {"Mongolia","MN"};
        vecCountry[459] = {"Nepal","NP"};
        vecCountry[461] = {"Oman","OM"};
        vecCountry[463] = {"Pakistan","PK"};
        vecCountry[466] = {"Qatar","QA"};
        vecCountry[468] = {"Syria","SY"};
        vecCountry[470] = {"UAE","AE"};
        vecCountry[472] = {"Tajikistan","TJ"};
        vecCountry[473] = {"Yemen","YE"};
        vecCountry[475] = {"Yemen","YE"};
        vecCountry[477] = {"Hong Kong","HK"};
        vecCountry[478] = {"Bosnia and Herzegovina","BA"};
        vecCountry[501] = {"Antarctica","AQ"};
        vecCountry[503] = {"Australia","AU"};
        vecCountry[506] = {"Myanmar","MM"};
        vecCountry[508] = {"Brunei","BN"};
        vecCountry[510] = {"Micronesia","FM"};
        vecCountry[511] = {"Palau","PW"};
        vecCountry[512] = {"New Zealand","NZ"};
        vecCountry[514] = {"Cambodia","KH"};
        vecCountry[515] = {"Cambodia","KH"};
        vecCountry[516] = {"Christmas Is","CX"};
        vecCountry[518] = {"Cook Is","CK"};
        vecCountry[520] = {"Fiji","FJ"};
        vecCountry[523] = {"Cocos Is","CC"};
        vecCountry[525] = {"Indonesia","ID"};
        vecCountry[529] = {"Kiribati","KI"};
        vecCountry[531] = {"Laos","LA"};
        vecCountry[533] = {"Malaysia","MY"};
        vecCountry[536] = {"N Mariana Is","MP"};
        vecCountry[538] = {"Marshall Is","MH"};
        vecCountry[540] = {"New Caledonia","NC"};
        vecCountry[542] = {"Niue","NU"};
        vecCountry[544] = {"Nauru","NR"};
        vecCountry[546] = {"French Polynesia","TF"};
        vecCountry[548] = {"Philippines","PH"};
        vecCountry[553] = {"Papua New Guinea","PG"};
        vecCountry[555] = {"Pitcairn Is","PN"};
        vecCountry[557] = {"Solomon Is","SB"};
        vecCountry[559] = {"American Samoa","AS"};
        vecCountry[561] = {"Samoa","WS"};
        vecCountry[563] = {"Singapore","SG"};
        vecCountry[564] = {"Singapore","SG"};
        vecCountry[565] = {"Singapore","SG"};
        vecCountry[566] = {"Singapore","SG"};
        vecCountry[567] = {"Thailand","TH"};
        vecCountry[570] = {"Tonga","TO"};
        vecCountry[572] = {"Tuvalu","TV"};
        vecCountry[574] = {"Vietnam","VN"};
        vecCountry[576] = {"Vanuatu","VU"};
        vecCountry[577] = {"Vanuatu","VU"};
        vecCountry[578] = {"Wallis Futuna Is","WF"};
        vecCountry[601] = {"South Africa","ZA"};
        vecCountry[603] = {"Angola","AO"};
        vecCountry[605] = {"Algeria","DZ"};
        vecCountry[607] = {"St Paul Amsterdam Is","XX"};
        vecCountry[608] = {"Ascension Is","IO"};
        vecCountry[609] = {"Burundi","BI"};
        vecCountry[610] = {"Benin","BJ"};
        vecCountry[611] = {"Botswana","BW"};
        vecCountry[612] = {"Cen Afr Rep","CF"};
        vecCountry[613] = {"Cameroon","CM"};
        vecCountry[615] = {"Congo","CG"};
        vecCountry[616] = {"Comoros","KM"};
        vecCountry[617] = {"Cape Verde","CV"};
        vecCountry[618] = {"Antarctica","AQ"};
        vecCountry[619] = {"Ivory Coast","CI"};
        vecCountry[620] = {"Comoros","KM"};
        vecCountry[621] = {"Djibouti","DJ"};
        vecCountry[622] = {"Egypt","EG"};
        vecCountry[624] = {"Ethiopia","ET"};
        vecCountry[625] = {"Eritrea","ER"};
        vecCountry[626] = {"Gabon","GA"};
        vecCountry[627] = {"Ghana","GH"};
        vecCountry[629] = {"Gambia","GM"};
        vecCountry[630] = {"Guinea-Bissau","GW"};
        vecCountry[631] = {"Equ. Guinea","GQ"};
        vecCountry[632] = {"Guinea","GN"};
        vecCountry[633] = {"Burkina Faso","BF"};
        vecCountry[634] = {"Kenya","KE"};
        vecCountry[635] = {"Antarctica","AQ"};
        vecCountry[636] = {"Liberia","LR"};
        vecCountry[637] = {"Liberia","LR"};
        vecCountry[642] = {"Libya","LY"};
        vecCountry[644] = {"Lesotho","LS"};
        vecCountry[645] = {"Mauritius","MU"};
        vecCountry[647] = {"Madagascar","MG"};
        vecCountry[649] = {"Mali","ML"};
        vecCountry[650] = {"Mozambique","MZ"};
        vecCountry[654] = {"Mauritania","MR"};
        vecCountry[655] = {"Malawi","MW"};
        vecCountry[656] = {"Niger","NE"};
        vecCountry[657] = {"Nigeria","NG"};
        vecCountry[659] = {"Namibia","NA"};
        vecCountry[660] = {"Reunion","RE"};
        vecCountry[661] = {"Rwanda","RW"};
        vecCountry[662] = {"Sudan","SD"};
        vecCountry[663] = {"Senegal","SN"};
        vecCountry[664] = {"Seychelles","SC"};
        vecCountry[665] = {"St Helena","SH"};
        vecCountry[666] = {"Somalia","SO"};
        vecCountry[667] = {"Sierra Leone","SL"};
        vecCountry[668] = {"Sao Tome Principe","ST"};
        vecCountry[669] = {"Swaziland","SZ"};
        vecCountry[670] = {"Chad","TD"};
        vecCountry[671] = {"Togo","TG"};
        vecCountry[672] = {"Tunisia","TN"};
        vecCountry[674] = {"Tanzania","TZ"};
        vecCountry[675] = {"Uganda","UG"};
        vecCountry[676] = {"DR Congo","CD"};
        vecCountry[677] = {"Tanzania","TZ"};
        vecCountry[678] = {"Zambia","ZM"};
        vecCountry[679] = {"Zimbabwe","ZW"};
        vecCountry[701] = {"Argentina","AR"};
        vecCountry[710] = {"Brazil","BR"};
        vecCountry[720] = {"Bolivia","BO"};
        vecCountry[725] = {"Chile","CL"};
        vecCountry[730] = {"Colombia","CO"};
        vecCountry[735] = {"Ecuador","EC"};
        vecCountry[740] = {"UK","UK"};
        vecCountry[745] = {"Guiana","GF"};
        vecCountry[750] = {"Guyana","GY"};
        vecCountry[755] = {"Paraguay","PY"};
        vecCountry[760] = {"Peru","PE"};
        vecCountry[765] = {"Suriname","SR"};
        vecCountry[770] = {"Uruguay","UY"};
        vecCountry[775] = {"Venezuela","VE"};
        
        return vecCountry;
    }
    
    
    // nav-aid types for message type 21
    std::vector<std::string> loadNavAidTypes()
    {
        std::vector<std::string> types = {
                "",
                "Reference",
                "RACON",
                "Fixed Structure",
                "",
                "Light",
                "Light",
                "Leading Light Front",
                "Leading Light Rear",
                "Beacon, Cardinal N",
                "Beacon, Cardinal E",
                "Beacon, Cardinal S",
                "Beacon, Cardinal W",
                "Beacon, Port hand",
                "Beacon, Starboard hand",
                "Beacon, Preferred Channel port hand",
                "Beacon, Preferred Channel starboard hand",
                "Beacon, Isolated danger",
                "Beacon, Safe water",
                "Beacon, Special mark",
                "Cardinal Mark N",
                "Cardinal Mark E",
                "Cardinal Mark S",
                "Cardinal Mark W",
                "Port hand Mark",
                "Starboard hand Mark",
                "Preferred Channel Port hand",
                "Preferred Channel Starboard hand",
                "Isolated danger",
                "Safe Water",
                "Special Mark",
                "Rig"};
        
        return types;
    };
};


/* returns abbreviated class name for the given code */
const std::string &AIS::getAisVesselClassShort(int _iRawAisCode)
{
    static std::vector<std::string> typeList = loadAisVesselShortStrings();
    if ( (_iRawAisCode < 0) ||
         (_iRawAisCode >= (int)typeList.size()) )
    {
        return typeList[90];
    }
    else
    {
        return typeList[(size_t)_iRawAisCode];
    }
}

/* returns full class name for the given code */
const std::string &AIS::getAisVesselClassFull(int _iRawAisCode)
{
    static std::vector<std::string> typeList = loadAisVesselFullStrings();
    if ( (_iRawAisCode < 0) ||
         (_iRawAisCode >= (int)typeList.size()) )
    {
        return typeList[90];
    }
    else
    {
        return typeList[(size_t)_iRawAisCode];
    }
}


/* does a reverse lookup on vessel class */
int AIS::getAisVesselClassCode(const std::string &_strClass)
{
    if (_strClass.empty() == false)
    {
        // try to match to full class names
        {
            static std::vector<std::string> typeList = loadAisVesselFullStrings();
            for (int i = 0; i < (int)typeList.size(); i++)
            {
                const std::string &str = typeList[i];
                if (ascii_stricmp(str, _strClass) == 0)
                {
                    return i;
                }
            }
        }
        
        // try to match to abbreviated class names
        {
            static std::vector<std::string> typeList = loadAisVesselShortStrings();
            for (int i = 0; i < (int)typeList.size(); i++)
            {
                const std::string &str = typeList[i];
                if (ascii_stricmp(str, _strClass) == 0)
                {
                    return i;
                }
            }
        }
    }
    
    return 90;
}


/* returns navigation status string for the given code */
const std::string &AIS::getAisNavigationStatus(int _iRawAisCode)
{
    static std::vector<std::string> typeList = loadAisNavigationalStatusStrings();
    if ( (_iRawAisCode < 0) ||
         (_iRawAisCode >= (int)typeList.size()) )
    {
        return typeList[0];
    }
    else
    {
        return typeList[(size_t)_iRawAisCode];
    }
}


/* does a lookup of the country/flag details from MMSI */
const std::pair<std::string, std::string> &AIS::getAisCountryCodes(const std::string &_strMmsi)
{
    static std::vector<std::pair<std::string, std::string>> countryList = loadCountryCodes();
    std::string strFullMmsi = AIS::mmsi_to_string(_strMmsi);
    size_t uMid = 0;
    
    if ( (strFullMmsi.compare(0, 1, "0") == 0) ||
         (strFullMmsi.compare(0, 1, "8") == 0) )
    {
        uMid = std::strtoul(strFullMmsi.substr(1, 3).c_str(), nullptr, 10);
    }
    else if ( (strFullMmsi.compare(0, 2, "00") == 0) ||
              (strFullMmsi.compare(0, 2, "98") == 0) ||
              (strFullMmsi.compare(0, 2, "99") == 0) )
    {
        uMid = std::strtoul(strFullMmsi.substr(2, 3).c_str(), nullptr, 10);
    }
    else if ( (strFullMmsi.compare(0, 3, "111") == 0) ||
              (strFullMmsi.compare(0, 3, "970") == 0) ||
              (strFullMmsi.compare(0, 3, "972") == 0) ||
              (strFullMmsi.compare(0, 3, "974") == 0) )
    {
        uMid = std::strtoul(strFullMmsi.substr(3, 3).c_str(), nullptr, 10);
    }
    else
    {
        uMid = std::strtoul(strFullMmsi.substr(0, 3).c_str(), nullptr, 10);
    }
    
    if (uMid >= countryList.size())
    {
        uMid = 0;
    }
    
    return countryList[uMid];
}


/* formats name and removes illegal characters */
std::string AIS::getCleanName(const std::string &_str)
{
    std::string ret(_str);
    
    if (ret.empty() == false)
    {
        // strip all chars after (and including) '@'
        stripTrailingAll(ret, '@');
        
        // strip trailing whitespace
        stripTrailingWhitespace(ret);
    }
    
    return ret;
}


/* make sure mmsi has 9 digits */
std::string AIS::mmsi_to_string(long _iMmsi)
{
    return mmsi_to_string(std::to_string(_iMmsi));
}

/* make sure mmsi has 9 digits */
std::string AIS::mmsi_to_string(const std::string &_strMmsi)
{
    return std::string(std::max(0, 9 - (int)_strMmsi.length()), '0') + _strMmsi;
}

/* returns nav aid type string */
const std::string &AIS::getAisNavAidType(int _iNavAidType)
{
    static std::vector<std::string> typeList = loadNavAidTypes();
    if ( (_iNavAidType < 0) ||
         (_iNavAidType >= (int)typeList.size()) )
    {
        return typeList[0];
    }
    else
    {
        return typeList[(size_t)_iNavAidType];
    }
}




