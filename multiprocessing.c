#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/time.h>

struct Word {
    char word[50];// word with max length 50
    int word_freq;// frequency of the word
};

// function declarations
int read_from_file(struct Word **Array);// read words from a file
int frequency(int wordCount, struct Word *Array, struct Word **Array2, int num);// calculate word frequencies using multiprocessing
void sort(int wordCount, struct Word *Array2);// sorts the words by frequency using qsort

int main() {
    struct timeval end, start;// for measuring the time
    double totalTime;// to store total time
    // start measuring time
    gettimeofday(&start, NULL);
    int numOfProcesses;// number of processes
    printf("Aws Hammad - 1221697 \nEnter the number of processes: ");
    scanf("%d", &numOfProcesses);
    struct Word *arr = NULL;// to store words read from the file
    struct Word *arr3 = NULL;// shared array to store the results that came out from the child processes
    // read words from the file
    int wordCount = read_from_file(&arr);
    if (wordCount == 0) {
        // if there is no words exit the program
        printf("No words read from the file. Exiting...\n");
        return 1;
    }
    // calculate word frequencies with multiprocessing
    int lastCount = frequency(wordCount, arr, &arr3, numOfProcesses);
    // sorting the words on frequency
    sort(lastCount, arr3);
    // display the top 10 words
    printf("Top 10 most frequent words:\n");
    for (int j = 0; j < 10 && j < lastCount; j++) {
        printf("%d) %s: %d\n", j + 1, arr3[j].word, arr3[j].word_freq);
    }
    // end time
    gettimeofday(&end, NULL);
    // total time in ((seconds))
    totalTime = (end.tv_sec - start.tv_sec) * 1000000.0;
    totalTime += (end.tv_usec - start.tv_usec);
    printf("\n%f seconds\n", totalTime / 1000000.0);
    return 0;
}

// read words from the file into an array
int read_from_file(struct Word **arr) {
    // open the file
    FILE *file = fopen("/home/aws-hammad/Downloads/text8.txt", "r");
    if (file == NULL) {// the file didn't open
        perror("Failed to open the file");
        return 0;
    }
    int capacity = 1000000;// initial capacity of the array
    int c = 0;// counter for the number of words
    // allocate memory for the array of structs
    *arr = (struct Word *)malloc(capacity * sizeof(struct Word));
    if (*arr == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return 0;
    }
    // read words from the file word word
    while (fscanf(file, "%s", (*arr)[c].word) == 1) {
        (*arr)[c].word_freq = 0; // initialize the frequency to 0
        c++;
        // if the array is full double its capacity
        if (c >= capacity) {
            capacity *= 2;
            *arr = (struct Word *)realloc(*arr, capacity * sizeof(struct Word));
            if (*arr == NULL) {
                perror("Memory reallocation failed");
                fclose(file);
                return 0;
            }
        }
    }
    fclose(file);// close the file
    return c;// return the number of words read
}

// calculates word frequencies using multiple processes and shared memory
int frequency(int wordCount, struct Word *Array, struct Word **Array2, int num) {
    int capacity = 10000000;// the capacity of the shared array
    int pid[num];// an array to store processes IDs
    int chunk_size = wordCount / num;// the number of words per process
    int mod = wordCount % num;// the remaining words to be handled by the last process
    // unlink any existing semaphore
    sem_unlink("/mysemaphore");
    // initialize a semaphore
    sem_t *sem = sem_open("/mysemaphore", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("Semaphore initialization failed");
        exit(1);
    }
    // allocate shared memory for results array
    *Array2 = mmap(NULL, capacity * sizeof(struct Word), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (*Array2 == MAP_FAILED) {
        perror("Shared memory allocation failed for Array2");
        sem_close(sem);
        sem_unlink("/mysemaphore");
        exit(1);
    }
    // allocate shared memory for the count of last words
    int *lastCount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (lastCount == MAP_FAILED) {
        perror("Shared memory allocation failed");
        sem_close(sem);
        sem_unlink("/mysemaphore");
        exit(1);
    }
    *lastCount = 0;// initialize the shared counter to 0
    // create child processes
    for (int r = 0; r < num; r++) {
        pid[r] = fork();// fork a new process
        if (pid[r] == 0) {
            // child process code
            int local_size = chunk_size + (r == num - 1 ? mod : 0);// the remaining words in the last process
            struct Word *local_array = (struct Word *)malloc(local_size * sizeof(struct Word));
            if (local_array == NULL) {
                perror("Memory allocation failed in child");
                exit(1);
            }
            int startIndex = r * chunk_size;// the starting index for this process
            int endIndex = startIndex + local_size;// the ending index for this process
            int localC = 0;// local counter for last words
            // loop through the words
            for (int z = startIndex; z < endIndex; z++) {
                int duplicate = 0;// to check if the word is already counted
                for (int y = 0; y < localC; y++) {
                    if (strcmp(local_array[y].word, Array[z].word) == 0) {
                        // if word is found increment its frequency
                        local_array[y].word_freq++;
                        duplicate = 1;
                        break;
                    }
                }
                if (!duplicate) {
                    // if word is not found add it to the array
                    strcpy(local_array[localC].word, Array[z].word);
                    local_array[localC].word_freq = 1;
                    localC++;
                }
            }
            // merge local results into the shared memory
            sem_wait(sem); // lock the semaphore
            for (int i = 0; i < localC; i++) {
                int found = 0;
                for (int j = 0; j < *lastCount; j++) {
                    if (strcmp((*Array2)[j].word, local_array[i].word) == 0) {
                        // if word already exists in the shared array update its frequency
                        (*Array2)[j].word_freq += local_array[i].word_freq;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    // if word is new add it to the shared array
                    if (*lastCount < capacity) {
                        strcpy((*Array2)[*lastCount].word, local_array[i].word);
                        (*Array2)[*lastCount].word_freq = local_array[i].word_freq;
                        (*lastCount)++;
                    }
                }
            }
            sem_post(sem); // unlock the semaphore
            // free local memory and exit the child process
            free(local_array);
            exit(0);
        }
    }
    // wait for all child processes to finish
    for (int r = 0; r < num; r++) {
        wait(NULL);
    }
    // clean up the semaphore and the shared memory
    sem_close(sem);
    sem_unlink("/mysemaphore");
    return *lastCount; // Return the total number of unique words
}

// comparator function for qsort
int compareWords(const void *a, const void *b) {
    struct Word *wordA = (struct Word *)a;
    struct Word *wordB = (struct Word *)b;
    return wordB->word_freq - wordA->word_freq; // compare frequencies
}

// sorts the words in descending order of frequency
void sort(int wordCount, struct Word *Array2) {
    qsort(Array2, wordCount, sizeof(struct Word), compareWords);
}
