

SET(BLINK_SRC_FILES
  jaluta_unittest.cc
  blink/blink_tree.cpp
  blink/blink_node.cpp
  blink/node.cpp
  ../../util/one_test.cpp
  ../../util/random_key_generator.cpp
  )


INCLUDE(${CMAKE_SOURCE_DIR}/storage/innobase/innodb.cmake)

ADD_EXECUTABLE(jaluta-tree-t ${BLINK_SRC_FILES})
ADD_LIBRARY(lttng_jaluta_tracepoints STATIC util/blink-tp.cpp)

TARGET_LINK_LIBRARIES(jaluta-tree-t ${GTEST_LIBRARIES} gunit_large server_unittest_library  -Wl,--no-as-needed -ldl ${LTTNG_UST_LIBRARY})
target_link_libraries(jaluta-tree-t lttng_jaluta_tracepoints)

ADD_DEPENDENCIES(jaluta-tree-t GenError)

# These executables get their main() from gunit_small, and have stack trace.
SET_TARGET_PROPERTIES(jaluta-tree-t PROPERTIES ENABLE_EXPORTS TRUE)

# Don't run these by default, run manually
# ADD_TEST(lot0lot  lot0lot-t)
# ADD_TEST(ut0plist ut0plist-t)
# ADD_TEST(ut0frags ut0frags-t)
# ADD_TEST(zlob0int zlob0int-t)
