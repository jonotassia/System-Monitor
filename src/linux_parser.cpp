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
int LinuxParser::NumProcessors() {
  int proc_count = -1;
  string line, token;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      // Get the each token
      std::istringstream linestream(line);
      linestream >> token;

      // Break once the CPU section has been passed
      if (token == "intr") {
        break;
      }

      // Increment proc_count by 1 for each loop, starting at -1 in order to avoid pulling in the aggregate cpu
      proc_count++;
    }
  }

  return proc_count;  
}

// Read and return the system memory data as a dictionary for further processing
unordered_map<string, long> LinuxParser::MemoryData() { 
  // Memory utilization to be calculated as total memory as a dict of memory, non-cache/buffer memory, buffers, cached memory, swap
  unordered_map<string, long> memory_data;
  string line, token;
  string memory_usage;

  std::ifstream stream(kProcDirectory + kMeminfoFilename);
    if (stream.is_open()) {
      while (std::getline(stream, line) && token != "SReclaimable") {
        // Grab the token from the start of the line, then use it win our dict to assign the value (from istream_iterator since only one int) to the key. 
        std::istringstream linestream(line);
        linestream >> token >> memory_usage;
        // Process string for adding to dictionary
        token.erase(token.end()-1, token.end());
        memory_data[token] = (memory_usage != "") ? std::stol(memory_usage) : 0;
      }
    }

  // Define dict and dict values
  unordered_map<string, long> parsed_mem_data;

  // Parse data into relevant chunks for calling
  parsed_mem_data["mem_total"] = memory_data["MemTotal"];
  parsed_mem_data["mem_free"] = memory_data["MemFree"];
  parsed_mem_data["buffers"] = memory_data["Buffers"];
  parsed_mem_data["cached"] = memory_data["Cached"] + memory_data["SReclaimable"] - memory_data["Shmem"];
  parsed_mem_data["swap"] = memory_data["SwapTotal"] - memory_data["SwapFree"];
  parsed_mem_data["non_cache_buffer"] = parsed_mem_data["mem_total"] - parsed_mem_data["mem_free"] - parsed_mem_data["buffers"] - parsed_mem_data["cached"];

  return parsed_mem_data; 
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  unordered_map<string, long> mem_data = MemoryData();
  return (mem_data["mem_total"] - mem_data["mem_free"]) / (float)mem_data["mem_total"];
}

// Read and return Total memory usage
long LinuxParser::TotalMemoryUsage() {
  unordered_map<string, long> memory_data = MemoryData();
  return memory_data["mem_total"];
}

// Read and return Non Cache/Buffer Memory: Total used memory - (Buffers + Cached memory)
long LinuxParser::NonCacheBufferMem() {
  unordered_map<string, long> memory_data = MemoryData();
  return memory_data["non_cache_buffer"];
}

// Read and return buffer memory
long LinuxParser::BufferMem() {
  unordered_map<string, long> memory_data = MemoryData();
  return memory_data["buffers"];
}

// Read and return cached memory: Cached + SReclaimable - Shmem
long LinuxParser::CachedMem() {
  unordered_map<string, long> memory_data = MemoryData();
  return memory_data["cached"];
}

// Read and return swap memory: SwapTotal - SwapFree
long LinuxParser::SwapMem() {
  unordered_map<string, long> memory_data = MemoryData();
  return memory_data["swap"];
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);

  // Since file only contains 2 numbers in a single line, stream it a single time and assign first value to uptime
  if (stream.is_open()) {
    std::getline(stream, line, ' '); 
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  if (uptime != "") {
    return std::stol(uptime);
  } else {
    return 0;
  }
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  vector<string> jiffies;
  string line;

  // Gather data from proc/pid/stat file
  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      copy(std::istream_iterator<string>(linestream), std::istream_iterator<string>(), std::back_inserter(jiffies));
    }
  }
  // Calculate active jiffies based on: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
  long utime = (jiffies[13])!= "" ? std::stol(jiffies[13]) : 0;
  long stime = (jiffies[14])!= "" ? std::stol(jiffies[14]) : 0;
  long cutime = (jiffies[15]) != "" ? std::stol(jiffies[15]) : 0;
  long cstime = (jiffies[16]) != "" ? std::stol(jiffies[16]) : 0;
  return utime + stime + cutime + cstime; 
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveCpuJiffies(int cpu_number) { 
  vector<string> jiffies;
  string line, token;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      // Get the CPU number and active jiffies
      std::istringstream linestream(line);
      linestream >> token;

      // Check for equality, then return a vector of relevant cpu jiffy data
      if (token == "cpu" + std::to_string(cpu_number)) {
        copy(std::istream_iterator<string>(linestream), std::istream_iterator<string>(), std::back_inserter(jiffies));
        break;
      }
    }
  }
  // Sum up the relevant jiffies for total active number: user + nice + system + irq + softirq + steal
  long active_jiffies = 0;
  // Create list of indices that should be checked non-null value before stol and assignment
  vector<int> indices = {0, 1, 2, 5, 6, 7};
  for (auto i : indices) {
    if (jiffies[i] != "") {
      active_jiffies += std::stol(jiffies[i]);
    }
  }
  return active_jiffies; 
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies(int cpu_number) { 
  vector<string> jiffies;
  string line, token;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      // Get the CPU number and active jiffies
      std::istringstream linestream(line);
      linestream >> token;

      // Check for equality, then return a vector of relevant cpu jiffy data
      if (token == "cpu" + std::to_string(cpu_number)) {
        copy(std::istream_iterator<string>(linestream), std::istream_iterator<string>(), std::back_inserter(jiffies));
        break;
      }
    }
  }
  // Sum up the relevant jiffies for total idle number: idle + iowait
  long idle = (jiffies[3] != "") ? std::stol(jiffies[3]) : 0;
  long iowait = (jiffies[4] != "") ? std::stol(jiffies[4]) : 0;
  long idle_jiffies =  idle + iowait;
  return idle_jiffies;  
}

// Read and return the number of jiffies for each processor
long LinuxParser::Jiffies(int cpu_number) { 
  return ActiveCpuJiffies(cpu_number) + IdleJiffies(cpu_number); 
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
  
  std::ifstream stream(kProcDirectory + kStatFilename);
  
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
  
  std::ifstream stream(kProcDirectory + kStatFilename);
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> token >> proc_running;

    if (token == "procs_running") {
      return proc_running;
    }
  }
  return 0; 
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line, command;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kCmdlineFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> command;
    }
  }
  return command; 
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string token, line, ram;

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
  long lram = (ram != "") ? std::stol(ram)/1000 : 0;

  return std::to_string(lram);
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

      if (token == "Uid:") {
          linestream >> uid;
          break;
      }
    }
  }
  return uid; 
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  string user, middle_char, id;
  string line;

  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line) && id != Uid(pid)) {
      std::istringstream linestream(line);

      // Get user by splitting line by ":", then get "x" in between user and id, then get id
      std::getline(linestream, user, ':');
      std::getline(linestream, middle_char, ':');
      std::getline(linestream, id, ':');
    }
  }
  return user; 
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid){
  vector<string> jiffies;
  string line;

  // Gather data from proc/pid/stat file
  std::ifstream stream(kProcDirectory + std::to_string(pid) + "/" + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      copy(std::istream_iterator<string>(linestream), std::istream_iterator<string>(), std::back_inserter(jiffies));
    }
  }
  // Calculate active jiffies based on: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
  long starttime = (jiffies[21] != "") ? std::stol(jiffies[21]) : 0;
  long hertz = sysconf(_SC_CLK_TCK);
  long uptime = UpTime() - starttime / hertz;

  return uptime; 
}
