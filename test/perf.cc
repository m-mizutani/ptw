#include <ptw.h>
#include <deque>
#include <vector>
#include <sys/time.h>
#include <stdio.h>

class Task : public ptw::Queue {
public:
  void exec () {
    // nothing to do
  }
};

int main (int argc, char *argv[]) {
  const int task_num = 1000000;
  std::vector <Task*> task_array (task_num);
  const int test_num = 10;
  std::deque <double> ts_list;

  for (size_t i = 0; i < task_array.size (); i++) {
    task_array[i] = new Task ();
  }

  const size_t core_num = ptw::Ptw::cpu_core_num ();

  for (int t = 0; t < test_num; t++) {
    ptw::Ptw *pw = new ptw::Ptw (core_num);
    struct timeval ts_start, ts_end, ts_sub;
    gettimeofday (&ts_start, NULL);
    for (size_t i = 0; i < task_array.size (); i++) {
      pw->push_queue (task_array[i]);
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
  printf ("Average: %f task/sec\n", (double)(task_num) / avg);
  printf ("Fastest: %f task/sec\n", (double)(task_num) / min);
  printf ("Slowest: %f task/sec\n", (double)(task_num) / max);
  
  return 0;
}
