#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <linux_parser.h>

#include "process.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { 
    return this->pid_; 
}

// Return this process's CPU utilization
float Process::CpuUtilization() { 
    long hertz = sysconf(_SC_CLK_TCK);
    return 100 * (LinuxParser::ActiveJiffies(this->Pid())/hertz) / this->UpTime(); 
}

// Return the command that generated this process
string Process::Command() { 
    return LinuxParser::Command(this->Pid()); 
}

// TODO: Return this process's memory utilization
string Process::Ram() { 
    return LinuxParser::Ram(this->Pid()); 
}

// TODO: Return the user (name) that generated this process
string Process::User() { 
    return LinuxParser::User(this->Pid()); 
}

// Return the age of this process (in seconds)
long int Process::UpTime() { 
    return LinuxParser::UpTime(this->Pid()); 
}

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { 
    if (Pid() < a.Pid()) {
        return true; 
    }
    return false;
}