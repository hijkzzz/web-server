# speedX
![Build Status](https://img.shields.io/teamcity/codebetter/bt428.svg)

Multi-Threads Web Server

## Structure

**One loop per thread + Thread Pool**

*Establish Connection*

![](https://raw.githubusercontent.com/hijkzzz/speedX/master/newConnection.png)

*Close Connection*

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
