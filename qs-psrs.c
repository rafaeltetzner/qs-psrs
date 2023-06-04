#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <string.h>

#define SEED 42

#define MIN(a,b) (((a)<(b))?(a):(b))


/*==================================================================*/
/*==================================================================*/
/*                      Functions for Array                         */

// Generate an array of random integers
int* gen_arr(int n) {
    int* arr = (int*) malloc(n * sizeof *arr);
    for(int i = 0; i < n; i++)
        arr[i] = rand();
    return arr;
}

// Print an array
void print_arr(int* arr, int start, int end) {
    for(int i = start; i < end; i++)
        printf("%d, ", arr[i]);
    printf("%d.\n", arr[end]);
}


/*==================================================================*/
/*==================================================================*/
/*                       Sequential QuickSort                       */

// Swap two elements
void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

// Partition function for QuickSort
int partition(int* arr, int low, int high) {
    int pivot = arr[high];
    int i = low-1;
    for(int j = low; j < high; j++) {
        if(arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i+1], &arr[high]);
    return i+1;
}

// Sequential QuickSort implementation
void quicksort(int* arr, int low, int high) {
    if(low < high) {
        int pivotIdx = partition(arr, low, high);
        quicksort(arr, low, pivotIdx-1);
        quicksort(arr, pivotIdx+1, high);
    }
}

/*==================================================================*/
/*==================================================================*/
/*                       Sequential MergeSort                       */

// Merge two sorted subarrays
void merge(int arr[], int low, int mid, int high) {
    int n1 = mid - low + 1;
    int n2 = high - mid;

    int left[n1], right[n2];
    for (int i = 0; i < n1; i++)
        left[i] = arr[low + i];
    for (int j = 0; j < n2; j++)
        right[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = low;
    while (i < n1 && j < n2) {
        if (left[i] <= right[j])
            arr[k++] = left[i++];
        else
            arr[k++] = right[j++];
    }

    while (i < n1)
        arr[k++] = left[i++];
    while (j < n2)
        arr[k++] = right[j++];
}

// Sequential MergeSort implementation
void mergeSort(int arr[], int low, int high) {
    if (low < high) {
        int mid = low + (high - low) / 2;

        mergeSort(arr, low, mid);

        mergeSort(arr, mid + 1, high);

        merge(arr, low, mid, high);
    }
}

/*==================================================================*/
/*==================================================================*/
/*                Parallel Sorting by Regular Sampling              */

// Struct to hold thread-specific context information
struct thread_context {
    int* arr;   // Input array
    int n;      // Size of the input array
    int p;      // Total number of threads
    int id;     // Thread ID
    int low;    // Low index of the subarray assigned to the thread
    int high;   // High index of the subarray assigned to the thread
};

// Get the context for a thread
struct thread_context get_context(int* arr, int n, int p) {
    struct thread_context ctx;
    int div = n/p;
    int rem = n%p;
    ctx.arr = arr;
    ctx.n = n;
    ctx.p = p;
    ctx.id = omp_get_thread_num();
    ctx.low = ctx.id * div + MIN(ctx.id, rem);
    ctx.high= (ctx.id+1) * div + MIN(ctx.id+1, rem) - 1;
    return ctx;
}

// Phase I: Each thread sorts its assigned subarray and selects samples
void phaseI(struct thread_context t, int* samples) {
    quicksort(t.arr, t.low, t.high);
    int offset = t.p * t.id;
    for(int idx = 0; idx < t.p; idx++) {
        samples[offset + idx] = t.arr[t.low + idx * t.n / (t.p * t.p)];
    }

    #pragma omp barrier
}

// Phase II: The master thread sorts the samples and selects pivots
void phaseII(int p, int* samples, int* pivots) {
    #pragma omp single
    {
        quicksort(samples, 0, p * p - 1);
        for(int i = 1; i < p; i++)
            pivots[i-1] = samples[i*p + p/2 -1];
    }
}

// Phase III: Each thread counts the number of elements that belong to each partition
void phaseIII(struct thread_context t, int* pivots, int* partitions) {
    int j = t.low;
    for (int i = 0; i < t.p - 1; i++) {
        while (j <= t.high && t.arr[j] <= pivots[i]) {
            partitions[t.id * t.p + i]++;
            j++;
        }
    }
    while (j <= t.high) {
        partitions[t.id * t.p + t.p - 1]++;
        j++;
    }
    #pragma omp barrier
}

// Phase IV: Each thread merges its assigned partitions and performs final sorting
void phaseIV(struct thread_context t, int* partitions, int* res) {
    int k = 0;
    for(int i = 0; i < t.id; i++) {
        for(int j = 0; j < t.p; j++) {
            int ptIdx = i + j * t.p;
            k += partitions[ptIdx];
        }
    }

    int start = k;

    int offset = 0;
    int offsetIdx = 0;
    for(int i = 0; i < t.p; i++) {
        int ptIdx = t.id + i * t.p;
        while(offsetIdx < ptIdx)
            offset += partitions[offsetIdx++];
        for(int j = offset; j < offset + partitions[ptIdx]; j++)
            res[k++] = t.arr[j];
    }

    mergeSort(res, start, k-1);
    #pragma omp barrier
}

// Main function for parallel sorting using PSRS algorithm
void qs_psrs(int** arr, int n, int p) {
    int* samples = (int*) calloc(p*p, sizeof(int));
    int* pivots  = (int*) calloc(p-1, sizeof(int));
    int* partitions = (int*) calloc(p*p, sizeof(int));
    int* res = (int*) calloc(n, sizeof(int));

    #pragma omp parallel shared(arr, n, p, samples, pivots, partitions, res) num_threads(p)
    {
        struct thread_context ctx = get_context(*arr, n, p);

        phaseI(ctx, samples);
        phaseII(p, samples, pivots);
        phaseIII(ctx, pivots, partitions);
        phaseIV(ctx, partitions, res);
    }

    free(samples);
    free(pivots);
    free(partitions);
    free(*arr);
    *arr = res;
}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        printf("Incorrect number of arguments, usage:\n\t./qs-psrs <n> <nthreads>\n");
        return EXIT_FAILURE;
    }

    srand(SEED);

    int n = atoi(argv[1]);
    int nthreads = atoi(argv[2]);
    
    int* arr = gen_arr(n);

    qs_psrs(&arr, n, nthreads);

    print_arr(arr, 0, n-1);

    free(arr);

    return EXIT_SUCCESS;
}