#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <linux_parser.h>

#include "process.h"
#include "processor.h"

class System {
 public:
  std::vector<Processor>& Cpu();                   
  std::vector<Process>& Processes();  
  float MemoryUtilization();
  long TotalMemoryUsage();
  long NonCacheBufferMem();
  long BufferMem();
  long CachedMem();
  long SwapMem();          
  long UpTime();                      
  int TotalProcesses();               
  int RunningProcesses();             
  std::string Kernel();               
  std::string OperatingSystem();      

  // Define any necessary private members
 private:
  std::vector<Processor> cpu_ = {};
  std::vector<Process> processes_ = {};
};

#endif