# Word Frequency Counter — Naive vs Multithreading vs Multiprocessing (C)

## Overview
This repository contains **three C implementations** that compute **word frequencies** from a large text file and print the **Top 10 most frequent words** along with execution time:

1. **Naive (single-process)** — simple dynamic array + linear search for each word.
2. **Multithreading (POSIX Threads)** — splits the input into chunks and uses multiple threads.
3. **Multiprocessing (fork + shared memory)** — splits work across multiple processes and merges results using shared memory + a semaphore.

These programs were created to compare **performance** and **parallelization approaches** in C.

---

## Programs
- `Naive.c`  
  - Reads the file, counts word frequencies, sorts by frequency (qsort), prints Top 10, prints timing.

- `Multithreading.c`  
  - Prompts for number of threads.
  - Counts frequencies using pthreads.
  - Sorts and prints Top 10 + timing.

- `multiprocessing.c`  
  - Prompts for number of processes.
  - Uses `fork()` + `mmap()` shared memory and a named semaphore to merge results safely.
  - Sorts and prints Top 10 + timing.

---

## Input File (IMPORTANT)
All three programs currently open a **hardcoded file path** like:
```c
fopen("/home/aws-hammad/Downloads/text8.txt", "r");
```

### Recommended fix (make it portable)
1. Put your dataset file in the same folder as the code (example: `text8.txt`)
2. Replace the hardcoded path with a relative path:
```c
FILE *in = fopen("text8.txt", "r");
```

> If you don’t want to edit code, just change the path string to match your own file location.

---

## How to Compile

### Linux / macOS (gcc)
```bash
gcc Naive.c -o naive
gcc Multithreading.c -o multithreading -pthread
gcc multiprocessing.c -o multiprocessing -pthread
```

> On some systems, `multiprocessing.c` may need `-lrt` as well:
```bash
gcc multiprocessing.c -o multiprocessing -pthread -lrt
```

### Windows
These programs use POSIX APIs (`pthread`, `fork`, `mmap`, `semaphore`), so they are intended for:
- Linux
- macOS (threads OK; `fork/mmap/sem_open` generally OK)
- WSL (Windows Subsystem for Linux)

---

## How to Run

### Naive
```bash
./naive
```

### Multithreading
```bash
./multithreading
```
Then enter the number of threads when prompted.

### Multiprocessing
```bash
./multiprocessing
```
Then enter the number of processes when prompted.

---

## Output (Typical)
Each program prints:
- The Top 10 most frequent words:
  ```
  Top 10 most frequent words:
  1) the: 1061396
  2) of: 593677
  ...
  ```
- Execution time in seconds.

---

## Notes
- The multithreading implementation merges results into shared arrays; in a production-grade implementation you would protect shared updates with a mutex.
- The multiprocessing implementation uses a named semaphore and shared memory to coordinate merging.

---

## Files
- `Naive.c`
- `Multithreading.c`
- `multiprocessing.c`
- `1221697.pdf` (report/documentation, if included)

---

## Author
- Aws Hammad (1221697)
