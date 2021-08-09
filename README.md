# Graphically Recursive Simultaneous Task Allocation, Planning, and Scheduling

This repository includes the code used for 
```
citation...
```

*We are currently working on a refactored cleaner version of the code which will be used for future papers and linked here when completed.*


## Dependencies:
### apt:
- xorg-dev
- libglu1-mesa-dev
### Manually
- [OMPL](https://ompl.kavrakilab.org/installation.html) Follow the from source instructions (Only 3 commands)
```
sudo ln -s /usr/include/eigen3/Eigen /usr/include/Eigen 
```
### included submodules:
- [args](https://github.com/Taywee/args) - Argument Parsing
- [box2d](https://github.com/erincatto/box2d) - A 2D physics engine for games (Used for 2d collision detection)
- [fmt](https://github.com/fmtlib/fmt) - A formatting library
- [json](https://github.com/nlohmann/json) - JSON for Modern C++ (Magic)
- [googletest](https://github.com/google/googletest) - Unit Testing
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
