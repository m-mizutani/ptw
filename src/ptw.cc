#include "ptw.h"

namespace ptw {
  // ----------------------------------------------------------
  // Queue
  Queue::Queue () {
  }
  Queue::~Queue () {
  }

  // ----------------------------------------------------------
  // Worker
  Worker::Worker (Ptw * ptw) : ptw_(ptw) {
    ::pthread_mutex_init (&(this->mutex_), NULL);
    ::pthread_cond_init (&(this->cond_), NULL);
  }
  Worker::~Worker () {
  }
  void Worker::ret_queue (Queue * q) {
  }
  void Worker::input_queue (Queue * q) {
    
  }

  // ----------------------------------------------------------
  // Ptw
  Ptw::Ptw (size_t worker_num) : worker_(worker_num, NULL) { 
    for (size_t i = 0; i < worker_num; i++) {
      this->worker_[i] = new Worker (this);
    }
  }

  Ptw::~Ptw () {
  }
  void Ptw::push_queue (Queue *q) {
  }

  Queue * Ptw::pop_queue () {
    return NULL;
  }
}



