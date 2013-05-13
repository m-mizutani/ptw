#ifndef __LIB_PTW_H__
#define __LIB_PTW_H__

#include <vector>
#include <queue>
#include <pthread.h>

namespace ptw {
  class Ptw;

  class Queue {
  public:    
    Queue ();
    virtual ~Queue ();
    virtual void exec () = 0;    
  };

  class Worker {
  private:
    std::queue <Queue *> queue_;
    Ptw * ptw_;
    pthread_t th_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
    
    void ret_queue (Queue * q);
    static void * loop (void * obj);

  public:
    Worker (Ptw * ptw);
    virtual ~Worker ();
    void input_queue (Queue * q);
    void run ();
  };

  class Ptw {
    friend void Worker::ret_queue (Queue * q);

  private:
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;    
    std::vector <Worker *> worker_;
    std::queue <Queue *> queue_;
    int last_ptr_;
    int in_count_;
    int out_count_;

    void ret_queue (Queue * q);

  public:
    Ptw (size_t worker_num);
    ~Ptw ();
    void push_queue (Queue *q); 
    Queue * pop_queue (bool wait=true);
  };
}

#endif
