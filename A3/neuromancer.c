// A NEUROMANCER CYBER HEIST - synchronized version
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_PLAYERS 3
#define GAME_DURATION 10

int currentPlayer = 0; // Index of the current player
int gameActive = 1;    // Game state
int scores[NUM_PLAYERS] = {0}; // Keep track of each player's score

pthread_mutex_t lock;
pthread_cond_t turnCond;

void* hack(void* arg) {
    int id = *(int*)arg;

    while (1) {
        pthread_mutex_lock(&lock);

        // Wait for your turn or end if game stopped
        while (gameActive && id != currentPlayer) {
            pthread_cond_wait(&turnCond, &lock);
        }

        if (!gameActive) {
            pthread_mutex_unlock(&lock);
            break;
        }

        // --- Player's turn ---
        printf("Player %d is attempting to hack... (Current Player: %d)\n", id + 1, currentPlayer + 1);
        pthread_mutex_unlock(&lock);

        sleep(1); // simulate hacking time

        int hackResult = rand() % 10 + 1;
        if (hackResult <= 6) {
            printf("Player %d succeeded in hacking!\n", id + 1);
            scores[id]++;
        } else {
            printf("Player %d failed to hack!\n", id + 1);
        }

        // Move to the next player
        pthread_mutex_lock(&lock);
        currentPlayer = (currentPlayer + 1) % NUM_PLAYERS;
        pthread_cond_broadcast(&turnCond);
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main() {
    pthread_t players[NUM_PLAYERS];
    int playerIds[NUM_PLAYERS];
    srand(time(NULL));

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&turnCond, NULL);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        playerIds[i] = i;
        pthread_create(&players[i], NULL, hack, &playerIds[i]);
    }

    sleep(GAME_DURATION);
    pthread_mutex_lock(&lock);
    gameActive = 0;
    pthread_cond_broadcast(&turnCond);
    pthread_mutex_unlock(&lock);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        pthread_join(players[i], NULL);
    }

    printf("--------- GAME OVER! ---------\n");
    int winner = 0;
    int maxScore = scores[0];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (scores[i] > maxScore) {
            maxScore = scores[i];
            winner = i;
        }
    }
    printf("Player %d wins with %d points!\n", winner + 1, scores[winner]);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&turnCond);
    return 0;
}
