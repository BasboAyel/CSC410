#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int shield_power = 50;  

int main() {
    pid_t pid;
    int percentages[4] = {25, 20, 30, 15};
    const char *names[4] = {"Luke", "Han", "Chewbacca", "Leia"};

    printf("Millennium Falcon: Initial shield power level: %d%%\n\n", shield_power);

    for (int i = 0; i < 4; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        else if (pid == 0) { // Child process
            printf("%s: Adjusting shield power\n", names[i]);
            int new_level = shield_power + percentages[i];
            printf("%s: Shield power level now at %d%%\n", names[i], new_level);
            exit(0); // Child exits after work
        }
        // Parent continues loop
    }

    // Parent waits for all 4 children
    for (int i = 0; i < 4; i++) {
        wait(NULL);
    }

    printf("\nFinal shield power level on the Millennium Falcon: %d%%\n", shield_power);
    printf("May the forks be with you!\n");

    return 0;
}
