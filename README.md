PTW
===============

Library of Pthread based Worker Thread Model.


    #include "ptw.h"
    
    class MyTask : public ptw::Queue {
    public:
      int res_;
      void exec () {
        for (int i = 0; i < 100000; i++) {
          this->res_ += i;
        }
      }
    };
    
    int main (int argc, char *argv[]) {
      ptw::Ptw * p = new ptw::Ptw ();
    
      const int task_num = 10000;
      for (int n = 0; n < task_num; n++) {
        p->push_queue (new MyTask ());
      }
    
      MyTask * task;
      while (NULL != (task = dynamic_cast <MyTask*> (p->pop_queue ()))) {
        std::cout << task->res_ << std::endl;
        delete task;
      }
    
      delete p;
      return 0;
    }
    
