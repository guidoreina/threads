#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>

#define MIN_THREADS   2
#define MAX_THREADS   32
#define NUMBER_LOOPS  1000000ULL
#define COLUMN_WIDTH  25

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

typedef struct {
  const char* name;
  void* (*fn)(void* arg);
} test_t;

static int64_t counter;

static pthread_mutex_t mutex;
static pthread_spinlock_t spinlock;
static pthread_cond_t cond;
static int ready;

static void run_test(const test_t* test, unsigned long long* durations);
static void* use_sync_add_and_fetch(void* arg);
static void* use_mutex(void* arg);
static void* use_spinlock(void* arg);

int main()
{
  static const test_t tests[] = {
    {"__sync_add_and_fetch", use_sync_add_and_fetch},
    {"mutex",                use_mutex             },
    {"spinlock",             use_spinlock          }
  };

  unsigned long long durations[ARRAY_SIZE(tests)][MAX_THREADS + 1];
  size_t i, j;

  /* Create mutex. */
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    fprintf(stderr, "Error initializing mutex.\n");
    return -1;
  }

  /* Create spin lock. */
  if (pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0) {
    fprintf(stderr, "Error initializing spin lock.\n");

    pthread_mutex_destroy(&mutex);
    return -1;
  }

  /* Create condition variable. */
  if (pthread_cond_init(&cond, NULL) != 0) {
    fprintf(stderr, "Error initializing condition variable.\n");

    pthread_spin_destroy(&spinlock);
    pthread_mutex_destroy(&mutex);

    return -1;
  }

  /* Run tests. */
  for (i = 0; i < ARRAY_SIZE(tests); i++) {
    run_test(tests + i, &durations[i][MIN_THREADS]);
  }

  pthread_cond_destroy(&cond);
  pthread_spin_destroy(&spinlock);
  pthread_mutex_destroy(&mutex);

  printf("%-*s", COLUMN_WIDTH, "# Threads");

  for (i = 0; i < ARRAY_SIZE(tests); i++) {
    printf("%-*s", COLUMN_WIDTH, tests[i].name);
  }

  printf("\n");

  for (i = MIN_THREADS; i <= MAX_THREADS; i++) {
    printf("%-*zu", COLUMN_WIDTH, i);

    for (j = 0; j < ARRAY_SIZE(tests); j++) {
      printf("%-*llu", COLUMN_WIDTH, durations[j][i]);
    }

    printf("\n");
  }

  return 0;
}

void run_test(const test_t* test, unsigned long long* durations)
{
  pthread_t threads[MAX_THREADS];
  struct timeval start, stop;
  unsigned long long duration;
  size_t i, j, k;

  for (i = MIN_THREADS; i <= MAX_THREADS; i++) {
    ready = 0;

    for (j = 0; j < i; j++) {
      if (pthread_create(&threads[j], NULL, test->fn, NULL) != 0) {
        fprintf(stderr, "Error creating thread %zu.\n", j + 1);
        break;
      }
    }

    gettimeofday(&start, NULL);

    /* Notify threads that now they can start their job. */
    pthread_mutex_lock(&mutex);

    ready = 1;
    pthread_cond_broadcast(&cond);

    pthread_mutex_unlock(&mutex);

    /* Wait for threads. */
    for (k = 0; k < j; k++) {
      pthread_join(threads[k], NULL);
    }

    gettimeofday(&stop, NULL);

    stop.tv_sec -= start.tv_sec;
    duration = (stop.tv_sec * 1000000) + stop.tv_usec - start.tv_usec;

    *durations++ = duration;
  }
}

static inline void wait_ready(void)
{
  /* Wait for all the threads to be ready. */
  pthread_mutex_lock(&mutex);

  while (!ready) {
    pthread_cond_wait(&cond, &mutex);
  }

  pthread_mutex_unlock(&mutex);
}

void* use_sync_add_and_fetch(void* arg)
{
  uint64_t i;

  /* Wait for all the threads to be ready. */
  wait_ready();

  for (i = 0; i < NUMBER_LOOPS; i++) {
    __sync_add_and_fetch(&counter, 1);
  }

  return NULL;
}

void* use_mutex(void* arg)
{
  uint64_t i;

  /* Wait for all the threads to be ready. */
  wait_ready();

  for (i = 0; i < NUMBER_LOOPS; i++) {
    pthread_mutex_lock(&mutex);
    counter++;
    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

void* use_spinlock(void* arg)
{
  uint64_t i;

  /* Wait for all the threads to be ready. */
  wait_ready();

  for (i = 0; i < NUMBER_LOOPS; i++) {
    pthread_spin_lock(&spinlock);
    counter++;
    pthread_spin_unlock(&spinlock);
  }

  return NULL;
}
