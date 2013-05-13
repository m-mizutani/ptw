#include "ptw.h"
#include "debug.h"

namespace ptw {
  const bool DBG = false;

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
    this->ptw_->ret_queue (q);
  }
  void Worker::input_queue (Queue * q) {
    pthread_mutex_lock (&(this->mutex_));
    // debug (DBG, "input queue: %p", q);
    this->queue_.push (q);
    pthread_cond_signal (&(this->cond_));
    pthread_mutex_unlock (&(this->mutex_));    
  }
  void Worker::run () {
    ::pthread_create (&(this->th_), NULL, Worker::loop, this);
  }

  void * Worker::loop (void * obj) {
    Worker * w = static_cast<Worker *>(obj);
    Queue * q;
    debug (DBG, "start loop %p", w);

    while (true) {
      pthread_mutex_lock (&(w->mutex_));
      if (w->queue_.size () == 0) {
        debug (DBG, "enter waiting: %p", w);
        pthread_cond_wait (&(w->cond_), &(w->mutex_));
      }

      debug (DBG, "run: %p", w);
      if (w->queue_.size () > 0) {
        q = w->queue_.front ();
        w->queue_.pop ();
      }
      else {
        q = NULL;
      }
      pthread_mutex_unlock (&(w->mutex_));

      if (q) {
        q->exec ();
        w->ret_queue (q);
      }
    }    
  }

  // ----------------------------------------------------------
  // Ptw
  Ptw::Ptw (size_t worker_num) : 
    worker_(worker_num, NULL), 
    last_ptr_(0),
    in_count_(0),
    out_count_(0)
 { 
    ::pthread_mutex_init (&(this->mutex_), NULL);
    ::pthread_cond_init (&(this->cond_), NULL);
    for (size_t i = 0; i < worker_num; i++) {
      this->worker_[i] = new Worker (this);
      this->worker_[i]->run ();
    }
  }

  Ptw::~Ptw () {
  }

  void Ptw::ret_queue (Queue * q) {
    pthread_mutex_lock (&(this->mutex_));
    this->queue_.push (q);
    pthread_cond_signal (&(this->cond_));
    pthread_mutex_unlock (&(this->mutex_));
  }

  void Ptw::push_queue (Queue *q) {
    this->last_ptr_++;

    // simple round robin algorithm
    if (this->last_ptr_ >= this->worker_.size ()) {
      this->last_ptr_ = 0;
    }

    this->in_count_++;
    this->worker_[this->last_ptr_]->input_queue (q);
  }

  Queue * Ptw::pop_queue (bool wait) {
    Queue * q;

    pthread_mutex_lock (&(this->mutex_));
    if (this->queue_.size () == 0 && wait && this->in_count_ > this->out_count_) {
      pthread_cond_wait (&(this->cond_), &(this->mutex_));
    }

    if (this->queue_.size () > 0) {
      q = this->queue_.front ();
      this->queue_.pop ();
      this->out_count_++;
    }
    else {
      q = NULL;
    }
    pthread_mutex_unlock (&(this->mutex_));

    return q;
  }
}



