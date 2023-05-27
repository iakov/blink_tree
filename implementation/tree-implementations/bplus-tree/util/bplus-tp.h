#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER bplus_tree

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "unittest/gunit/innodb/tree-tests/tree-implementations/bplus-tree/util/bplus-tp.h"

#if !defined(_BPLUS_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _BPLUS_TP_H

#include <lttng/tracepoint.h>


TRACEPOINT_EVENT(
    bplus_tree,
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
    bplus_tree,
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
    bplus_tree,
    descend_to_leaf_s_locking_start,
    TP_ARGS(
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    bplus_tree,
    descend_to_leaf_s_locking_end,
    TP_ARGS(
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    bplus_tree,
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
    bplus_tree,
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
    bplus_tree,
    descending_no_locks_start,
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
    bplus_tree,
    descending_no_locks_end,
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
    bplus_tree,
    optimistic_write_start,
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
    bplus_tree,
    optimistic_write_end,
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
    bplus_tree,
    write_start,
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
    bplus_tree,
    write_end,
    TP_ARGS(
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    bplus_tree,
    index_lock_write_start,
    TP_ARGS(
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)

TRACEPOINT_EVENT(
    bplus_tree,
    index_lock_write_end,
    TP_ARGS(
        int, thread_arg,
        char*, workload_type
    ),
    TP_FIELDS(
        ctf_integer(int, thread, thread_arg)
        ctf_string(type, workload_type)
    )
)




TRACEPOINT_EVENT(
    bplus_tree,
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
    bplus_tree,
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
    bplus_tree,
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
    bplus_tree,
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



TRACEPOINT_EVENT(
    bplus_tree,
    pessimistic_write_start,
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
    bplus_tree,
    pessimistic_write_end,
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






#endif /* _BPLUS_TP_H */

#include <lttng/tracepoint-event.h>