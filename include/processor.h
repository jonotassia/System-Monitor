#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  Processor(int cpu_number);
  float Utilization();

 private:
    int cpu_number_;
};

#endif