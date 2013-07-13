#include <deque>
#include <vector>
#include <string>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include "opt_parse.h"

#include <ptw.h>

class Job : public ptw::Queue {
private:
  const static size_t base_load_ = 1000;
  size_t job_size_;
  size_t r_;
  int res_;

public:
  Job (size_t job_size) : job_size_(job_size) {
    this->r_ = (rand () % job_size) + 1;
  }
  void exec () {
    int c = 0;
    for (int i = 1; i < this->job_size_ * Job::base_load_; i++) {
      if (i % this->r_ == 0) {
        c++;
      }
    }
    this->res_ = c;
  }
};

int main (int argc, char *argv[]) {
  optparse::OptionParser psr = optparse::OptionParser ();
  psr.add_option("-j", "--job-num").set_default(10) .dest("job_num") .type("int") .help("number of job (x 1024)");
  psr.add_option("-t", "--test-num").set_default(32) .dest("test_num") .type("int") .help("number of test repeat");
  psr.add_option("-s", "--job-size").set_default(1) .dest("job_size") .type("int") .help("task size of each job");
  psr.add_option("-f", "--out-fmt").set_default("text") .dest("out_fmt") .help("output format [text,csv]");

  optparse::Values& opt = psr.parse_args(argc, argv);
  std::vector<std::string> args = psr.args();

  const int job_num = static_cast<int> (opt.get("job_num")) * 1024;
  const int test_num = opt.get("test_num");
  const size_t job_size = opt.get("job_size");


  std::vector <Job*> job_array (job_num);
  std::deque <double> ts_list;

  for (size_t i = 0; i < job_array.size (); i++) {
    job_array[i] = new Job (job_size);
  }

  const size_t core_num = ptw::Ptw::cpu_core_num ();

  for (int t = 0; t < test_num; t++) {
    ptw::Ptw *pw = new ptw::Ptw (core_num);
    struct timeval ts_start, ts_end, ts_sub;
    gettimeofday (&ts_start, NULL);
    for (size_t i = 0; i < job_array.size (); i++) {
      pw->push_queue (job_array[i]);
    }

    ptw::Queue * q;
    while (NULL != (q = pw->pop_queue (true))) {
      // nothing to do
    }
    gettimeofday (&ts_end, NULL);

    timersub (&ts_end, &ts_start, &ts_sub);
    double ts = double (ts_sub.tv_sec) + (double (ts_sub.tv_usec) / 1000000);
    delete pw;
    ts_list.push_back (ts);
  }

  double total = 0;
  double max = 0; 
  double min = 0;

  for (auto it = ts_list.begin (); it != ts_list.end (); it++) {
    double ts = (*it);
    total += ts;
    max = (ts > max || max == 0) ? ts : max;
    min = (ts < min || min == 0) ? ts : min;
  }

  double avg = total / (double)(test_num);
  printf ("Core Num: %zd\n", core_num);
  printf ("Average: %f job/sec\n", (double)(job_num) / avg);
  printf ("Fastest: %f job/sec\n", (double)(job_num) / min);
  printf ("Slowest: %f job/sec\n", (double)(job_num) / max);

  for (size_t i = 0; i < job_array.size (); i++) {
    delete job_array[i];
  }

  return 0;
}
