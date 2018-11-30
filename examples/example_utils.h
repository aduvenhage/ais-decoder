#ifndef AIS_EXAMPLE_UTILS_H
#define AIS_EXAMPLE_UTILS_H


#include <chrono>

namespace  UTILS
{
    namespace CLOCK
    {
        using clock_type = std::chrono::high_resolution_clock;
        
        double getClockResolution()
        {
            return (double)clock_type::period::num / (double)clock_type::period::den;
        }
        
        double getClockDurationS(const clock_type::duration &_td)
        {
            return _td.count() * getClockResolution();
        }
        
        auto getClockNow()
        {
            return clock_type::now();
        }
    };
    
};  // namespace UTILS


#endif // #ifndef AIS_EXAMPLE_UTILS_H
