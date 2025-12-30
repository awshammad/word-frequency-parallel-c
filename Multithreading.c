#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

struct Word {
    char word[50];// word with a max length 50
    int word_freq;// the frequency of the word
};

struct Word *input_words = NULL;// array that have the words from the file
struct Word *last_words = NULL;// array to store the unique words
int total_count = 0;// the number of words that have been read from the file
int unique_count = 0;// the number of unique words

// function declarations
int read_from_file(struct Word **words);// to read words from the file
int calculate_Freq(int totalWords, struct Word *words, struct Word **result, int threadCount);// to calculate word frequency using threads
void sort(int wordCount, struct Word *words);// to sort words by frequency
void *thread_freq(void *data);// to calculate frequency in a thread

typedef struct {
    int start;// the starting index for the thread
    int end;// the ending index for the thread
    struct Word *result;// array to store the results for the thread
} ThreadData;

int main() {
    struct timeval start, end;// to measure execution time
    double total_time;// to store total time
    gettimeofday(&start, NULL);// start measuring time
    int threadNum; // number of threads
    printf("Aws Hammad - 1221697\nEnter the number of threads: ");
    scanf("%d", &threadNum);
    total_count = read_from_file(&input_words);// read words from the file
    unique_count = calculate_Freq(total_count, input_words, &last_words, threadNum);// calculate frequency of each word
    sort(unique_count, last_words);// sort the words by frequency
    printf("Top 10 most frequent words:\n");// display the top 10 words
    for (int i = 0; i < 10 && i < unique_count; i++) {
        printf("%d) %s: %d\n", i + 1, last_words[i].word, last_words[i].word_freq);
    }
    gettimeofday(&end, NULL);// stop measuring time
    // calculate elapsed time in seconds
    total_time = (end.tv_sec - start.tv_sec) * 1000000.0;
    total_time += (end.tv_usec - start.tv_usec);
    printf("\n%f seconds\n", total_time / 1000000.0);
    return 0;
}

// to read words from the file into an array
int read_from_file(struct Word **words) {
    FILE *file = fopen("/home/aws-hammad/Downloads/text8.txt", "r");// open the file
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 0;
    }
    int initialSize = 1000000;// initial size of the array
    int count = 0;// counter for the number of words
    *words = (struct Word *)malloc(initialSize * sizeof(struct Word));// allocate memory for the array
    while (fscanf(file, "%s", (*words)[count].word) == 1) {// read words from the file word by word
        (*words)[count].word_freq = 0;// initialize the frequency to 0
        count++;
        if (count >= initialSize) {// increase the array size if the count is more than the size
            initialSize *= 2;
            *words = (struct Word *)realloc(*words, initialSize * sizeof(struct Word));
        }
    }
    fclose(file);// close the file
    return count;// return the total number of words read
}

// function to calculate word frequencies using multiple threads
int calculate_Freq(int totalWords, struct Word *words, struct Word **result, int threadCount) {
    int size = totalWords / threadCount;// size of each chunk of words
    int remainder = totalWords % threadCount;// remainder words to be handled by the last thread
    *result = (struct Word *)malloc(totalWords * sizeof(struct Word));// allocate memory for the result array
    pthread_t threads[threadCount];// array to store thread IDs
    ThreadData threadData[threadCount];// array to store thread-specific data
    for (int i = 0; i < threadCount; i++) {// create threads
        threadData[i].start = i * size;// set start index
        threadData[i].end = threadData[i].start + size;// set end index
        if (i == threadCount - 1)// add remainder to the last thread
            threadData[i].end += remainder;
        threadData[i].result = *result;
        pthread_create(&threads[i], NULL, thread_freq, &threadData[i]);// create thread
    }
    for (int i = 0; i < threadCount; i++) {// wait for all threads to complete
        pthread_join(threads[i], NULL);
    }
    return unique_count;// return the total number of unique words
}

// function to calculate frequency of words in a thread
void *thread_freq(void *data) {
    ThreadData *threadData = data;// get thread-specific data
    struct Word *local_array = (struct Word *)malloc((threadData->end - threadData->start) * sizeof(struct Word));// local array to store unique words
    int local_count = 0;// count of unique words in this thread
    for (int i = threadData->start; i < threadData->end; i++) {// calculate frequency of words in the assigned range
        int exists = 0;// flag to check if the word already exists in the local array
        for (int j = 0; j < local_count; j++) {
            if (strcmp(local_array[j].word, input_words[i].word) == 0) {
                local_array[j].word_freq++;// increment frequency if word exists
                exists = 1;
                break;
            }
        }
        if (!exists) {
            strcpy(local_array[local_count].word, input_words[i].word);// copy the the word to the local array
            local_array[local_count].word_freq = 1;// put its freq 1
            local_count++;// increase the counter by 1
        }
    }
    for (int i = 0; i < local_count; i++) {// merge local results into the shared result array
        int found = 0;// to check if the word is already exists in the shared array
        for (int j = 0; j < unique_count; j++) {
            if (strcmp(threadData->result[j].word, local_array[i].word) == 0) {// if found
                threadData->result[j].word_freq += local_array[i].word_freq;// update its frequency
                found = 1;// its found
                break;
            }
        }
        if (!found) {
            strcpy(threadData->result[unique_count].word, local_array[i].word);// copy word to the shared array
            threadData->result[unique_count].word_freq = local_array[i].word_freq;
            unique_count++;//increase its freq by 1
        }
    }
    free(local_array); // free local array memory
    pthread_exit(0);
}

// function to compare for qsort
int compareWords(const void *a, const void *b) {
    struct Word *wordA = (struct Word *)a;
    struct Word *wordB = (struct Word *)b;
    return wordB->word_freq - wordA->word_freq; // descending order by frequency
}

// function to sort words by frequency
void sort(int wordCount, struct Word *words) {
    qsort(words, wordCount, sizeof(struct Word), compareWords);
}
