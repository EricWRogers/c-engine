#pragma once

extern "C" {

// Called once at startup
void GameInit();

// Called every frame with delta time
void GameUpdate(float dt);

// Called once at shutdown
void GameShutdown();

}
