#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  /* End of "Additional fields here" */
  u32 waiting_time;
  u32 run_time;
  u32 response_time;
  u32 remaining_time;
  bool inq;
  bool executed;

};


TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}



int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size = 0;
  init_processes(argv[1], &data, &size); // initializes the size

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */
  u32 current_time = 0;
  u32 total_processes = 0;
  struct process_list proc_sort;
  TAILQ_INIT(&proc_sort);


  // Initialize an empty linked list for sorted processes
  // struct process_list sorted_list;
  // TAILQ_INIT(&sorted_list);

  // Insert processes into the sorted list in order of arrival time
  // for (u32 i = 0; i < size; ++i) {
  //     struct process *new_process = &data[i];
  //     struct process *iter;
  //     new_process->executed = false;
  //     new_process->waiting_time = 0;
  //     new_process->run_time = 0;
  //     new_process->response_time = 0;
  //     new_process->remaining_time = new_process->burst_time;
  //     total_processes++;

  for (u32 i = 0; i < size; ++i)
  {
    struct process *iter = &data[i];
    iter->inq = false;
    if (iter->arrival_time == 0)
    {
      iter->inq = true;
      iter->remaining_time = iter->burst_time;
      TAILQ_INSERT_TAIL(&list, iter, pointers);
    }
  }  

  // Iterate through the sorted list and print the arrival time of each process
  // struct process *process_ptr;
  // TAILQ_FOREACH(process_ptr, &list, pointers) {
  //     printf("Arrival time: %u\n", process_ptr->arrival_time);
  // }

  // Round Robin Scheduling Logic
  while (!TAILQ_EMPTY(&list)) {
      // printf("Current time: %u\n", current_time);
      // // Debugging chart header
      // printf("-----------------------------------------------------------------\n");
      // printf("| Process ID | Arrival Time | Burst Time | Time Remaining        |\n");
      // printf("-----------------------------------------------------------------\n");

      // // Iterate through the processes in the list and print their details
      // struct process *p;
      // TAILQ_FOREACH(p, &list, pointers) {
      //     printf("| %-11u| %-13u| %-15u| %-18u|\n", p->pid, p->arrival_time, p->burst_time, p->remaining_time);
      // }

      // // Debugging chart footer
      // printf("-----------------------------------------------------------------\n");

      struct process *current_process = TAILQ_FIRST(&list);
      TAILQ_REMOVE(&list, current_process, pointers);

      // Calculate response time for the process
      if (current_time < current_process->arrival_time)
        {
          // Advance the current time to the arrival time of the next process
          current_time = current_process->arrival_time;
        }
      if (current_time >= current_process->arrival_time) // ELSE IF OR JUST IF????? edge case?? for if there's a gap between execute and arrival
      {
        if (!current_process->executed) 
        {
            // Calculate response time
            current_process->response_time = current_time - current_process->arrival_time;
            // Update total response time
            total_response_time += current_process->response_time;
            current_process->executed = true; // Mark the process as executed


            // set the initial waiting time 
            current_process->waiting_time = current_time - current_process->arrival_time;
        }
        else{
            //current_process->waiting_time = current_time - current_process->arrival_time;
        }
      }

      // Execute process for a quantum length or until it finishes
      current_process->remaining_time = current_process->burst_time - current_process->run_time;
      u32 execute_time = (current_process->remaining_time > quantum_length) ? quantum_length :  current_process->remaining_time;
      
      current_process->run_time += execute_time;

      // Update waiting time for current process
      // u32 waiting_time = current_time - current_process->arrival_time;
      // current_process->waiting_time = waiting_time;

      // Update waiting time for all other processes 
      struct process *other_process;
      TAILQ_FOREACH(other_process, &list, pointers) {
          if (other_process != current_process) {
              // Update waiting time
              other_process->waiting_time += execute_time;
          }
      }

      // Update remaining time for the process
      current_process->remaining_time -= execute_time;

      // Increment the current time by either the exec time or the quantum_length
      current_time += execute_time;

      // Check if there are new processes arriving at the current time
      // while (total_processes < size && data[total_processes].arrival_time == current_time) {
      // for (u32 i = 0; i < size; ++i)
      // {
      //   // Insert the new process into the ready queue
      //     struct process *new_process = &data[i];
      //     if (new_process->inq == false && new_process->arrival_time <= current_time)
      //     {
      //       new_process->inq = true;
      //       new_process->waiting_time = 0;
      //       new_process->run_time = 0;
      //       new_process->response_time = 0;
      //       new_process->remaining_time = new_process->burst_time;


      //       TAILQ_INSERT_TAIL(&list, new_process, pointers);
      //       total_processes++;
      //     }
      // }
      // Loop through the processes and add those with the same arrival time to the temporary list
      struct process_list temp_list;
      TAILQ_INIT(&temp_list);

      for (u32 i = 0; i < size; ++i) {
          struct process *new_process = &data[i];
          if (!new_process->inq && new_process->arrival_time <= current_time) {
              new_process->inq = true;
              new_process->waiting_time = 0;
              new_process->run_time = 0;
              new_process->response_time = 0;
              new_process->remaining_time = new_process->burst_time;
              total_processes++;

              // Find the correct position to insert the process based on arrival time
              struct process *iter;
              bool inserted = false;
              TAILQ_FOREACH(iter, &temp_list, pointers) {
                  if (new_process->arrival_time < iter->arrival_time) {
                      // Insert new process before current process in the temp list
                      TAILQ_INSERT_BEFORE(iter, new_process, pointers);
                      inserted = true;
                      break;
                  }
              }
              if (!inserted) {
                  // If the new process is not inserted yet, insert it at the end of the temporary list
                  TAILQ_INSERT_TAIL(&temp_list, new_process, pointers);
              }
          }
      }

      // Merge the temporary list with the main queue while preserving the order
      while (!TAILQ_EMPTY(&temp_list)) {
          struct process *temp_process = TAILQ_FIRST(&temp_list);
          TAILQ_REMOVE(&temp_list, temp_process, pointers);
          
          // Insert the process into the main queue
          TAILQ_INSERT_TAIL(&list, temp_process, pointers);
      }


      // If the process is not completed, insert it back into the ready queue
      if (current_process->remaining_time > 0) {
        TAILQ_INSERT_TAIL(&list, current_process, pointers);
      }
      else
      {
        // Update total waiting time for the process that just executed
        total_waiting_time += current_process->waiting_time; //(current_time - current_process->arrival_time);
      }
  }

  
  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}


  // To debug:
  // print the queue each iteration (sec) by using TAILHEADFOREACH and provide the PID to print it out

  /* End of "Your code here" */

//   printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
//   printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

//   free(data);
//   return 0;
// }
