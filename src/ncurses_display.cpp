#include <curses.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "format.h"
#include "ncurses_display.h"
#include "system.h"
#include "processor.h"

using std::string;
using std::to_string;

// 50 bars uniformly displayed from 0 - 100 %
// 2% is one bar(|)
std::string NCursesDisplay::ProgressBar(float percent) {
  std::string result{"0%"};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  return result + " " + display + "/100%";
}

//Create progress bar to handle memory data
std::string NCursesDisplay::MemoryBar(float percent) {
  std::string result{""};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < bars; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  return result;
}

void NCursesDisplay::DisplaySystem(System& system, WINDOW* window) {
  int row{0};
  mvwprintw(window, ++row, 2, ("OS: " + system.OperatingSystem()).c_str());
  mvwprintw(window, ++row, 2, ("Kernel: " + system.Kernel()).c_str());
  mvwprintw(window, row, 10, "");
  // Loop through processors in system
    for (auto proc : system.Cpu()) {
    mvwprintw(window, ++row, 2, 
              ("CPU" + std::to_string(proc.CpuNumber()) + ": ").c_str());
    wattron(window, COLOR_PAIR(1));
    wprintw(window, (ProgressBar(proc.Utilization()).c_str()));
    wattroff(window, COLOR_PAIR(1));
  }
  // Validate memory usage and account for different breakdowns of usage in different colors
  mvwprintw(window, ++row, 2, "Memory: ");
  int color_counter = 1;

  wattron(window, COLOR_PAIR(color_counter));
  wprintw(window, "0%%"); 
  wattroff(window, COLOR_PAIR(color_counter));
  
  // Get size of each type of memory normalized to 50 for printing in different colors
  std::vector<long> memory_data{system.NonCacheBufferMem(), system.BufferMem(), system.CachedMem(), system.SwapMem()};
  // Loop through the list of different memory types, track position of bars, and assign different colors
  for (long mem :  memory_data) {
    float mem_usage = (float)mem / system.TotalMemoryUsage();
    wattron(window, COLOR_PAIR(color_counter));

    // Loop through each set of memory usages, accounting for current position in bar count
    wprintw(window, MemoryBar(mem_usage).c_str());
    
    wattroff(window, COLOR_PAIR(color_counter));
    color_counter++;
  }
  // End memory usage with total memory usage
  float percent = system.MemoryUtilization();
  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);

  wattron(window, COLOR_PAIR(4));
  mvwprintw(window, row, 63, (display + "/100%%").c_str());
  wattroff(window, COLOR_PAIR(4));

  // Continue to remaining statistics
  mvwprintw(window, ++row, 2,
            ("Total Processes: " + to_string(system.TotalProcesses())).c_str());
  mvwprintw(window, ++row, 2,
            ("Running Processes: " + to_string(system.RunningProcesses())).c_str());
  mvwprintw(window, ++row, 2,
            ("Up Time: " + Format::ElapsedTime(system.UpTime()) + " ").c_str());
}

void NCursesDisplay::DisplayProcesses(std::vector<Process>& processes,
                                      WINDOW* window, int n) {
  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const cpu_column{16};
  int const ram_column{26};
  int const time_column{35};
  int const command_column{46};
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, cpu_column, "CPU[%%]");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, time_column, "TIME+");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));
  for (int i = 0; i < n; ++i) {
    mvwprintw(window, ++row, pid_column, (string(window->_maxx-2, ' ').c_str()));
    
    mvwprintw(window, row, pid_column, to_string(processes[i].Pid()).c_str());
    mvwprintw(window, row, user_column, processes[i].User().c_str());
    float cpu = processes[i].CpuUtilization() * 100;
    mvwprintw(window, row, cpu_column, to_string(cpu).substr(0, 4).c_str());
    mvwprintw(window, row, ram_column, processes[i].Ram().c_str());
    mvwprintw(window, row, time_column,
              Format::ElapsedTime(processes[i].UpTime()).c_str());
    mvwprintw(window, row, command_column,
              processes[i].Command().substr(0, window->_maxx - 46).c_str());
  }
}

void NCursesDisplay::Display(System& system, int n) {
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color

  int x_max{getmaxx(stdscr)};
  WINDOW* system_window = newwin(8+system.Cpu().size(), x_max - 1, 0, 0);
  WINDOW* process_window =
      newwin(3 + n, x_max - 1, system_window->_maxy + 1, 0);

  while (1) {
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    DisplaySystem(system, system_window);
    DisplayProcesses(system.Processes(), process_window, n);
    wrefresh(system_window);
    wrefresh(process_window);
    refresh();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  endwin();
}
