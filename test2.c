#include "vfs_pipe.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() 
{
    int  level = 1;
    char name[64] = "PlayerOne";
    char * path = "/tmp/vfs";

    system("mkdir -p /tmp/vfs");
    vfs_init(path);
    
    vfs_register_int("level", &level);
    vfs_register_str("player_name", name);

    printf("Try: cat /tmp/vfs/player_name\n");
    printf("Try: echo 'TheBoss' > /tmp/vfs/player_name\n");

    while(1) { sleep(1); } // Keep alive
    return 0;
}