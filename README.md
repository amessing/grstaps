# Graphically Recursive Simultaneous Task Allocation, Planning, and Scheduling [![Build Status](https://travis-ci.com/amessing/grstaps.svg?token=ZbU2mzJqy2bwdxUCNR2d&branch=master)](https://travis-ci.com/amessing/grstaps)

## Dependencies:
### apt:
- xorg-dev
- libglu1-mesa-dev
### Manually
- (OMPL)[https://ompl.kavrakilab.org/installation.html] Follow the from source instructions (Only 3 commands)
- sudo ln -s /usr/include/eigen3/Eigen /usr/include/Eigen
### included submodules:
- [args](https://github.com/Taywee/args) - Argument Parsing
- [box2d](https://github.com/erincatto/box2d) - A 2D physics engine for games (Used for 2d collision detection)
- [fmt](https://github.com/fmtlib/fmt) - A formatting library
- [json](https://github.com/nlohmann/json) - JSON for Modern C++ (Magic)
- [googletest](https://github.com/google/googletest) - Unit Testing
- [lemon](https://lemon.cs.elte.hu/trac/lemon) - Library for Efficient Modeling and Optimiztation in Networks (Graph library)
- [spdlog](https://github.com/gabime/spdlog) - Fast logging library

## Cloning
### Through SSH
```
git clone --recurse-submodules -j4 git@github.com:amessing/grstaps.git
```
### Through HTTPS
```
git clone https://github.com/amessing/grstaps.git
cd grstaps
sed -i 's/git@github.com:/https:\/\/github.com\//' .gitmodules
git submodule update --init --recursive
```