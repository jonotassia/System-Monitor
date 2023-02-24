#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iterator>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::unordered_map;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return a vector of processor
vector<Processor> LinuxParser::Processors() {
  vector<Processors> processors;
  int proc_count = -1;
  string line, token;

  std::ifstream stream(kProcDirectory + kStatFilename) {
    if (stream.is_open()) {
      while (std::getline(stream, line)) {
        // Get the each token
        std::istringstream linestream(line);
        linestream >> token;

        // Increment proc_count by 1 for each loop, starting at -1 in order to avoid pulling in the aggregate cpu
        proc_count++;

        // Break once the CPU section has been passed
        if (token == "intr") {
          break;
        }
      }
    }
  }
  // Initialize and pushback processors
  for (int i = 1; i <= proc_count; i++) {
    processors.push_back(Processors(i));
  }

  return processors;  
}

// Read and return the system memory data as a dictionary for further processing
unordered_map& LinuxParser::MemoryData() { 
  // Memory utilization to be calculated as total memory as a dict of memory, non-cache/buffer memory, buffers, cached memory, swap
  unordered_map<string, int> memory_data;
  string line, token;

  std::ifstream stream(kProcDirectory + kStatFilename);
    if (stream.is_open()) {
      while (std::getline(stream, line)) {
        // Grab the token from the start of the line, then use it win our dict to assign the value (from istream_iterator since only one int) to the key. 
        std::istringstream linestream(line);
        linestream >> token;
        memory_data[token] = std::istream_iterator<int>(linestream);
      }
    }

  // Define dict and dict values
  unordered_map parsed_mem_data;
  string total_usage, non_cache_buffer, buffer, cache, swap;

  // Parse data into relevant chunks for calling
  parsed_mem_data["mem_total"] = memory_data["MemTotal"];
  parsed_mem_data["mem_free"] = memory_data["MemFree"];
  parsed_mem_data["non_cache_buffer"] = memory_data["MemTotal"] - memory_data["MemFree"] - memory_data["Buffers"] + memory_data["Cached"];
  parsed_mem_data["buffers"] = memory_data["Buffers"];
  parsed_mem_data["cached"] = memory_data["Cached"] + memory_data["Sreclaimable"] - memory_data["Shmem"];
  parsed_mem_data["swap"] = memory_data["SwapTotal"] - memory_data["SwapFree"];

  return (&parsed_mem_data); 
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  unordered_map mem_data = *MemoryData();
  return 100* (mem_data["mem_total"] - mem_data["mem_free"]) / mem_data["mem_total"];
}

// Read amd return Non Cache/Buffer Memory: Total used memory - (Buffers + Cached memory)
int LinuxParser::NonCacheBufferMem() {
  return *MemoryData()["non_cache_buffer"];
}

// Read and return buffer memory
int LinuxParser::BufferMem() {
  return *MemoryData()["buffers"];
}

// Read and return cached memory: Cached + SReclaimable - Shmem
int LinuxParser::CachedMem() {
  return *MemoryData()["cached"];
}

// Read and return swap memory: SwapTotal - SwapFree
int LinuxParser::SwapMem() {
  return *MemoryData()["swap"];
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);

  // Since file only contains 2 numbers in a single line, stream it a single time and assign first value to uptime
  if (stream.is_open()) {
    std::getline(stream, line) 
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime; 
}

// Read and return the number of jiffies for each processor
long LinuxParser::Jiffies(int cpu_number) { 
  return ActiveJiffies(cpu_number) + IdleJiffies(cpu_number); 
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  vector<long> jiffies;
  string line;

  // Gather data from proc/pid/stat file
  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      copy(std::istream_iterator<long>(linestream), std::istream_iterator<long>(), std::back_inserter(jiffies));
    }
  }
  // Calculate active jiffies based on: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
  long utime = jiffies[14];
  long stime = jiffies[15];
  long cutime = jiffies[16];
  long cstime = jiffies[17];
  return utime + stime + cutime + cstime; 
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies(int cpu_number) { 
  vector<long> jiffies;
  long active_jiffies;
  string line, token;

  std::ifstream stream(kProcDirectory + kStatFilename) {
    if (stream.is_open()) {
      while (std::getline(stream, line)) {
        // Get the CPU number and active jiffies
        std::istringstream linestream(line);
        linestream >> token;

        // Stream the cpu_number and attach to cpu string to check for equality, then return a vector of relevant cpu jiffy data
        std::ostringstream oss;
        if (token == oss << "cpu" << (string)cpu_number) {
          copy(std::istring_iterator<long>(linestream), std::istring_iterator(), std::back_insertor(jiffies));
          break;
        }
      }
    }
  }
  // Sum up the relevant jiffies for total active number: user + nice + system + irq + softirq + steal
  active_jiffies = jiffies[0] + jiffies[1] + jiffies[2] + jiffies[5] + jiffies[6] + jiffies[7];
  return active_jiffies; 
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies(int cpu_number) { 
  vector<long> jiffies;
  long idle_jiffies;
  string line, token;

  std::ifstream stream(kProcDirectory + kStatFilename) {
    if (stream.is_open()) {
      while (std::getline(stream, line)) {
        // Get the CPU number and active jiffies
        std::istringstream linestream(line);
        linestream >> token;

        // Stream the cpu_number and attach to cpu string to check for equality, then return a vector of relevant cpu jiffy data
        std::ostringstream oss;
        if (token == oss << "cpu" << (string)cpu_number) {
          copy(std::istring_iterator<long>(linestream), std::istring_iterator(), std::back_insertor(jiffies));
          break;
        }
      }
    }
  }
  // Sum up the relevant jiffies for total idle number: idle + iowait
  idle_jiffies = jiffies[3] + jiffies[4];
  return idle_jiffies;  
}

// Read and return CPU utilization
float LinuxParser::CpuUtilization(int cpu_number) { 
  float total = Jiffies(cpu_number);
  float idle = IdleJiffies(cpu_number);
  
  return (total-idle)/total; 
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string line;
  string token;
  int processes;
  
  ifstream stream(kProcDirectory + kStatFilename);
  
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token >> processes;

      if (token == "processes") {
        return processes;
      }
    }
  }
  return 0; 
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line;
  string token;
  int proc_running;
  
  ifstream stream(kProcDirectory + kStatFilename);
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> token >> proc_running;

    if (token == "processes") {
      return proc_running;
    }
  }
  return 0; 
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string command;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kCmdlineFilename);
  std::istringstream linestream(line);
  linestream >> command;
  
  return command; 
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  long ram;
  string token, line;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token;

      if (token == "VmSize:") {
          linestream >> ram;
          break;
      }
    }
  }
  return std::to_string(ram/1000); 
}

// Reads and returns the User ID for this process
string LinuxParser::Uid(int pid) { 
  string uid;
  string token, line;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> token;

      if (token == "uid:") {
          linestream >> uid;
          break;
      }
    }
  }
  return uid; 
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  string user;
  string line;
  int id;

  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> user >> id;

      if (id == (int)Uid(pid)) {
        break;
      }
    }
  }
  return user; 
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid){
  vector<long> jiffies;
  string line;

  // Gather data from proc/pid/stat file
  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      copy(std::istream_iterator<long>(linestream), std::istream_iterator<long>(), std::back_inserter(jiffies));
    }
  }
  // Calculate active jiffies based on: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
  long starttime = jiffies[22];
  long hertz = std::sysconf(_SC_CLK_TCK);
  int uptime = UpTime() - starttime / hertz;

  return uptime; 
}