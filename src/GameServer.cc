#include "Game.h"

int main(int argc, char **argv)
{
    GameServer es(argv[1], argv[2]);

    es.do_messages();

    return 0;
}
