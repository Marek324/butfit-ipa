/*
 * File:        main.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: Main
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */

#include "Game.h"

int main(int argc, char** argv) {
    Game game;
    if (!game.init(argc, argv)) {
        return 1;
    }
    game.run();
    game.cleanup();
    return 0;
}
