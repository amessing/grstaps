#!/bin/bash

if [ ! -d bin ]; then
  mkdir bin
fi

cd bin
cmake ..
make -j8 icra_experiments

# 16 GB
ulimit -v 17179869184

for i in {1..25}
do
  echo "Problem ${i}"
  if [ -f "problems/problem_${i}.json" ]; then
    echo "Already done"
    continue
  fi

  j=$(( ( i + 1 ) % 5 ))
  echo "Map ${j}"
  if [ "${j}" == "3" ]; then
    echo "Ignore"
    continue
  fi
  echo "timeout 10m ./icra_experiments -p ${i} -s"
  eval "timeout 10m ./icra_experiments -p ${i} -s"
  echo "timeout 10m ./icra_experiments -p ${i} -a 0.5"
  eval "timeout 10m ./icra_experiments -p ${i} -a 0.5"
done
