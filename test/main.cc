#include <gtest/gtest.h>
#include <deque>
#include <map>

#include "ptw.h"

class Fib : public ptw::Queue {
private:
  int res_;
  int n_;

public:
  Fib (int n) : res_(0), n_(n) {}  
  ~Fib () {}

  int calc (int n) {
    if (n <= 2 ) {
      return 1;
    }
    else {
      return this->calc (n - 2) + this->calc (n - 1);
    }
  }
  void exec () {
    this->res_ = this->calc (this->n_);
  }
  int res () { return this->res_; }
  int n () { return this->n_; }
};


TEST (Ptw, Basic) {
  ptw::Ptw * p = new ptw::Ptw (2);
  std::map <int, Fib *> q_list;

  const int task_num = 1000;
  for (int n = 0; n < task_num; n++) {
    Fib * f = new Fib (n);

    p->push_queue (f);
  }

  int count = 0;
  ptw::Queue * q;
  while (NULL != (q = p->pop_queue ())) {
    count++;
    Fib * f = dynamic_cast <Fib*> (q);    
    EXPECT_TRUE (f != NULL);
    EXPECT_GT (f->res (), 0);
    auto it = q_list.find (f->n ());
    EXPECT_TRUE (it != q_list.end ());
    q_list.erase (it);
  }

  EXPECT_EQ (task_num, count);
}

int main (int argc, char *argv[])
{
	::testing::InitGoogleTest( &argc, argv );
	return RUN_ALL_TESTS();
}

