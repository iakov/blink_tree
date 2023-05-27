#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from collections import defaultdict
import os
import bt2
# from numba import jit
import numpy as np


def print_result(total_result, header):
    for event, workload_thread_res in total_result.items():
        _, event_name = str(event).split(":")
        for workload, thread_res in workload_thread_res.items():
            amount = 0
            time = 0
            for thread, res in thread_res.items():
                time += res[0]
                amount += res[1]
            print(header, event_name, workload, amount, int(time / amount))


# @jit(parallel=True)
def iterate(it, traces):
    for msg in it:
        # We only care about event messages  
        if type(msg) is not bt2._EventMessageConst:
            continue

        event = msg.event
        name = event.cls.name
        time = msg.default_clock_snapshot.ns_from_origin
        thread_id = str(event['thread'])
        workload = str(event['type'])


        # print("name is " + name + " time is " + str(time) + " thread id is " + thread_id + " workload is " + workload)
        if name.endswith("start"):
            traces[name[:-6]][workload][thread_id][0] -= time
        else: 
            element = traces[name[:-4]][workload][thread_id]
            element[0] += time
            element[1] += 1

        
        
def main():
    # trace_dir header
    if len(sys.argv) >= 4:
        print("Too much arguments. Can be one: path to traced file or zero: then read stdin")
        exit(-1)
 

    it = bt2.TraceCollectionMessageIterator(sys.argv[1])
    
    header = sys.argv[2]
    traces = defaultdict(lambda : defaultdict(lambda : defaultdict(lambda : [0, 0])))

    iterate(it, traces)

    print_result(traces, header)

    # Code goes over here.
    return 0

if __name__ == "__main__":
    main()