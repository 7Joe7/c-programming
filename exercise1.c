#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_READERS 5
#define NUM_READS 5
#define NUM_WRITERS 5
#define NUM_WRITES 5

unsigned int gSharedResource = 0; /* shared resource */
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; /* mutex lock for resource */
pthread_cond_t gWritePhase = PTHREAD_COND_INITIALIZER; /* writer */
pthread_cond_t gReadPhase = PTHREAD_COND_INITIALIZER; /* reader */
int gWaitingReaders = 0, gWriters = 0, gReaders = 0;

void *readerMain(void *threadArgument) {

	int id = *((int*)threadArgument);
	int i = 0, numReaders = 0;

	for (i = 0; i < NUM_READS; i++) {
		usleep(1000 * (random() % NUM_READERS + NUM_WRITERS));

		pthread_mutex_lock(&m);
			gWaitingReaders++;
			while (gReaders == -1) {
				pthread_cond_wait(&gReadPhase, &m);
			}
			gWaitingReaders--;
			numReaders = ++gReaders;
		pthread_mutex_unlock(&m);

		fprintf(stdout, "[r%d] reading %u [readers: %2d][writers: %2d]\n", id, gSharedResource, numReaders, gWriters);

		pthread_mutex_lock(&m);
			if (--gReaders == 0) {
				pthread_cond_signal(&gWritePhase);
			}
		pthread_mutex_unlock(&m);
	}

	pthread_exit(0);
}

void *writerMain(void *threadArgument) {
	int id = *((int*)threadArgument);
	int i = 0, numReaders = 0, numWriters = 0;

	for (i = 0; i < NUM_WRITES; i++) {
		usleep(1000 * (random() % NUM_READERS + NUM_WRITERS));

		pthread_mutex_lock(&m);
			while (gReaders != 0) {
				pthread_cond_wait(&gWritePhase, &m);
			}
			gReaders = -1;
			numWriters = ++gWriters;
			numReaders = gReaders;
		pthread_mutex_unlock(&m);

		fprintf(stdout, "[w%d] writing %u [readers: %2d][writers: %2d]\n", id, gSharedResource, numReaders, numWriters);

		pthread_mutex_lock(&m);
			gReaders = 0;
			gWriters--;
			if (gWaitingReaders > 0) {
				pthread_cond_broadcast(&gReadPhase);
			} else {
				pthread_cond_signal(&gWritePhase);
			}
		pthread_mutex_unlock(&m);
	}

	pthread_exit(0);
}

int main(int argc, char *argv[]) {
	int i;

	int numReaders[NUM_READERS];
	int numWriters[NUM_WRITERS];

	pthread_t readerThreadIDs[NUM_READERS];
	pthread_t writerThreadIDs[NUM_WRITERS];

	// Seed the random number generator
	srandom((unsigned int)time(NULL));

	for (i = 0; i < NUM_READERS; i++) {
		numReaders[i] = i;
		pthread_create(&readerThreadIDs[i], NULL, readerMain, &numReaders[i]);
	}

	for (i = 0; i < NUM_WRITERS; i++) {
		numWriters[i] = i;
		pthread_create(&writerThreadIDs[i], NULL, writerMain, &numWriters[i]);
	}

	for (i = 0; i < NUM_READERS; i++) {
		pthread_join(readerThreadIDs[i], NULL);
	}

	for (i = 0; i < NUM_WRITERS; i++) {
		pthread_join(writerThreadIDs[i], NULL);
	}
}