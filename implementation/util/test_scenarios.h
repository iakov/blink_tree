// First include (the generated) my_config.h, to get correct platform defines.

#ifndef test_scenarios_h
#define test_scenarios_h


struct my_thread_arg
{
  int id;
  int total_keys;
  std::shared_ptr<test_tree_type> bt;
  std::atomic<int>*  key_counter;
  long long* time;
};

struct my_thread_arg_set
{
  std::mutex* m;
  std::unordered_set<uint>* s;
  std::shared_ptr<test_tree_type> bt;
};


std::mutex global_set_mutex;

void bench_function(long start_tree_size, long total_keys, int test_cases, int thread_number_array[], bool recreate_tree_between_runs,
  std::function<abstract_tree*()> constructor);
void bench_function(long start_tree_size, long total_keys, int test_cases, int thread_number_array[], bool recreate_tree_between_runs,
  std::function<abstract_tree*()> constructor, uint32_t node_size);
void configurable_bench_function(long start_tree_size, long total_keys, int thread_number, int key_size, int read_threads,
  std::function<abstract_tree*(uint32_t)> constructor, uint32_t node_size);

static bool test_general_init() {
  ut_new_boot_safe();
  srv_log_write_events = 64;
  srv_log_flush_events = 64;
  srv_log_recent_written_size = 4 * 4096;
  srv_log_recent_closed_size = 4 * 4096;
  srv_log_wait_for_write_spin_delay = 0;
  srv_log_wait_for_flush_timeout = 100000;
  srv_log_write_max_size = 4096;
  srv_log_writer_spin_delay = 25000;
  srv_log_checkpoint_every = 1000000000;
  srv_log_flusher_spin_delay = 25000;
  srv_log_write_notifier_spin_delay = 0;
  srv_log_flush_notifier_spin_delay = 0;
  srv_log_spin_cpu_abs_lwm = 0;
  srv_log_spin_cpu_pct_hwm = 100;
  srv_log_wait_for_flush_spin_hwm = 1000;
  srv_max_n_threads = 1000;
  srv_n_log_files = 2;
  srv_log_file_size = 1024 * 1024;
  srv_log_buffer_size = 256 * 1024;
  srv_log_write_ahead_size = 4096;
  srv_log_group_home_dir = strdup(".");
  srv_buf_pool_instances = 0;
  log_checksum_algorithm_ptr = log_block_calc_checksum_none;
  srv_n_read_io_threads = 1;
  srv_n_write_io_threads = 1;

  os_event_global_init();
  sync_check_init(srv_max_n_threads);
  recv_sys_var_init();
  os_thread_open();
  ut_crc32_init();
  os_create_block_cache();
  clone_init();

  const size_t max_n_pending_sync_ios = 100;

  if (!os_aio_init(srv_n_read_io_threads, srv_n_write_io_threads,
                   max_n_pending_sync_ios)) {
    std::cerr << "Cannot initialize aio system" << std::endl;
    return (false);
  }

  ut_ad(fil_validate());
  return (true);
}

extern SERVICE_TYPE_NO_CONST(registry) * srv_registry;



static void test_general_close() {
  clone_free();

  undo_spaces_deinit();

  os_aio_free();

  fil_close();

  os_thread_close();

  sync_check_close();

  os_event_global_destroy();

  srv_shutdown_state = SRV_SHUTDOWN_NONE;

  free(srv_log_group_home_dir);
  srv_log_group_home_dir = nullptr;
}


static void* test_run_with_set(void *arg) {
  struct my_thread_arg_set* ta = (struct my_thread_arg_set*)arg;

  while (true) {
    ta->m->lock();
    // printf("size: %lu\n", ta->s->size());
    if (ta->s->empty()) {
      ta->m->unlock();
      return (void *)ta;
    }
    uint key_number = *(ta->s->begin());
    ta->s->erase(ta->s->begin());
    ta->m->unlock();

    char* key = new char[12];
    memset(key, 0, 12);
    sprintf(key, "key%08d", key_number);
    char* value = new char[14];
    sprintf(value, "value%08d", key_number);
    int res;
    if(!(res = ta->bt->write((const void *)key, 12, (const void *)value))) {
      printf("\nwrite error: %i key= %s\n", res, key);
    }
  }

}


static void* test_run(void *arg) {
  struct my_thread_arg* ta = (struct my_thread_arg*)arg;
  int key_number;
  while ((key_number = ta->key_counter->fetch_add(1) - 1) < ta->total_keys) {
    char* key = new char[12];
    memset(key, 0, 12);
    sprintf(key, "key%08d", key_number);
    char* value = new char[14];
    sprintf(value, "value%08d", key_number);
    int res;
    if(!(res = ta->bt->write((const void *)key, 12, (const void *)value))) {
      printf("\nwrite error: %i key= %s\n", res, key);
    }
  }

  return (void *)ta;
}

static std::unordered_set<uint>* generateRandomSet(size_t size, uint maxPossibleValue) {
  std::unordered_set<uint>* result = new std::unordered_set<uint>();
  while (result->size() < size)
  {
    uint new_key = rand() % maxPossibleValue;
    result->insert(new_key);
  }
  
  return result;
}

TEST(bplus, tree_correctness_single_thread_small_nodes) {
  ASSERT_TRUE(test_general_init());

  auto bt = std::make_unique<test_tree_type>(((uint32)1) << 10);
  int keys = 1000;
  for (int i = 0; i < keys; ++i) {
    char* key = new char[102];
    memset(key, 0, 102);
    sprintf(key, "key%08d", i);
    char* value = new char[104];
    sprintf(value, "value%08d", i);
    ut_a(bt->write((const void *)key, 102, (const void *)value) == 1);
  }

  int keyNotFound = 0;
  for (int i = 0; i < keys; ++i) {
    char key[102] = {0};
    sprintf(key, "key%08d", i);
    char expected_value[104] = {0};
    char* actual_value;
    sprintf(expected_value, "value%08d", i);
    if (bt->read((const void *)key, 102, (void **)&actual_value)) {
      ASSERT_TRUE(strcmp(actual_value, expected_value) == 0);
    } else {
      // printf("not found key: %s\n", key);
      std::cout << bt.get();
      std::cout << std::endl << "key: " << key << std::endl;
      exit(-1);
      keyNotFound++;
    }
  }
  if (keyNotFound) {
    printf("\nkeyNotFound=%i\n", keyNotFound);
  }

  test_general_close();
}

TEST(bplus, tree_correctness_single_thread) {
  ASSERT_TRUE(test_general_init());

  auto bt = std::make_unique<test_tree_type>();
  int keys = 500000;
  for (int i = 0; i < keys; ++i) {
    char* key = new char[12];
    memset(key, 0, 12);
    sprintf(key, "key%08d", i);
    char* value = new char[14];
    sprintf(value, "value%08d", i);
    ut_a(bt->write((const void *)key, 12, (const void *)value) == 1);
  }

  int keyNotFound = 0;
  for (int i = 0; i < keys; ++i) {
    char key[12] = {0};
    sprintf(key, "key%08d", i);
    char expected_value[14] = {0};
    char* actual_value;
    sprintf(expected_value, "value%08d", i);
    if (bt->read((const void *)key, 12, (void **)&actual_value)) {
      ASSERT_TRUE(strcmp(actual_value, expected_value) == 0);
    } else {
      // printf("not found key: %s\n", key);
      std::cout << bt.get();
      std::cout << std::endl << "key: " << key << std::endl;
      exit(-1);
      keyNotFound++;
    }
  }
  if (keyNotFound) {
    printf("\nkeyNotFound=%i\n", keyNotFound);
  }

  test_general_close();
}


TEST(bplus, tree_correctness_single_thread_random_keys_small_nodes_big_keys) {
  ASSERT_TRUE(test_general_init());

  int keyAmount = 500000;
  std::unique_ptr<std::unordered_set<uint>> keys = 
    std::make_unique<std::unordered_set<uint>>(*generateRandomSet(keyAmount, keyAmount * 100));
  auto bt = std::make_unique<test_tree_type>(((uint32)1) << 10);

  for (auto i = keys->begin(); i != keys->end(); ++i) {
    char* key = new char[80];
    memset(key, 0, 80);
    sprintf(key, "key%08d", *i);
    char* value = new char[14];
    sprintf(value, "value%08d", *i);
    bt->write((const void *)key, 80, (const void *)value);

  }

  int keyNotFound = 0;
    for (auto i = keys->begin(); i != keys->end(); ++i) {
    char key[80] = {0};
    sprintf(key, "key%08d", *i);
    char expected_value[14] = {0};
    char* actual_value;
    sprintf(expected_value, "value%08d", *i);
    if (bt->read((const void *)key, 80, (void **)&actual_value)) {
      ASSERT_TRUE(strcmp(actual_value, expected_value) == 0);
    } else {
      // printf("not found key: %s\n", key);
      std::cout << bt.get();
      std::cout << std::endl << "key: " << key << std::endl;
      exit(-1);
      keyNotFound++;
    }
  }
  if (keyNotFound) {
    printf("\nkeyNotFound=%i\n", keyNotFound);
  }

  test_general_close();
}


TEST(bplus, tree_correctness_single_thread_random_keys_small_nodes) {
  ASSERT_TRUE(test_general_init());

  int keyAmount = 500000;
  std::unique_ptr<std::unordered_set<uint>> keys = std::make_unique<std::unordered_set<uint>>(*generateRandomSet(keyAmount, keyAmount * 100));
  auto bt = std::make_unique<test_tree_type>(((uint32)1) << 10);

  for (auto i = keys->begin(); i != keys->end(); ++i) {
    char* key = new char[12];
    memset(key, 0, 12);
    sprintf(key, "key%08d", *i);
    char* value = new char[14];
    sprintf(value, "value%08d", *i);
    bt->write((const void *)key, 12, (const void *)value);
  }


  int keyNotFound = 0;
    for (auto i = keys->begin(); i != keys->end(); ++i) {
    char key[12] = {0};
    sprintf(key, "key%08d", *i);
    char expected_value[14] = {0};
    char* actual_value;
    sprintf(expected_value, "value%08d", *i);
    if (bt->read((const void *)key, 12, (void **)&actual_value)) {
      ASSERT_TRUE(strcmp(actual_value, expected_value) == 0);
    } else {
      std::cout << bt.get();
      std::cout << std::endl << "key: " << key << std::endl;
      exit(-1);
      // printf("not found key: %s\n", key);
      keyNotFound++;
    }
  }
  if (keyNotFound) {
    printf("\nkeyNotFound=%i\n", keyNotFound);
  }

  test_general_close();
}




TEST(bplus, tree_correctness_multi_thread) {
  ASSERT_TRUE(test_general_init());

  auto bt = std::make_shared<test_tree_type>();
  int keys = 1000000;

  const int threads_amount = 4;
  pthread_t ids[threads_amount];

  std::atomic<int> key_counter{0};
  auto thread_args = std::make_unique<std::vector<struct my_thread_arg>>(threads_amount);

  for (int i = 0; i < threads_amount; ++i) {
    auto t = &(*thread_args)[i];
    t->total_keys = keys;
    t->bt = bt;
    t->key_counter = &key_counter;
    pthread_create(&ids[i], 0, test_run, (void *)t);
  }

  for (int i = 0; i < threads_amount; ++i) {
    pthread_join(ids[i], nullptr);
  }


  int keyNotFound = 0;
  for (int i = 0; i < keys; ++i) {
    char key[12] = {0};
    sprintf(key, "key%08d", i);
    char expected_value[14] = {0};
    char* actual_value;
    sprintf(expected_value, "value%08d", i);
    bt->read((const void *)key, 12, (void **)&actual_value);
    if (bt->read((const void *)key, 12, (void **)&actual_value)) {
      ASSERT_TRUE(strcmp(actual_value, expected_value) == 0);
    } else {
      std::cout << bt.get();
      std::cout << std::endl << "key: " << key << std::endl;
      exit(-1);
      // printf("not found key: %s\n", key);
      keyNotFound++;
    }
  }

  if (keyNotFound) {
    printf("\nkeyNotFound=%i\n", keyNotFound);
  }


  test_general_close();
}


TEST(bplus, tree_correctness_multi_thread_random_keys) {
  ASSERT_TRUE(test_general_init());

  int keyAmount = 1000000;
  std::unique_ptr<std::unordered_set<uint>> keys = std::make_unique<std::unordered_set<uint>>(*generateRandomSet(keyAmount, keyAmount * 100));
  auto bt = std::make_shared<test_tree_type>();

  const int threads_amount = 16;
  pthread_t ids[threads_amount];
  std::mutex set_mutex; 
  auto thread_args = std::make_unique<std::vector<struct my_thread_arg_set>>(threads_amount);

  for (int i = 0; i < threads_amount; ++i) {
    auto thread_arg = &(*thread_args)[i];
    thread_arg->s = keys.get();
    thread_arg->m = &set_mutex;
    thread_arg->bt = bt;
    pthread_create(&ids[i], 0, test_run_with_set, (void *)thread_arg);
  }

  for (int i = 0; i < threads_amount; ++i) {
    pthread_join(ids[i], nullptr);
  }

  int keyNotFound = 0;
  for (auto i = keys->begin(); i != keys->end(); ++i) {
    char key[12] = {0};
    sprintf(key, "key%08d", *i);
    char expected_value[14] = {0};
    char* actual_value;
    sprintf(expected_value, "value%08d", *i);
    if (bt->read((const void *)key, 12, (void **)&actual_value)) {
      ASSERT_TRUE(strcmp(actual_value, expected_value) == 0);
    } else {
      printf("not found key: %s\n", key);
      keyNotFound++;
    }
  }
  if (keyNotFound) {
    printf("\nkeyNotFound=%i\n", keyNotFound);
  }

  test_general_close();
}


TEST(bplus, tree_correctness_multi_thread_random_keys_small_nodes) {
  ASSERT_TRUE(test_general_init());

  int keyAmount = 1000000;
  std::unique_ptr<std::unordered_set<uint>> keys = 
    std::make_unique<std::unordered_set<uint>>(*generateRandomSet(keyAmount, keyAmount * 100));
  auto bt = std::make_shared<test_tree_type>(((uint32)1) << 10);

  const int threads_amount = 16;
  pthread_t ids[threads_amount];
  std::mutex set_mutex; 
  auto thread_args = std::make_unique<std::vector<struct my_thread_arg_set>>(threads_amount);

  for (int i = 0; i < threads_amount; ++i) {
    auto thread_arg  = &(*thread_args)[i];
    thread_arg->s = keys.get();
    thread_arg->m = &set_mutex;
    thread_arg->bt = bt;
    pthread_create(&ids[i], 0, test_run_with_set, (void *)thread_arg);
  }

  for (int i = 0; i < threads_amount; ++i) {
    pthread_join(ids[i], nullptr);
  }

  int keyNotFound = 0;
    for (auto i = keys->begin(); i != keys->end(); ++i) {
    char key[12] = {0};
    sprintf(key, "key%08d", *i);
    char expected_value[14] = {0};
    char* actual_value;
    sprintf(expected_value, "value%08d", *i);
    if (bt->read((const void *)key, 12, (void **)&actual_value)) {
      ASSERT_TRUE(strcmp(actual_value, expected_value) == 0);
    } else {
      printf("not found key: %s\n", key);
      keyNotFound++;
    }
  }
  if (keyNotFound) {
    printf("\nkeyNotFound=%i\n", keyNotFound);
  }

  test_general_close();
}


std::function<abstract_tree*()> tree_factory = [](){return new test_tree_type();};

TEST(blink, empty_billion_tree_size) {
  ASSERT_TRUE(test_general_init());

  int threads[] = {1, 2, 4, 16, 64};
  bench_function(0, 1000000, 5, threads, true, tree_factory);

  test_general_close();
}

TEST(blink, 1_billion_tree_size) {
  ASSERT_TRUE(test_general_init());

  int threads[] = {1, 2, 4, 16, 64};
  bench_function(1000000, 1000000, 5, threads, true, tree_factory);

  test_general_close();
}

TEST(blink, 10_billion_tree_size) {
  ASSERT_TRUE(test_general_init());

  int threads[] = {1, 2, 4, 16, 64};
  bench_function(10000000, 1000000, 5, threads, true, tree_factory);

  test_general_close();
}


TEST(jaluta, configurable_test) {
  ASSERT_TRUE(test_general_init());
  const char* threads = std::getenv("THREADS_AMOUNT");
  const char* start_tree_size = std::getenv("START_TREE_SIZE");
  const char* key_size = std::getenv("KEY_SIZE");
  const char* read_threads = std::getenv("READ_THREADS");
  const char* keys_amount = std::getenv("KEYS_AMOUNT");
  const char* node_size = std::getenv("NODE_SIZE");

  long keys_to_insert = 100000;
  if (keys_amount) {
    keys_to_insert = std::stol(keys_amount);
  }

  int node_size_bytes_log = 14;
  if (node_size) {
    node_size_bytes_log = std::stoi(node_size);
  }

  if (threads && start_tree_size && key_size && read_threads) {
    std::cout << "Using tree with node size: " << node_size_bytes_log << " inserting " << keys_to_insert << " keys" << std::endl;
    configurable_bench_function(std::stol(start_tree_size), keys_to_insert, std::stoi(threads), std::stoi(key_size), std::stoi(read_threads),
      [](uint32_t x){return new test_tree_type(x);}, (uint32_t(1)) << node_size_bytes_log);
  } else {
    fprintf(stderr, "THREADS_AMOUNT or START_TREE_SIZE or KEY_SIZE or READ_THREADS not set!");
    exit(-1);
  }

  test_general_close();
}

// TEST(bplus, 100_billion_tree_size) {
//   ASSERT_TRUE(test_general_init());

//   int threads[] = {1, 2, 4, 16, 64};
//   bench_function(100000000, 1000000, 5, threads, false);

//   test_general_close();
// }

#endif
