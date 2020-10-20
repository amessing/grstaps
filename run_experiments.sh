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

for alpha in "s" "0.00" "0.25" "0.50" "0.75" "1.00"; do
  problem_nr=0
  for problem_version in {0..9}; do
    for iteration in {0..14}; do
      problem_nr=$((problem_nr + 1))
      echo "Problem ${problem_nr}"

      map_nr=$(( (problem_nr + 1) % 5 ))
      if [ "${map_nr}" == "3" ]; then
        echo "Map 3; skipping"
        continue
      fi

      if [ "${alpha}" == "s" ]; then
        if [ ! -f "outputs/output_${problem_nr}_0.00_1.json" ]; then
          echo "./icra_experiments -p ${problem_nr} -v ${problem_version} -s"
          eval "./icra_experiments -p ${problem_nr} -v ${problem_version} -s"
        fi
      else
        if [ ! -f "outputs/output_${problem_nr}_${alpha}_0.json" ]; then
          echo "./icra_experiments -p ${problem_nr} -v ${problem_version} -a ${alpha}"
          eval "./icra_experiments -p ${problem_nr} -v ${problem_version} -a ${alpha}"
        fi
      fi
    done
  done
done