//#define LOG_FILE ("/tmp/log.txt")
#include "Cuttlebone/Help.hpp"
#include <unistd.h>

struct State {
  int data[100];
};

struct MyApp : SubscriberManualPolling<State> {
  State* state;
  MyApp() {
    shouldLog = true;
    LOG("MyApp() - State is %d bytes", sizeof(State));
    state = new State;
  }
  virtual void firstRun() {
    LOG("firstRun()");
    while (true) {
      int popCount = getState(*state);
      LOG("state.data[0] = %d; popCount = %d", state->data[0], popCount);
      usleep(16666);
    }
  }
};

int main() {
  LOG("main()");
  MyApp app;
  app.start();
}