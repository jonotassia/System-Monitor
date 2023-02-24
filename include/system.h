#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <linux_parser.h>

#include "process.h"
#include "processor.h"

class System {
 public:
  Processor& Cpu();                   
  std::vector<Process>& Processes();  
  float MemoryUtilization();          
  long UpTime();                      
  int TotalProcesses();               
  int RunningProcesses();             
  std::string Kernel();               
  std::string OperatingSystem();      

  // Define any necessary private members
 private:
  vector<Processor> cpu_ = {};
  vector<Process> processes_ = {};
};

#endif