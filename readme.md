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

 To build the program you need to run this commands in terminal:
  ```sh
    mkdir build
    cd build
    cmake --DCMAKE_BUILD_TYPE=Release ..
    make 
  ```
## Using
---
  After you build the program, the current bin files will be located in the $bin$ folder, where you can run them.
    
  ```sh
  ./Server <port number (5555 is default)> 
  
  ./Client <servers port number (5555 is default)>
  ```    
