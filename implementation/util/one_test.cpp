#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <sys/time.h>
#include <numeric>
#include <memory>


#include "sync0rw.h"
#include "abstract_tree.h"
#include "random_key_generator.h"



static void show_statistic(long long before, std::vector<long long>& after) {

  long long max_time = *std::max_element(after.begin(), after.end());
  long long min_time = *std::min_element(after.begin(), after.end());
  float test_time_sum = std::accumulate(after.begin(), after.end(), 0.0) - after.size() * before;

  printf("\033[33mmax time: %.4f  s\033[0m\n", (float)(max_time - before) / 1000);
  printf("\033[33mmin time: %.4f  s\033[0m\n", (float)(min_time - before) / 1000);
  printf("\033[32mavg time: %.4f  s\033[0m\n", test_time_sum / 1000 / after.size());
}


static long long mstime()
{
  struct timeval tv;
  long long ust;

  gettimeofday(&tv, NULL);
  ust = ((long long)tv.tv_sec)*1000000;
  ust += tv.tv_usec;
  return ust / 1000;
}

struct statistic_counters {
  std::atomic_long successfull_inserts;
  std::atomic_long inserts_existed_values;
  std::atomic_long successfull_reads;
  std::atomic_long not_found_reads;
  std::shared_ptr<abstract_tree> tree;

  statistic_counters() : successfull_inserts(0), inserts_existed_values(0), successfull_reads(0), not_found_reads(0) {}
};

struct thread_arg {
  int id;
  std::shared_ptr<abstract_tree> tree;
  int  key_size;
  long keys;
  int  is_put;
  std::atomic_bool* stop_flag;
  long long* time;
#ifdef BLINK_STATISTIC
  struct statistic_counters* stat;
#endif
  bool stat_enabled;
};



static void* run(void *arg)
{
  struct thread_arg *ta = (struct thread_arg *)arg;
  ut_a(ta->key_size > 0);

  std::unique_ptr<random_util::random_key_generator> generator = random_util::get_key_generator(ta->key_size, rand());
  uint64_t *key; 

  switch (ta->is_put) {
    case 1:
      for (long i = 0; i < ta->keys; ++i) {
        key = generator->next_write();
        #ifdef BLINK_STATISTIC
          if (ta->stat_enabled) {
            switch(ta->tree->write(key, ta->key_size, (void *)3190)) {
              case 1:
                ta->stat->successfull_inserts.fetch_add(1);
                break;
              case 0:
                ta->stat->inserts_existed_values.fetch_add(1);
                break;
            }
          } else {
            ut_a(ta->tree->write(key, ta->key_size, (void *)3190) >= 0);
          }
        #else
          ut_a(ta->tree->write(key, ta->key_size, (void *)3190) >= 0);
        #endif
      } 
      break;
    case 0: {
      uint64_t val;
      long reads_amount = 0;
      while(!ta->stop_flag->load() and reads_amount < ta->keys) {
        reads_amount++;
        key = generator->next_read();
        #ifdef BLINK_STATISTIC
          if (ta->stat_enabled) {
            switch(ta->tree->read(key, ta->key_size, (void **)&val)) {
              case 1:
                ta->stat->successfull_reads.fetch_add(1);
                break;
              case 0:
                ta->stat->not_found_reads.fetch_add(1);
                break;
            }
          } else {
            ut_a(ta->tree->read(key, ta->key_size, (void **)&val) >= 0);
          }
        #else
          ut_a(ta->tree->read(key, ta->key_size, (void **)&val) >= 0);
        #endif
      }
      break;
    }
    default:
      ut_a(0);
  }

  *ta->time = mstime();
  return (void *)ta;
}

#ifdef BLINK_STATISTIC
  static void* print_statistic(void *arg) {
    struct statistic_counters *statistic = (struct statistic_counters *)arg;

    long inserted_prev = 0, repeated_prev = 0, tree_size = 1,
    read_prev = 0, not_found_read_prev = 0;

    const char* statistic_interval = std::getenv("STATISTIC_INTERVAL");
    
    int statistic_interval_ms = 1000;
    if (statistic_interval) {
      statistic_interval_ms = std::stoi(statistic_interval);
    }

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(statistic_interval_ms));

      long inserted_now  = statistic->successfull_inserts.load();
      long repeated_now  = statistic->inserts_existed_values.load();
      long tree_size_now = statistic->tree->size.load();
      long leafs_now = statistic->tree->leafs_amount.load();
      long read_now = statistic->successfull_reads.load();
      long read_not_found_now = statistic->not_found_reads.load();

      std::cout << "inserted: " << inserted_now - inserted_prev
        << " repeated: " << repeated_now - repeated_prev
          << " splits: " << tree_size_now - tree_size
            << std:: endl 
              << " reads: " << read_now - read_prev 
                << " not found reads " << read_not_found_now - not_found_read_prev
                  << std::endl
                    << "tree size: " << tree_size_now 
                      << " leafs: " << leafs_now
                        << " key/leafnode: " << (float)inserted_now / leafs_now
                          << " tree hight: " << statistic->tree->hight.load()
                            << " values: " << inserted_now
                              << std::endl;
      inserted_prev       = inserted_now;
      repeated_prev       = repeated_now;
      read_prev           = read_now;
      not_found_read_prev = read_not_found_now;
      tree_size           = tree_size_now;
    }

    return arg;
  }
#endif

void bench_function(long start_tree_size, long total_keys, int test_cases, int thread_number_array[], bool recreate_tree_between_tests,
  std::function<abstract_tree*()> constructor) {
  struct thread_arg ta{};
  long current_tree_size = 0;
  std::srand(std::time(nullptr));

  if (!recreate_tree_between_tests) {
    ta.tree = std::shared_ptr<abstract_tree>(constructor());
  }

  ta.stat_enabled = false;
  std::atomic_bool read_stop(false);
  ta.stop_flag = &read_stop;

  for (int test_case = 0; test_case < test_cases; ++test_case, ++thread_number_array) {
    int thread_number = *thread_number_array;

    std::vector<pthread_t> ids(thread_number);
    std::vector<long long> tests_end_time(thread_number);

    if (recreate_tree_between_tests) {
      ta.tree = std::shared_ptr<abstract_tree>(constructor());
      current_tree_size = start_tree_size;

      const int threads_for_setup = 64;
      std::vector<long long> time_setup(threads_for_setup);
      pthread_t setup_ids[threads_for_setup];
      printf("-- setting up --\n");
      if (start_tree_size > 0) {
        for (int i = 0; i < threads_for_setup; ++i) {
          struct thread_arg *t = new struct thread_arg();
          *t = ta;
          t->is_put = 1;
          t->keys = start_tree_size / threads_for_setup;
          t->id = i + 1;
          t->key_size = 8;
          t->time = &time_setup[i];
          pthread_create(&setup_ids[i], 0, run, (void *)t);
        }

        for (int i = 0; i < threads_for_setup; ++i) {
          struct thread_arg *t;
          pthread_join(setup_ids[i], (void **)&t);
          delete(t);
        }
      }
    }
   

    long keys_per_thread = total_keys / thread_number;
    ta.keys = keys_per_thread;
    long long before = mstime();

    printf("-- read start -- threads: %i tree size: %ld total keys: %ld\n",
      thread_number, start_tree_size, total_keys);
    before = mstime();

    for (int i = 0; i < thread_number; ++i) {
      struct thread_arg *t = new struct thread_arg();
      *t = ta;
      t->is_put = 0;
      t->id = i + 1;
      t->key_size = 8;
      t->time = &tests_end_time[i];
      pthread_create(&ids[i], 0, run, (void *)t);
    }

    for (int i = 0; i < thread_number; ++i) {
      struct thread_arg *t;
      pthread_join(ids[i], (void **)&t);
      free(t);
    }

    show_statistic(before, tests_end_time);
    printf("-- write start -- threads: %i tree size: %ld total keys: %ld\n",
      thread_number, start_tree_size, total_keys);
    before = mstime();
  

    for (int i = 0; i < thread_number; ++i) {
      struct thread_arg *t = new struct thread_arg();
      *t = ta;
      t->is_put = 1;
      t->id = i + 1;
      t->key_size = 8;
      t->time = &tests_end_time[i];
      pthread_create(&ids[i], 0, run, (void *)t);
    }

    for (int i = 0; i < thread_number; ++i) {
      struct thread_arg *t;
      pthread_join(ids[i], (void **)&t);
      free(t);
    }
    show_statistic(before, tests_end_time);
    current_tree_size += total_keys;


    printf("-- mixed start -- threads: %i tree size: %ld total keys: %ld\n",
      thread_number, start_tree_size + total_keys, total_keys);
    before = mstime();

    int i;
    for (i = 0; i < thread_number / 2; ++i) {
      struct thread_arg *t = new thread_arg();
      *t = ta;
      t->is_put = 1;
      t->id = i + 1;
      t->key_size = 8;
      t->time = &tests_end_time[i];
      pthread_create(&ids[i], 0, run, (void *)t);
    }

    for (; i < thread_number; ++i) {
      struct thread_arg *t = new thread_arg();
      *t = ta;
      t->is_put = 0;
      t->id = i + 1;
      t->key_size = 8;
      t->time = &tests_end_time[i];
      pthread_create(&ids[i], 0, run, (void *)t);
    }

    for (i = 0; i < thread_number; ++i) {
      struct thread_arg *t;
      pthread_join(ids[i], (void **)&t);
      free(t);
    }
    current_tree_size += total_keys;

    show_statistic(before, tests_end_time);

    // delete(ta.tree.bt);
  }
}


void bench_function(long start_tree_size, long total_keys, int test_cases, int thread_number_array[], bool recreate_tree_between_tests,
  std::function<abstract_tree*(uint32_t)> constructor, uint32_t node_size)
{
  struct thread_arg ta{};
  long current_tree_size = 0;

  std::srand(std::time(nullptr));

  if (!recreate_tree_between_tests) {
    ta.tree = std::shared_ptr<abstract_tree>(constructor(node_size));
  }

  ta.stat_enabled = false;
  std::atomic_bool read_stop(false);
  ta.stop_flag = &read_stop;
  
  for (int test_case = 0; test_case < test_cases; ++test_case, ++thread_number_array) {
    int thread_number = *thread_number_array;

    std::vector<pthread_t> ids(thread_number);
    std::vector<long long> tests_end_time(thread_number);

    if (recreate_tree_between_tests) {
      ta.tree = std::shared_ptr<abstract_tree>(constructor(node_size));
      current_tree_size = start_tree_size;

      const int threads_for_setup = 64;
      std::vector<long long> time_setup(threads_for_setup);
      pthread_t setup_ids[threads_for_setup];
      printf("-- setting up --\n");
      if (start_tree_size > 0) {
        for (int i = 0; i < threads_for_setup; ++i) {
          struct thread_arg *t = new struct thread_arg();
          *t = ta;
          t->is_put = 1;
          t->keys = start_tree_size / threads_for_setup;
          t->id = i + 1;
          t->key_size = 8;
          t->time = &time_setup[i];
          pthread_create(&setup_ids[i], 0, run, (void *)t);
        }

        for (int i = 0; i < threads_for_setup; ++i) {
          struct thread_arg *t;
          pthread_join(setup_ids[i], (void **)&t);
          delete(t);
        }
      }
    }
   

    long keys_per_thread = total_keys / thread_number;
    ta.keys = keys_per_thread;
    long long before = mstime();

    printf("-- read start -- threads: %i tree size: %ld total keys: %ld\n",
      thread_number, start_tree_size, total_keys);
    before = mstime();

    for (int i = 0; i < thread_number; ++i) {
      struct thread_arg *t = new struct thread_arg();
      *t = ta;
      t->is_put = 0;
      t->id = i + 1;
      t->key_size = 8;
      t->time = &tests_end_time[i];
      pthread_create(&ids[i], 0, run, (void *)t);
    }

    for (int i = 0; i < thread_number; ++i) {
      struct thread_arg *t;
      pthread_join(ids[i], (void **)&t);
      free(t);
    }

    show_statistic(before, tests_end_time);
    printf("-- write start -- threads: %i tree size: %ld total keys: %ld\n",
      thread_number, start_tree_size, total_keys);
    before = mstime();
  

    for (int i = 0; i < thread_number; ++i) {
      struct thread_arg *t = new struct thread_arg();
      *t = ta;
      t->is_put = 1;
      t->id = i + 1;
      t->key_size = 8;
      t->time = &tests_end_time[i];
      pthread_create(&ids[i], 0, run, (void *)t);
    }

    for (int i = 0; i < thread_number; ++i) {
      struct thread_arg *t;
      pthread_join(ids[i], (void **)&t);
      free(t);
    }
    show_statistic(before, tests_end_time);
    current_tree_size += total_keys;


    printf("-- mixed start -- threads: %i tree size: %ld total keys: %ld\n",
      thread_number, start_tree_size + total_keys, total_keys);
    before = mstime();

    int i;
    for (i = 0; i < thread_number / 2; ++i) {
      struct thread_arg *t = new thread_arg();
      *t = ta;
      t->is_put = 1;
      t->id = i + 1;
      t->key_size = 8;
      t->time = &tests_end_time[i];
      pthread_create(&ids[i], 0, run, (void *)t);
    }

    for (; i < thread_number; ++i) {
      struct thread_arg *t = new thread_arg();
      *t = ta;
      t->is_put = 0;
      t->id = i + 1;
      t->key_size = 8;
      t->time = &tests_end_time[i];
      pthread_create(&ids[i], 0, run, (void *)t);
    }

    for (i = 0; i < thread_number; ++i) {
      struct thread_arg *t;
      pthread_join(ids[i], (void **)&t);
      free(t);
    }
    current_tree_size += total_keys;

    show_statistic(before, tests_end_time);

    // delete(ta.tree.bt);
  }
}

void configurable_bench_function(long start_tree_size, long total_keys, int write_threads, int key_size, int read_threads,
    std::function<abstract_tree*(uint32_t)> constructor, uint32_t node_size) {

  struct thread_arg ta{};
  ta.tree = std::shared_ptr<abstract_tree>(constructor(node_size));
  std::srand(std::time(nullptr));

  #ifdef BLINK_STATISTIC
    ta.stat = new statistic_counters();
    ta.stat->tree = ta.tree;
    ta.stat_enabled = true;
  #endif

  std::vector<pthread_t> ids(write_threads + read_threads);
  std::vector<long long> tests_end_time(write_threads + read_threads);

  const int threads_for_setup = 32;
  std::vector<long long> time_setup(threads_for_setup);
  pthread_t setup_ids[threads_for_setup];
  printf("-- setting up --\n");
  if (start_tree_size > 0) {
    for (int i = 0; i < threads_for_setup; ++i) {
      struct thread_arg *t = new struct thread_arg();
      *t = ta;
      t->is_put = 1;
      t->keys = start_tree_size / threads_for_setup + 1;
      t->id = i + 1;
      t->key_size = key_size;
      t->time = &time_setup[i];
      pthread_create(&setup_ids[i], 0, run, (void *)t);
    }

    for (int i = 0; i < threads_for_setup; ++i) {
      struct thread_arg *t;
      pthread_join(setup_ids[i], (void **)&t);
      delete(t);
    }
  }
  
  if (write_threads) {
    ta.keys = total_keys / write_threads;
  } else {
    ta.keys = total_keys / read_threads;
  }

  if (std::getenv("LTTNG_TRACE")) {
    printf("-- starting lttng --\n");
    system("lttng start");
  }

  printf("-- start workload --\n");
  std::atomic_bool stop_reads(false);

  for (int i = 0; i < write_threads; ++i) {
    struct thread_arg *t = new thread_arg();
    *t = ta;
    t->is_put = 1;
    t->id = i + 1;
    t->key_size = key_size;
    t->time = &tests_end_time[i];
    pthread_create(&ids[i], 0, run, (void *)t);
  }

  for (int i = write_threads; i < read_threads + write_threads; ++i) {
    struct thread_arg *t = new thread_arg();
    *t = ta;
    t->is_put = 0;
    t->id = i + 1;
    t->key_size = key_size;
    t->time = &tests_end_time[i];
    t->stop_flag = &stop_reads;
    pthread_create(&ids[i], 0, run, (void *)t);
  }

  #ifdef BLINK_STATISTIC
    pthread_t stat_thread;
    pthread_create(&stat_thread, 0, print_statistic, (void *)ta.stat);
  #endif

  for (int i = 0; i < write_threads; ++i) {
    struct thread_arg *t;
    pthread_join(ids[i], (void **)&t);
    free(t);
  }

  stop_reads.store(true);

   for (int i = write_threads; i < read_threads + write_threads; ++i) {
    struct thread_arg *t;
    pthread_join(ids[i], (void **)&t);
    free(t);
  }

  #ifdef BLINK_STATISTIC
    pthread_cancel(stat_thread);
    delete(ta.stat);
  #endif
}

