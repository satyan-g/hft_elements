This project implements HFT elements - a collection of libraries, services and components that are part of a HFT architecture. 

Project 1:
----------
Implement a library and services for building a MBO feed to MBL feed with following details:
Input is a databento binary MBO feed (replayed using a file). Output is a json MBL book snapshot feed (one per update)

Assumptions and Tradeoffs:
* Cannot drop events from incoming feed
* Aim to produce 1:1 snapshots in the output stream. If there is backpressure, accuracy (Consistent & complete snapshot) with lowest latency trumps gauranteed delivery of every update.


Architecture:
1. Simulator to read dbn file and produce a TCP feed with commandline controllable rate. (Might decide to switch to shm later)
2. MBO incremental to MBL incremental processer in binary
3. MBL incremental to MBL snapshot processor using optimized json caching
4. TCP client/connection handling using epoll. Client on connection sees the latest complete snapshot and continues to receive json snapshots until disconnected.
5. collector client that saves the json snapshots to file.

Implemention:
* optimize for AMD on 
* lockfree and waitfree implementation to support very high throughputs for MBO->MBL conversion
* Employ json caching and incremental building for MBL snapshot serialization
* Use C++20 
* use busy waiting & numa apis, be aware of false sharing
* use CMAKE
* P95 metrics for hop-hop latency and end-end latency (from the point binary MBO event is read from socket to client app receiving same snapshot)

Deployment:
* Docker images for building - reproducible builds
* Docker images for running/demo
* Grafana dashboard 