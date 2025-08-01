#pragma once

extern "C" {

// Called once at startup
void* GameInit(void* _app);

// Called every frame with delta time
void GameUpdate(void* _app, float dt, void* _data);

// Called once at shutdown
void GameShutdown(void* _app, void* _data);

}
