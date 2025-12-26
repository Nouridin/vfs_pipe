#include "vfs_pipe.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() 
{
    int kills  = 0;
    int deaths = 0;
    int hp     = 100;
    char * path = "/tmp/vfs";

    system("mkdir -p /tmp/vfs");
    
    vfs_init(path);
    vfs_register("player_kills",  &kills);
    vfs_register("player_deaths", &deaths);
    vfs_register("player_hp",     &hp);

    printf("Check your files in: %s\n", path);
    printf("Try: watch -n 0.5 'ls -l %s && cat %s/*'\n", path, path);

    while (1) 
    {
        kills++;
        if (kills % 5 == 0) deaths++;
        hp = rand() % 100; // Randomize HP to see it jump
        
        usleep(500000); // Update every 0.5 seconds
    }

    vfs_cleanup(path);
    return 0;
}