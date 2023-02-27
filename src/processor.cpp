#include <linux_parser.h>

#include "processor.h"

Processor::Processor(int cpu_number) : cpu_number_(cpu_number) {};

// Return the aggregate CPU utilization
float Processor::Utilization() { 
    return LinuxParser::CpuUtilization(this->cpu_number_); 
}

int Processor::CpuNumber() {
    return this->cpu_number_;
}