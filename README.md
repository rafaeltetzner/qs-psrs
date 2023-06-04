# qs-psrs

This is an implementation of the "Parallel Sorting by Regular Sampling" (PSRS) algorithm in C, created as a part of a college assignment for the subject SSC0903 - High Performance Computing.

## Overview
The PSRS algorithm is designed to efficiently sort large arrays in parallel using multiple threads. It follows a four-phase approach to divide, sort, and merge the array elements among the threads, resulting in a sorted array.

## Usage
To use this program, follow the instructions below:

1. Clone the repository:
   ```shell
   git clone <repository_url>
   ```

2. Compile the program using make:
   ```shell
    make
   ```

3. Run the program with the desired input parameters:
   ```shell
   ./qs-psrs <n> <nthreads>
   ```
   - `<n>`: The size of the array to be sorted.
   - `<nthreads>`: The number of threads to be used for parallel execution.

4. The program will generate a sorted array as the output.

## Algorithm Details
The PSRS algorithm consists of four phases:

1. **Phase I**:  Each thread is assigned a portion of the array and sorts it locally using the quicksort algorithm. Additionally, each thread selects specific elements as samples to be used in the later phases.

2. **Phase II**: The master thread merges and sorts the selected samples from all the threads. The samples are sorted using a sequential quicksort, and pivot elements are selected from the sorted samples.

3. **Phase III**: Each thread counts the number of elements that belong to each partition based on the selected pivots. This is done by comparing the elements in the thread's portion with the pivots and updating the count for each partition.

4. **Phase IV**: Each thread merges its assigned partitions by tracking the partition sizes and offsets. Finally, the sorted subarrays are merged into a single sorted array.
