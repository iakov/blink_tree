#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER jaluta_tree

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "unittest/gunit/innodb/tree-tests/tree-implementations/jaluta-tree/util/blink-tp.h"

#if !defined(_BLINK_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _BLINK_TP_H

#include <lttng/tracepoint.h>


TRACEPOINT_EVENT(
    jaluta_tree,
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
    jaluta_tree,
    write_end,
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
    jaluta_tree,
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
    jaluta_tree,
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
    jaluta_tree,
    descend_cover_block_start,
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
    jaluta_tree,
    descend_cover_block_end,
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
    jaluta_tree,
    descend_root_block_start,
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
    jaluta_tree,
    descend_root_block_end,
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
    jaluta_tree,
    pessimistic_root_wait_start,
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
    jaluta_tree,
    pessimistic_root_wait_end,
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
    jaluta_tree,
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
    jaluta_tree,
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


TRACEPOINT_EVENT(
    jaluta_tree,
    descending_start,
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
    jaluta_tree,
    descending_end,
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
    jaluta_tree,
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
    jaluta_tree,
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
    jaluta_tree,
    link_start,
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
    jaluta_tree,
    link_end,
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
    jaluta_tree,
    update_traverse_start,
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
    jaluta_tree,
    update_traverse_end,
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
    jaluta_tree,
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
    jaluta_tree,
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
    jaluta_tree,
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
    jaluta_tree,
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

#endif /* _BLINK_TP_H */

#include <lttng/tracepoint-event.h>
