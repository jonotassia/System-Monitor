#include <string>
#include <ctime>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
    // Get current time then subtract seconds to get delta
    long current_time = std::time(0);
    long delta = current_time - seconds;
    
    // Divide until you get hours, minutes, and seconds
    auto result = div(delta, (long)3600);
    string hours_elapsed = std::to_string(result.quot);

    result = div(result.rem, (long)60);
    string minutes_elapsed = std::to_string(result.quot);
    string seconds_elapsed = std::to_string(result.rem);

    return hours_elapsed + ":" + minutes_elapsed + ":" + seconds_elapsed; 
}