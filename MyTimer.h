#ifndef mytimer_h
#define mytimer_h

// Class for software timers. Should be useful for debouncing and buttons management.

class MyTimer {
  private: // internal variables
  int delay_time;
  unsigned long t_int;
  bool active = false;

  public:
  MyTimer(); // constructor
  
  // methods declaration
  void set(int); // set the delay time
  void start(); // start the timer
  bool check(); // check the timer status
};  // <-- This semicolon is needed!!!


// methods definitions

MyTimer::MyTimer() {
  active = false;
}

void MyTimer::set(int n) {
  delay_time = n;
}

void MyTimer::start() {
  t_int = millis();
  active = true;
}

bool MyTimer::check() {
  if (active == true) {
    unsigned long dt = millis()-t_int;
    if (dt >= delay_time) { active = false; }
  }
  return active;
}

#endif
