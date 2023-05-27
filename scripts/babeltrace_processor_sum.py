#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from collections import defaultdict
import numpy as np
import os

def str_to_ns(time_str):
     h, m, s = time_str.split(":")
     int_s, ns = s.split(".")
     ns = map(lambda t, unit: np.timedelta64(t, unit),
              [h,m,int_s,ns.ljust(9, '0')],['h','m','s','ns'])
     return sum(ns)

def parse_trace(trace):
    trace = trace.replace("{", "").replace("}", "").replace("=", "").replace(",", "").replace("[", "").replace("]", "")
    time, time_delta, host, point_name, _ , cpu_id, _, thread_id, _, workload = trace.split()
    point_name = point_name[:-1] # remove last ":" 
    return point_name, time, thread_id, workload.replace("\"", "")

def get_time_delta(time1, time2):
    # print("time1 " + time1)
    # print("time2 " + time2)
    return str_to_ns(time1) - str_to_ns(time2)


def main():
    if len(sys.argv) >= 3:
        print("Too much arguments. Can be one: path to traced file or zero: then read stdin")
        exit(-1)
    elif len(sys.argv) == 2:
        trace_source = open(sys.argv[1], 'r')
    else:
        trace_source = sys.stdin

    traces = {}
    for trace in trace_source:
        name, time, thread_id, workload = parse_trace(trace)
        # print("name is " + name + " time is " + time + " thread id is " + thread_id)
        if name.endswith("start"):
            if name + str(thread_id) + workload in traces:
                print("Two starts in a row: " + trace)
                exit(-1)
            else:
                traces[name + str(thread_id) + workload] = time
        elif name.endswith("end"):
            if name[:-3] + "start" + thread_id  + workload in traces:
                print(name[:-4] + " " + workload, time, get_time_delta(time, traces[name[:-3] + "start" + thread_id + workload]), thread_id)
                traces.pop(name[:-3] + "start" + thread_id + workload)
            else:
                print("Unknown trace: " + trace)
                exit(-1)

    if len(traces.keys()) != 0:
        print("Warning! Not ended events")
    # Code goes over here.
    return 0

if __name__ == "__main__":
    main()