#include <stdio.h>
#include <unistd.h> // for sleep()

// 1. Define the implementation macro BEFORE including the header
#define VFS_PIPE_IMPLM
#include "vfs_pipe.h"

int main() {
    // 2. Define some local variables to track
    int health = 100;
    char status[64] = "Patrolling";

    // 3. Register them with our VFS
    vfs_register_int("player_health", &health);
    vfs_register_str("player_status", status, sizeof(status));

    // 4. Mount the VFS to a folder named 'mnt'
    // Make sure the 'mnt' directory exists.
    printf("Mounting VFS to ./mnt...\n");
    vfs_init("./mnt");

    // 5. Loop and print the variables to see them change in real-time.
    printf("VFS is running. Try 'cat mnt/player_health' or 'echo 50 > mnt/player_health'\n");
    printf("Press Ctrl+C to stop.\n");

    while(1) {
        printf("\r[LIVE DATA] Health: %d | Status: %s   ", health, status);
        fflush(stdout);
        sleep(1);
    }

    return 0;
}
