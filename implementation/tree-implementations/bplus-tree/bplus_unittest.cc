

// First include (the generated) my_config.h, to get correct platform defines.


#include "my_config.h"
#include "univ.i"


#include <atomic>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "clone0api.h" /* clone_init(), clone_free() */
#include "dict0dict.h" /* dict_sys_t::s_log_space_first_id */
#include "fil0fil.h"
#include "log0log.h"
#include "log0recv.h"
#include "srv0srv.h"
#include "srv0start.h" /* srv_is_being_started */
#include "ut0byte.h"
#include "ut0new.h"
#include "ut0rnd.h"


#include <mysql/components/minimal_chassis.h> /* minimal_chassis_init() */
#include <mysql/components/service.h>         /* SERVICE_TYPE_NO_CONST */

#include "bplus/bplus_tree.h"

using test_tree_type = bplus_tree;

#include "../../util/test_scenarios.h"




