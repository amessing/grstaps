 #!/bin/bash

# todo: make better...


mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
cmake --build . --config Debug -- -j $(nproc)

# Unit test
./grstaps_test

# Coverage?