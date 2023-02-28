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

// Constructor for generating a new Process from a pid
Process::Process(int pid) : pid_(pid) {}

// Return this process's ID
int Process::Pid() const { 
    return this->pid_; 
}

// Return this process's CPU utilization
float Process::CpuUtilization() { 
    float hertz = sysconf(_SC_CLK_TCK);
    float cpu_util1 = (LinuxParser::ActiveJiffies(this->Pid())/hertz) / this->UpTime();
    float cpu_util2 = (LinuxParser::ActiveJiffies(this->Pid())/hertz) / this->UpTime();
    return cpu_util2 - cpu_util1; 
}

// Return the command that generated this process
string Process::Command() { 
    return LinuxParser::Command(this->Pid()); 
}

// TODO: Return this process's memory utilization
string Process::Ram() { 
    long ram1 = LinuxParser::Ram(this->Pid());
    long ram2 = LinuxParser::Ram(this->Pid());
    return to_string(ram2 - ram1); 
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