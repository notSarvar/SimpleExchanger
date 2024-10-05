# SimpleExchanger

## Description
---

Simple exchenger with client-server arcitecture. 
Exchanger provides:
 - Connecting multiple clients simultaneosly
 - Make orders for selling or buying currency for client
 - Check client's balance and orders 
 - Logging transactions #TODO

 ## Technical requirements and build
 ---

 For buld programm requires: 
 - Boost.Asio library
 - C++17 compiler
 - Cmake minimum version 3.21

 ***

 To build you need this commands run in terminal:
  ```sh
    mkdir build
    cd build
    cmake --DCMAKE_BUILD_TYPE=Release ..
    make 
  ```
## Using
    
    ./Server <port number (5555 is default)> 
    
    ./Client <server's port number (5555 is default)>
    
