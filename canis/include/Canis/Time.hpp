#pragma once

namespace Canis
{
namespace Time
{
// init engine time system
extern void Init(float _targetFPS);
// clean up engine time system
extern void Quit();

// call when you start working on your frame
// return deltaTime
extern float StartFrame();

// call when you finish your frame
// return fps
extern float EndFrame();


// game code

// set target fps
extern void SetTargetFPS(float _targetFPS);

// get deltaTime of last frame
extern float DeltaTime();

// get average fps
extern float FPS();
}
}