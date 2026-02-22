# Raw epoll Server

An educational, single-threaded web server built from scratch in C++ using Linux `epoll` and non-blocking I/O. 

## Motivation
This project was built to understand the raw mechanics of high-performance, event-driven backend frameworks. Instead of relying on existing libraries or wrappers, the goal was to communicate directly with the Linux kernel to handle thousands of concurrent connections on a single thread. 

## Architecture
* **Event Loop:** Built directly on Linux `epoll` using Edge-Triggered (`EPOLLET`) mode.
* **Non-Blocking I/O:** All sockets (both server and client) are configured with `O_NONBLOCK`. The event loop aggressively drains the kernel read/write buffers until an `EAGAIN` (Resource temporarily unavailable) error is encountered, ensuring the thread never blocks.
* **Persistent Connections:** Implements HTTP Keep-Alive, keeping TCP connections open to bypass the overhead of repeated 3-way handshakes.
* **Graceful Teardown:** Handles sudden client disconnects safely by ignoring `SIGPIPE` signals with `MSG_NOSIGNAL`, preventing fatal application crashes.

## Benchmark
In local testing using `autocannon` with 100 concurrent connections, this raw engine handled approximately 505,000 requests in 10 seconds (~50,000 req/sec) with an average latency of 1.5ms on a single CPU core.

![Autocannon Benchmark Results](assets/benchmark.png)

## Usage
Compile the server using g++:

```bash
g++ server.cpp -o server
