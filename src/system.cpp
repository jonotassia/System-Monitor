#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <iterator>

#include "process.h"
#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Sets and return the system's CPU
vector<Processor>& System::Cpu() { 
    if (cpu_.size() == 0) {
        int num_processors = LinuxParser::NumProcessors();

        // Initialize and pushback processors
        for (int i = 0; i < num_processors; i++) {
            cpu_.push_back(Processor(i));
        }
    }
    return cpu_; 
}

// Return a container composed of the system's processes
vector<Process>& System::Processes() { 
    if (processes_.size() == 0) {
        for (int pid : LinuxParser::Pids()) {
            processes_.push_back(Process(pid));
        }
    }
    return processes_; 
}

// Return the system's kernel identifier (string)
std::string System::Kernel() { 
    return LinuxParser::Kernel(); 
}

// Return the system's memory utilization
float System::MemoryUtilization() { 
    return LinuxParser::MemoryUtilization(); 
}

// Return the operating system name
std::string System::OperatingSystem() { 
    return LinuxParser::OperatingSystem(); 
}

// Return the number of processes actively running on the system
int System::RunningProcesses() { 
    return LinuxParser::RunningProcesses(); 
}

// Return the total number of processes on the system
int System::TotalProcesses() { 
    return LinuxParser::TotalProcesses(); 
}

// Return the number of seconds since the system started running
long int System::UpTime() { 
    return LinuxParser::UpTime(); 
}

// Make new functions for other memory utilization