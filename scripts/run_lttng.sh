#!/usr/bin/env bash

set -x
set -e

readonly TESTS_PATH="/home/dmitrii/mysql-server/build/unittest/gunit/innodb/tree-tests/tree-implementations"
DATA=$(date +%Y-%m-%d_%H-%M-%S)
LTTNG_PATH="/home/dmitrii/lttng-traces"

readonly BABELTRACE_OUTPUT_PATH="/home/dmitrii/babeltrace"
readonly PARSER="/home/dmitrii/lttng-scripts/babeltrace_processor.py"
readonly PARSER2="/home/dmitrii/lttng-scripts/babeltrace_processor_sum.py"
readonly PARSER2_FOLDER="/home/dmitrii/lttngs_row_data/result_${DATA}"
readonly RESULT_FOLDER="/home/dmitrii/lttng_results"
readonly RESULT_FILE="${RESULT_FOLDER}/result_${DATA}"


#readonly THREADS=(1 4 12 16 24 32 48 64 128)
readonly THREADS=(1 4 12 16 24)
readonly READ_THREADS_PERC=(0.0 0.2 0.5 0.8)
readonly KEY_SIZES=(4 8 16 32 64 128)
readonly CASES=(bplus_tree jaluta_tree)
readonly START_TREE_SIZES=(0 1000000 10000000)
readonly RAND_TYPES=("RANDOM" "AUTOINCREAMENT" "SHAREDAUTOINCREAMENT")



export LTTNG_TRACE=1
export NODE_SIZE=14  #16 KB
export KEY_GAP=10000000
export KEYS_AMOUNT=1000000

mkdir -p "${BABELTRACE_OUTPUT_PATH}"
mkdir -p "${RESULT_FOLDER}"
mkdir -p "${PARSER2_FOLDER}"

touch  "${RESULT_FILE}"

echo "" >"${RESULT_FILE}"
rm -rf ${LTTNG_PATH}jal*
rm -rf ${LTTNG_PATH}bplus*
rm -rf ${LTTNG_PATH}cla*

for rand_type in "${RAND_TYPES[@]}"; do
    for key_size in "${KEY_SIZES[@]}"; do
        for start_tree_size in "${START_TREE_SIZES[@]}"; do
            for threads in "${THREADS[@]}"; do
                for read_threads_part in "${READ_THREADS_PERC[@]}"; do
                    read_threads="$(python -c "print('%i' % (${threads} * ${read_threads_part}))")"
                    for case in "${CASES[@]}"; do
                        export START_TREE_SIZE="$start_tree_size"
                        export THREADS_AMOUNT="$threads"
                        export KEY_SIZE="$key_size"
                        export READ_THREADS="$read_threads"
                        export RAND_TYPE="$rand_type"

                        session_name="${case}-${start_tree_size}_${threads}_${read_threads}_${key_size}_${read_threads_part}_${rand_type}"
                        echo "case: ${session_name}"

                        lttng create "${session_name}" --output="${LTTNG_PATH}/${session_name}"
                        lttng enable-channel -u --subbuf-size=2097152 blink_trace_channel
                        lttng enable-event -c blink_trace_channel -u "${case}:*"
                        # lttng start

                        binary_name=""

                        if [ "$case" = "jaluta_tree" ]; then
                            binary_name="jaluta-tree/jaluta-tree-t"
                        fi 
                        if [ "$case" = "classic_blink_tree" ]; then
                            binary_name="blink-tree/blink-tree-t"
                        fi 
                        if [ "$case" = "bplus_tree" ]; then
                            binary_name="bplus-tree/bplus-tree-t"
                        fi 

                        ${TESTS_PATH}/${binary_name} --gtest_filter="*configurable_test"
                        lttng destroy
                    done
                done
            done

            for threads in "${THREADS[@]}"; do
                for read_threads_part in "${READ_THREADS_PERC[@]}"; do
                    read_threads="$(python -c "print('%i' % (${threads} * ${read_threads_part}))")"
                    for case in "${CASES[@]}"; do
                        session_name="${case}-${start_tree_size}_${threads}_${read_threads}_${key_size}_${read_threads_part}_${rand_type}"
                        (babeltrace "${LTTNG_PATH}/${session_name}"  >"${BABELTRACE_OUTPUT_PATH}/${session_name}";
                        ${PARSER} "${BABELTRACE_OUTPUT_PATH}/${session_name}" >"${RESULT_FILE}_${session_name}";
                        rm "${BABELTRACE_OUTPUT_PATH}/${session_name}") &
                    done
                done
            done

            wait
        done
    done
done

for rand_type in "${RAND_TYPE[@]}"; do
    for key_size in "${KEY_SIZES[@]}"; do
        for start_tree_size in "${START_TREE_SIZES[@]}"; do
            for threads in "${THREADS[@]}"; do
                for read_threads_part in "${READ_THREADS_PERC[@]}"; do
                    read_threads="$(python -c "print('%i' % (${threads} * ${read_threads_part}))")"
                    for case in "${CASES[@]}"; do
                        session_name="${case}-${start_tree_size}_${threads}_${read_threads}_${key_size}_${read_threads_part}_${rand_type}"
                        cat "${RESULT_FILE}_${session_name}" >>"${RESULT_FILE}"
                        rm "${RESULT_FILE}_${session_name}"
                    done
                done
            done
        done
    done
done

for key_size in "${KEY_SIZES[@]}"; do
    for start_tree_size in "${START_TREE_SIZES[@]}"; do
        for threads in "${THREADS[@]}"; do
            for read_threads_part in "${READ_THREADS_PERC[@]}"; do
                read_threads="$(python -c "print('%i' % (${threads} * ${read_threads_part}))")"
                for case in "${CASES[@]}"; do
                    session_name="${case}-${start_tree_size}_${threads}_${read_threads}_${key_size}_${read_threads_part}_${rand_type}"
                    ${PARSER2} "${BABELTRACE_OUTPUT_PATH}${session_name}" >"${PARSER2_FOLDER}/${session_name}" &
                done
            done
        done
    done
done

wait
