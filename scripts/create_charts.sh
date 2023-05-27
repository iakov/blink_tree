#!/bin/bash

#set -x
set -e

readonly THREADS=(1 4 12 16 24 32 48 64 128)
readonly KEY_SIZES=(4 8 16 32 64 128)
readonly CASES=(bplus_tree jaluta_tree)
readonly START_TREE_SIZES=(0 1000000 10000000)
readonly RAND_TYPES=("RANDOM" "AUTOINCREAMENT" "SHAREDAUTOINCREAMENT")

echo "Making charts from ${1} into ${2}"
mkdir -p "${2}"

for rand_type in "${RAND_TYPES[@]}"; do
    for key_size in "${KEY_SIZES[@]}"; do
        for start_tree_size in "${START_TREE_SIZES[@]}"; do
            export START_TREE_SIZE="$start_tree_size"
            export KEY_SIZE="$key_size"
            export RAND_TYPE="$rand_type"

            python3 events_statistic.py "${1}" "${2}/events_${start_tree_size}_${key_size}_${rand_type}" &
        done

        wait
    done
done


for rand_type in "${RAND_TYPES[@]}"; do
    export RAND_TYPE="$rand_type"

    python3 tree_size_dependency.py "${1}" "${2}/tree_size_${rand_type}" &

done
wait

for start_tree_size in "${START_TREE_SIZES[@]}"; do
    export START_TREE_SIZE="$start_tree_size"

    python3 rand_type_dependency.py "${1}" "${2}/random_${start_tree_size}" &
done
wait

for rand_type in "${RAND_TYPES[@]}"; do
      for start_tree_size in "${START_TREE_SIZES[@]}"; do
          export RAND_TYPE="$rand_type"
          export START_TREE_SIZE="$start_tree_size"

          python3 rw_statistic.py "${1}" "${2}/rw_${rand_type}" &
      done
      wait
done
