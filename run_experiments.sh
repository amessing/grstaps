#!/bin/bash

if [ ! -d bin ]; then
  mkdir bin
fi

cd bin
cmake ..
make -j8 icra_experiments

# 16 GB
ulimit -v 17179869184

for i in {1..200}
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
  echo "timeout 30m ./icra_experiments -p ${i} -s"
  eval "timeout 30m ./icra_experiments -p ${i} -s"
done

for a in "0.0" "0.25" "0.5" "0.75" "1.0"
do
  for i in {1..200}
  do
    echo "Problem ${i}"
    j=$(( ( i + 1 ) % 5 ))
    echo "Map ${j}"
    if [ "${j}" == "3" ]; then
      echo "Ignore"
      continue
    fi

    if [ ! -f "outputs/output_${i}_${a}_0.json" ]; then
      echo "timeout 30m ./icra_experiments -p ${i} -a ${a}"
      eval "timeout 30m ./icra_experiments -p ${i} -a ${a}"
    fi
  done
done

