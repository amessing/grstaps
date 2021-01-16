#!/bin/bash

if [ ! -d bin ]; then
  mkdir bin
fi

cd bin
cmake ..
make -j8 icra_experiments
cd icra_2021_experiments

echo "./icra_experiments -p 1002 -v 11 -a 0.75"
eval "./icra_experiments -p 1002 -v 11 -a 0.75"
mv problems/problem_1002.json problems/robotarium_problem.json
mv outputs/output_1002_0.75_0.json outputs/robotarium_output.json