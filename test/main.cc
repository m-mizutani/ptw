#include <gtest/gtest.h>
#include <deque>
#include <map>

#include "ptw.h"

class Prime : public ptw::Queue {
private:
  int res_;
  int n_;

public:
  Prime (int n) : res_(0), n_(n) {}  
  ~Prime () {}

  int calc (int n) {
    for (int i = 2; i < n; i++) {
      if (n % i == 0) {
        return 1;
      }
    }
    
    return 2;
  }
  void exec () {
    this->res_ = this->calc (this->n_);
  }
  int res () { return this->res_; }
  int n () { return this->n_; }
};


TEST (Ptw, Basic) {
  ptw::Ptw * p = new ptw::Ptw (2);
  std::map <int, Prime *> q_list;

  const int task_num = 10000;
  for (int n = 0; n < task_num; n++) {
    Prime * f = new Prime (n);
    p->push_queue (f);
    q_list.insert (std::make_pair (n, f));
  }

  int count = 0;
  for (int n = 0; n < task_num; n++) {
    ptw::Queue * q = p->pop_queue ();
    ASSERT_TRUE (q != NULL);
    count++;
    Prime * f = dynamic_cast <Prime*> (q);    
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

