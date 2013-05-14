#ifndef __LIB_PTW_H__
#define __LIB_PTW_H__

#include <vector>
#include <queue>
#include <pthread.h>

namespace ptw {
  class Ptw;
  class QueueList;

  class Queue {
    friend QueueList;
  private:
    Queue * next_;

  public:    
    Queue ();
    virtual ~Queue ();
    virtual void exec () = 0; 
  };

  class QueueList {
  private:
    Queue * head_;
    Queue * tail_;

  public:
    QueueList ();
    ~QueueList ();
    void push (Queue * q);
    Queue * pop ();
    Queue * pop_bulk ();
  };

  class Worker {
  private:
    QueueList queue_;
    // std::queue <Queue *> queue_;
    Ptw * ptw_;
    pthread_t th_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
    
    static void * loop (void * obj);

  public:
    Worker (Ptw * ptw);
    virtual ~Worker ();
    void input_queue (Queue * q);
    void run ();
    void ret_queue (Queue * q);
    pthread_t pthread () const;
  };

  class Ptw {
    friend void Worker::ret_queue (Queue * q);

  private:
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;    
    std::vector <Worker *> worker_;
    QueueList queue_;
    // std::queue <Queue *> queue_;
    int last_ptr_;
    int in_count_;
    int out_count_;

    void ret_queue (Queue * q);

  public:
    Ptw (size_t worker_num=0);
    ~Ptw ();
    void push_queue (Queue *q); 
    Queue * pop_queue (bool wait=true);

    size_t worker_num () const;
    static size_t cpu_core_num () ;
  };
}

#endif
