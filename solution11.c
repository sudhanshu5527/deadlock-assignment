/*
 * copyright 2019 Ashar <ashar786khan@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef VERBOSE_ENABLED
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(...) \
  do {           \
  } while (false);
#endif

/*
 * This is a wrapper that will hold all the information about the current system
 * state
 *
 */
struct system_state {
  int resource_count;
  int process_count;
  int* avail_resource;
  int** allocation_table;
  int** max_table;

} * global_system_state, *global_transient_state;

/*
 * This is a wrapper that holds the request information that is made
 */
struct request {
  int process_id;
  int* resource_requests;

} * request_info;

/*
 * This function free all the dynamically allocated resources.
 */
void free_dynamic_resource() {
  for (int a = 0; a < global_system_state->process_count; a++) {
    free(global_system_state->allocation_table[a]);
    free(global_system_state->max_table[a]);
    free(global_transient_state->max_table[a]);
    free(global_transient_state->allocation_table[a]);
  }
  free(global_system_state->allocation_table);
  free(global_transient_state->allocation_table);
  free(global_system_state->max_table);
  free(global_transient_state->max_table);
  free(global_system_state->avail_resource);
  free(global_transient_state->avail_resource);
  free(global_system_state);
  free(global_transient_state);
}

/**
 * This is function asks for the input of the user and allocates and fills the
 * system state.
 * */
void input() {
  LOG("\nStarted input function...");

  global_system_state =
      (struct system_state*)malloc(sizeof(struct system_state));

  global_transient_state =
      (struct system_state*)malloc(sizeof(struct system_state));

  LOG("\nAllocated global_system_state variable...");

  printf("Enter Total Process Count : ");
  scanf("%d", &(global_system_state->process_count));
  global_transient_state->process_count = global_system_state->process_count;
  printf("\nEnter Total Resource Count : ");
  scanf("%d", &(global_system_state->resource_count));
  global_transient_state->resource_count = global_system_state->resource_count;
  LOG("\nRead Process and resource counts...");
  printf(
      "\nEnter Allocated Resource Count as Table (Process in rows, and "
      "Resource in columns) : \n");
  LOG("\nAllocating memory for allocation table...");

  global_system_state->allocation_table =
      (int**)malloc(global_system_state->process_count * sizeof(int*));

  global_transient_state->allocation_table =
      (int**)malloc(global_transient_state->process_count * sizeof(int*));

  for (int a = 0; a < global_system_state->process_count; a++) {
    global_system_state->allocation_table[a] =
        (int*)malloc(global_system_state->resource_count * sizeof(int));
    global_transient_state->allocation_table[a] =
        (int*)malloc(global_transient_state->resource_count * sizeof(int));
  }

  LOG("\nAllocated allocation table memory now reading...");
  // Read the allocation table
  for (int a = 0; a < global_system_state->process_count; a++)
    for (int b = 0; b < global_system_state->resource_count; b++) {
      scanf("%d", &(global_system_state->allocation_table[a][b]));
      global_transient_state->allocation_table[a][b] =
          global_system_state->allocation_table[a][b];
    }
  LOG("\nMax Table Allocating memory...");
  // Allocate memory for Max resource table

  global_system_state->max_table =
      (int**)malloc(global_system_state->process_count * sizeof(int*));
  global_transient_state->max_table =
      (int**)malloc(global_transient_state->process_count * sizeof(int*));

  for (int a = 0; a < global_system_state->process_count; a++) {
    global_system_state->max_table[a] =
        (int*)malloc(global_system_state->resource_count * sizeof(int));
    global_transient_state->max_table[a] =
        (int*)malloc(global_transient_state->resource_count * sizeof(int));
  }

  LOG("\nAllocated max table memory now reading...");
  printf(
      "\nEnter Maximum Resource Count Limit as Table (Process in rows, and "
      "Resource limit in columns) : \n");

  for (int a = 0; a < global_system_state->process_count; a++)
    for (int b = 0; b < global_system_state->resource_count; b++) {
      scanf("%d", &(global_system_state->max_table[a][b]));
      global_transient_state->max_table[a][b] =
          global_system_state->max_table[a][b];
    }

  LOG("\nAllocating memory to avail_resources...");
  // Allocate memory for avail resource vector and list;
  global_system_state->avail_resource =
      (int*)malloc(sizeof(int) * global_system_state->resource_count);
  global_transient_state->avail_resource =
      (int*)malloc(sizeof(int) * global_transient_state->resource_count);

  LOG("\nReading values to available_resources...");

  printf(
      "\nEnter available resource count in the same order as above for each "
      "resource : \n");
  for (int a = 0; a < global_system_state->resource_count; a++) {
    scanf("%d", &(global_system_state->avail_resource[a]));
    global_transient_state->avail_resource[a] =
        global_system_state->avail_resource[a];
  }
};

/**
 * This is a function responsible for actually finding the stable state if found
 * it sets the solution_state with the state else sets it to NULL
 * */

/*
These are some useful math functions for easing the solving task
a : available;
b : allocated;
c : required;
*/
bool vec_math_is_allocatable(int* a, int* b, int* c, int len) {
  for (int iter = 0; iter < len; iter++)
    if (a[iter] + b[iter] < c[iter]) return false;
  return true;
}
/*
These are some useful math functions for easing the solving task
a : available;
b : allocated;
c : required;
*/
void vec_math_allocate_and_free(int* a, int* b, int* c, int len) {
  assert(vec_math_is_allocatable(a, b, c, len));
  for (int iter = 0; iter < len; iter++) {
    a[iter] += b[iter];
    b[iter] = 0;
  }
}

/*
 * a : allocated
 * b : requested
 * c : max_limit
 */

bool vec_math_should_grant(int* a, int* b, int* c, int len) {
  for (int t = 0; t < len; t++)
    if (a[t] + b[t] > c[t]) return false;
  return true;
}

/*
 * This function restores the state to old state onces a request has been
 * processed
 */

void restore() {
  LOG("\nRestoring Available Resource State to Global State...");

  for (int a = 0; a < global_system_state->resource_count; a++)
    global_transient_state->avail_resource[a] =
        global_system_state->avail_resource[a];

  LOG("\nResources Allocation Table Restored...");

  for (int a = 0; a < global_system_state->process_count; a++)
    for (int b = 0; b < global_system_state->resource_count; b++)
      global_transient_state->allocation_table[a][b] =
          global_system_state->allocation_table[a][b];
}
/*
 * This function solves the state after the request has been granted and returns
 * true if system is in stable state or else false if deadlock is encountered
 */
bool solve() {
  LOG("\nStarted solving the system...");
  int non_executed = global_system_state->process_count;
  bool dead_lock = false;
  bool has_completed[global_system_state->process_count];

  for (int a = 0; a < global_system_state->process_count; a++)
    has_completed[a] = false;

  while (non_executed) {
    dead_lock = true;
    for (int a = 0; a < global_system_state->process_count; a++) {
      if (has_completed[a]) continue;
      LOG("\nChecking if P%d can be allocated for execution...", a);
      bool res =
          vec_math_is_allocatable(global_transient_state->avail_resource,
                                  global_transient_state->allocation_table[a],
                                  global_transient_state->max_table[a],
                                  global_transient_state->resource_count);
      if (res) {
        has_completed[a] = true;
        non_executed--;
        dead_lock = false;
        LOG("\nAllocating and releasing resources for P%d", a);

        vec_math_allocate_and_free(global_transient_state->avail_resource,
                                   global_transient_state->allocation_table[a],
                                   global_transient_state->max_table[a],
                                   global_transient_state->resource_count);
      } else {
        LOG("\nCannot satisfy needs for P%d. Skipping...", a);
      }
    }
    if (dead_lock) {
      LOG("\nEncountered a deadlock...");
      return false;
    }
  }
  return true;
};

/*
 * This asks the request count from user.
 */

void ask_request_count(int* target) {
  printf("\nHow many number of requests will arrive : ");
  scanf("%d", target);
}
/*
 * This asks the actual request and solves the state and prints if it is safe
 * state or not after request has been granted
 */
void ask_requests(int n) {
  for (int t = 0; t < n; t++) {
    printf("\nRequest %d : ", t + 1);
    int p_c;
    printf("\nEnter Process ID which request resources : ");
    scanf("%d", &p_c);
    p_c--;

    if (p_c > global_system_state->process_count) {
      printf("\nOpps !! No such PID found");
      t--;
      continue;
    }

    printf("\nEnter %d space separated integers each for each resource type : ",
           global_system_state->resource_count);
    request_info = (struct request*)malloc(sizeof(struct request));
    request_info->process_id = p_c;
    request_info->resource_requests =
        (int*)malloc(sizeof(int) * global_system_state->resource_count);

    for (int a = 0; a < global_system_state->resource_count; a++)
      scanf("%d", &(request_info->resource_requests[a]));

    if (vec_math_should_grant(global_system_state->allocation_table[p_c],
                              request_info->resource_requests,
                              global_system_state->max_table[p_c],
                              global_system_state->resource_count)) {
      bool flag = true;
      for (int a = 0; a < global_system_state->resource_count; a++)
        if (global_transient_state->avail_resource[a] <
            request_info->resource_requests[a]) {
          flag = false;
          break;
        }

      if (flag)
        printf("\nGranting the Resources. We have Enough resource to grant.");
      else
        printf(
            "\nPartially Granted Request. Limit increased in the allocation "
            "table");

      for (int a = 0; a < global_system_state->resource_count; a++) {
        if (flag)
          global_transient_state->avail_resource[a] -=
              request_info->resource_requests[a];
        global_transient_state->allocation_table[p_c][a] +=
            request_info->resource_requests[a];
      }

      if (!solve())
        printf(
            "\nDEADLOCK : After Request was Granted the system went into "
            "DEADLOCK");
      else
        printf(
            "\nSystem has a Stable State even after the resource requested was "
            "granted");
    }

    else {
      printf(
          "\nRequest for the resources denied... (Requested more than limit)");
    }

    free(request_info->resource_requests);
    free(request_info);
    restore();
    printf("\n");
  }
}

/**
 * This is main driver program.
 * */
int main() {
  input();
  if (solve())
    printf("\nInitially system is in Safe State");
  else {
    printf("\nPanic Deadlock initially.");
    return 0;
  }
  restore();
  int n;
  ask_request_count(&n);
  ask_requests(n);
  free_dynamic_resource();
  return 0;
}
