#ifndef __SELECTOR__
#define __SELECTOR__

#include <sys/select.h>  // select
#include <arpa/inet.h>   // struct sockaddr_in
#include <cstring>       // linux
#include <cstdlib>       // linux
#include <cstdio>        // linux
#include <string>
#include <cassert>
#include "alloutil/Log.hpp"

struct Receiver {
  int fileDescriptor;
  float waitingTime;

  void init(unsigned port) {

    if ((fileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
      perror("socket");
      exit(-1);
    }

    waitingTime = 0;

    // magic
    //
    int window = 16777216;
    if (setsockopt(fileDescriptor, SOL_SOCKET, SO_RCVBUF, &window, sizeof(int)) == -1) {
      fprintf(stderr, "Error setting socket opts: %s\n", strerror(errno));
    }
    printf("%d byte receive buffer (aka \"window\")\n", window);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    // we need the :: to disambiguate between std::bind (c++11) socket.h bind
    //
    if (::bind(fileDescriptor, (sockaddr*)&address, sizeof(sockaddr)) == -1) {
      perror("bind");
      exit(-1);
    }

    LOG("Receiver listening on port %d", port);
  }

  bool receive(void* buffer, unsigned packetSize, float timeOut) {
    fd_set fileDescriptorSet;
    FD_ZERO(&fileDescriptorSet);
    FD_SET(fileDescriptor, &fileDescriptorSet);

    int seconds = (int)timeOut;
    int microseconds = (timeOut - (int)timeOut) * 1000000;
    if (microseconds > 999999) microseconds = 999999;

    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = microseconds;

    int rv = select(fileDescriptor + 1, &fileDescriptorSet, 0, 0, &tv);

    // XXX so is tv now the actual time that select waited?
    //
    float waited = tv.tv_sec + tv.tv_usec / 1000000.0f;

    if (rv == -1) {
      LOG("select error %d", errno);
      return false;
    } else if (rv == 0) {
      waitingTime += timeOut;
      return false;
    } else {
      //LOG("waited for %fs for packet", waited + waitingTime);
      waitingTime = 0;
      int bytesReceived = recvfrom(fileDescriptor, buffer, packetSize, 0, 0, 0);
      if (bytesReceived == -1) {
        perror("recvfrom");
        return false;
      } else if (bytesReceived != (int)packetSize) {
        printf("Received less than expected\n");
        return false;
      } else {
        return true;
      }
    }
  }
};

#endif
