#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER classic_blink_tree

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "unittest/gunit/innodb/tree-tests/tree-implementations/blink-tree/util/blink-tp.h"

#if !defined(_BLINK_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _BLINK_TP_H

#include <lttng/tracepoint.h>


TRACEPOINT_EVENT(
    classic_blink_tree,
    write_start,
    TP_ARGS(
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    classic_blink_tree,
    write_end,
    TP_ARGS(
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    classic_blink_tree,
    read_start,
    TP_ARGS(
        // char*, key_arg,
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    classic_blink_tree,
    read_end,
    TP_ARGS(
        // char*, key_arg,
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    classic_blink_tree,
    descending_start,
    TP_ARGS(
        // char*, key_arg,
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    classic_blink_tree,
    descending_end,
    TP_ARGS(
        // char*, key_arg,
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    classic_blink_tree,
    root_split_start,
    TP_ARGS(
        // char*, key_arg,
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    classic_blink_tree,
    root_split_end,
    TP_ARGS(
        // char*, key_arg,
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)


TRACEPOINT_EVENT(
    classic_blink_tree,
    split_start,
    TP_ARGS(
        // char*, key_arg,
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    classic_blink_tree,
    split_end,
    TP_ARGS(
        // char*, key_arg,
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        // ctf_string(key, key_arg)
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)


#endif /* _BLINK_TP_H */

#include <lttng/tracepoint-event.h>