#!/bin/bash

if [ ! -d bin ]; then
  mkdir bin
fi

cd bin
cmake ..
make -j8 icra_experiments
cd icra_2021_experiments

# 16 GB
ulimit -v $((16 * 1024 * 1024)) # kilobytes

# 10m
ulimit -t $((10 * 60)) # seconds

# Display?
ulimit -a

timeout="5m"

for i in {1..10}
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
  if [ ! -f "outputs/output_${i}_0.0_1.json" ]; then

    echo "./icra_experiments -p ${i} -s"
    eval "./icra_experiments -p ${i} -s"
    #echo "timeout --foreground -s 15 -k 10 ${timeout} ./icra_experiments -p ${i} -s"
    #eval "timeout --foreground -s 15 -k 10 ${timeout}  ./icra_experiments -p ${i} -s"
  fi

  for a in "0.0" "0.25" "0.5" "0.75" "1.0"
  do
    if [ ! -f "outputs/output_${i}_${a}_0.json" ]; then
      echo "./icra_experiments -p ${i} -a ${a}"
      eval "./icra_experiments -p ${i} -a ${a}"
      #echo "timeout --foreground -s 15 -k 10 ${timeout} ./icra_experiments -p ${i} -a ${a}"
      #eval "timeout --foreground -s 15 -k 10 ${timeout} ./icra_experiments -p ${i} -a ${a}"
    fi
  done
done
