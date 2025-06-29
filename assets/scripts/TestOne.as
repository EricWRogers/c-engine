#include "Script.as"

class TestOne {
    void Create() {
        Canis::Frame.SetTargetFPS(1000.0);
    }

    void Start() {
        Print("Started!");
    }

    void Update(float dt) {
        /*Print("Updating with not fps dt: " + dt);
        Print("DeltaTime: " + Canis::Frame.GetDeltaTime());
        Print("FPS: " + Canis::Frame.GetFPS());
        Print("MaxFPS: " + Canis::Frame.GetMaxFPS());*/
        Print("FPS: " + Canis::Frame.GetFPS());
    }

    void Destroy() {
        Print("Destroyed!");
    }
}
