# speed-x

![Build Status](https://img.shields.io/teamcity/codebetter/bt428.svg)

Multi-Threaded Web Server

## Design Pattern

**One Loop Per Thread**

Each thread has an event loop `epoll`

**Multiple Reactor**

Main event loop is responsible for `listenfd` and `accept`
Sub event loop is responsible for `handleEvent`
Use the roundbin scheduling policy

## Flow Diagram

**Establish Connection**

![](https://raw.githubusercontent.com/hijkzzz/speedX/master/newConnection.png)

The main reactor(event loop) distributes the connection to the sub-reactor, when a new connection is made

**Close Connection**

![](https://raw.githubusercontent.com/hijkzzz/speedX/master/closeConnection.png)

## Requirements

- Linux
- GCC
- Boost

## Build 

```
sudo apt-get install libboost-all-dev
cd build
cmake ..
make
```

## Usage

```
./speedX

http://127.0.0.1:8000
```
