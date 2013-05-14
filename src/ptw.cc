#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "ptw.h"
#include "debug.h"

namespace ptw {
  const bool DBG = false;

  // ----------------------------------------------------------
  // Queue
  Queue::Queue () : next_(NULL) {
  }
  Queue::~Queue () {
  }

  QueueList::QueueList () : head_(NULL), tail_(NULL) {}
  QueueList::~QueueList () {}
  void QueueList::push (Queue * q) {
    assert (!((this->head_ == NULL) ^ (this->tail_ == NULL)));
    q->next_ = NULL;
    if (this->tail_) {
      this->tail_->next_ = q;
      this->tail_ = q;
    }
    else {
      assert (this->head_ == NULL);
      this->head_ = this->tail_ = q;
    }
  }
  Queue * QueueList::pop () {
    assert (!((this->head_ == NULL) ^ (this->tail_ == NULL)));
    // debug (1, "(%p) head = %p", this, this->head_);
    if (this->head_) {
      Queue * q = this->head_;
      this->head_ = q->next_;

      if (this->tail_ == q) {
        assert (q->next_ == NULL);
        this->tail_ = NULL;
      }

      q->next_ = NULL;
      return q;
    } 
    else {
      return NULL;
    }
  }
  Queue * QueueList::pop_bulk () {
    assert (!((this->head_ == NULL) ^ (this->tail_ == NULL)));

    if (this->head_) {
      Queue * q = this->head_;      
      this->head_ = this->tail_ = NULL;
      return q;
    } 
    else {
      return NULL;
    }    
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
      if (NULL == (q = w->queue_.pop ())) {
        debug (DBG, "enter waiting: %p", w);
        pthread_cond_wait (&(w->cond_), &(w->mutex_));
      }

      debug (DBG, "run: %p", w);
      if (NULL == q) {
        q = w->queue_.pop ();
      }
      pthread_mutex_unlock (&(w->mutex_));

      if (q) {
        q->exec ();
        w->ret_queue (q);
      }
    }    
  }

  pthread_t Worker::pthread () const {
    return this->th_;
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

    if (worker_num == 0) {
      worker_num = Ptw::cpu_core_num ();
      this->worker_.resize (worker_num, NULL);
    }
    for (size_t i = 0; i < worker_num; i++) {
      this->worker_[i] = new Worker (this);
      this->worker_[i]->run ();
    }
  }

  Ptw::~Ptw () {
    for (size_t i = 0; i < this->worker_.size (); i++) {
      pthread_cancel (this->worker_[i]->pthread ());
      pthread_join (this->worker_[i]->pthread (), NULL);
      delete this->worker_[i];
    }
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
    if (NULL == (q = this->queue_.pop ()) && 
        wait && this->in_count_ > this->out_count_) {
      pthread_cond_wait (&(this->cond_), &(this->mutex_));
    }

    if (NULL == q) {
      q = this->queue_.pop ();
    }
    if (q) {
      this->out_count_++;
    }

    pthread_mutex_unlock (&(this->mutex_));

    return q;
  }

  size_t Ptw::worker_num () const {
    return this->worker_.size ();
  }

  size_t Ptw::cpu_core_num () {
    return ::sysconf(_SC_NPROCESSORS_ONLN);
  }

}



