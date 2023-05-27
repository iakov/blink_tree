
SET(BPLUS_SRC_FILES
  bplus_unittest.cc
  bplus/bplus_tree.cpp
  bplus/node.cpp
  bplus/bplus_node.cpp
  ../../util/one_test.cpp
  ../../util/random_key_generator.cpp
  )
INCLUDE(${CMAKE_SOURCE_DIR}/storage/innobase/innodb.cmake)

ADD_EXECUTABLE(bplus-tree-t ${BPLUS_SRC_FILES})
ADD_LIBRARY(lttng_tracepoints STATIC util/bplus-tp.cpp)

TARGET_LINK_LIBRARIES(bplus-tree-t ${GTEST_LIBRARIES} gunit_large server_unittest_library  -Wl,--no-as-needed -ldl ${LTTNG_UST_LIBRARY})
TARGET_LINK_LIBRARIES(bplus-tree-t lttng_tracepoints)

ADD_DEPENDENCIES(bplus-tree-t GenError)

# These executables get their main() from gunit_small, and have stack trace.
SET_TARGET_PROPERTIES(bplus-tree-t PROPERTIES ENABLE_EXPORTS TRUE)

# Don't run these by default, run manually
# ADD_TEST(lot0lot  lot0lot-t)
# ADD_TEST(ut0plist ut0plist-t)
# ADD_TEST(ut0frags ut0frags-t)
# ADD_TEST(zlob0int zlob0int-t)
