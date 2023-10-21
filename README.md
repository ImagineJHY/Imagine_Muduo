# Imagine_Muduo
Imagine_Muduo is a network communication implementer providing thread pool with producer-consumer module and realizing IO multiplexing with epoll in C++.

## Any Problems
If you find a bug, post an [issue](https://github.com/ImagineJHY/Imagine_Muduo/issues)! If you have questions about how to use Imagine_Muduo, feel free to shoot me an email at imaginejhy@163.com

## How to Build
Imagine_Muduo uses [CMake](http://www.cmake.org) to support building. Install CMake before proceeding.

#### 1. Navigating into the source directory

#### 2. If it's your first time to build Imagine_Muduo, excuting command 'make init' to init the thirdparty

#### 3. Excuting command 'make prepare' in the source directory

**Note:** If you know what are you doing, you can Navigating into the thirdparty directory and using command 'git checkout' choosing the correct commitId of thirdparty before excuting command 'make prepare'

#### 4. Excuting command 'make build' and Imagine_Muduo builds a shared library by default