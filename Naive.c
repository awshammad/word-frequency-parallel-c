#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

#define MAX_WORD_LENGTH 100

typedef struct {
    char word[MAX_WORD_LENGTH];// to have the word in it
    int count;// for the word freq
} WordFreq;

void read_from_file(WordFreq **wordFreqArray, int *wordCount, int *arraySize) {
    FILE *in = fopen("/home/aws-hammad/Downloads/text8.txt", "r");
    if (in == NULL) {
        perror("Error opening file");
        return;
    }
    char word[MAX_WORD_LENGTH];// buffer to have words in it
    while (fscanf(in, "%99s", word) != EOF) {// read words one by one
        int found = 0;
        for (int i = 0; i < *wordCount; i++) {// check if the word already exists in the array
            if (strcmp((*wordFreqArray)[i].word, word) == 0) {// if found
                (*wordFreqArray)[i].count++;// increase its freq
                found = 1;
                break;
            }
        }
        if (!found) {// if not found
            if (*wordCount >= *arraySize) {// resize if needed
                *arraySize *= 2;// double the size
                *wordFreqArray = realloc(*wordFreqArray, (*arraySize) * sizeof(WordFreq));// reallocation
                if (!*wordFreqArray) {
                    perror("Memory reallocation failed");
                    fclose(in);
                    return;
                }
            }
            strcpy((*wordFreqArray)[*wordCount].word, word);// add the word
            (*wordFreqArray)[*wordCount].count = 1;// put its freq to 1
            (*wordCount)++;// increase wordCount by 1
        }
    }
    fclose(in);// close the file
}

// comparison function for qsort
int compare(const void *a, const void *b) {
    WordFreq *freqA = (WordFreq *)a;
    WordFreq *freqB = (WordFreq *)b;
    return freqB->count - freqA->count;// descending order by frequency
}

int main() {
    struct timeval startSerial, endSerial, startParallel, endParallel;
    int initialSize = 10;
    WordFreq *wordFreqArray = malloc(initialSize * sizeof(WordFreq));
    if (!wordFreqArray) {
        perror("Memory allocation failed");
        return EXIT_FAILURE;
    }
    int wordCount = 0;
    int arraySize = initialSize;
    gettimeofday(&startParallel, NULL);// start parallel time
    read_from_file(&wordFreqArray, &wordCount, &arraySize);// read words and populate the WordFreq array
    gettimeofday(&endParallel, NULL);// end parallel time
    gettimeofday(&startSerial, NULL);// start serial time
    qsort(wordFreqArray, wordCount, sizeof(WordFreq), compare);// sort the array by frequency
    gettimeofday(&endSerial, NULL);// end serial time
    // print the top 10 words
    printf("Aws Hammad - 1221697\nTop Word Frequencies:\n");
    int printCount = wordCount < 10 ? wordCount : 10;// if the words less than 10 make printCount equals them else equals 10
    for (int i = 0; i < printCount; i++) {
        printf("%d- %s: %d\n", (i+1) ,wordFreqArray[i].word, wordFreqArray[i].count);
    }
    double serialTime = (endSerial.tv_sec - startSerial.tv_sec) + (endSerial.tv_usec - startSerial.tv_usec) / 1000000.0;
    double parallelTime = (endParallel.tv_sec - startParallel.tv_sec) + (endParallel.tv_usec - startParallel.tv_usec) / 1000000.0;
    printf("\nSerial execution time: %.6f seconds\n", serialTime);
    printf("Sorting (parallel section) execution time: %.6f seconds\n\n", parallelTime);
    free(wordFreqArray);// free allocated memory
    return 0;
}
