class TestOne {
    void Create() {
        Print("Created!");
    }

    void Start() {
        Print("Started!");
    }

    void Update(float dt) {
        Print("Updating with not fps dt: " + dt);
        Print("DeltaTime: " + Canis::Frame.GetDeltaTime());
        Print("FPS: " + Canis::Frame.GetFPS());
        Print("MaxFPS: " + Canis::Frame.GetMaxFPS());
        Canis::Frame.SetTargetFPS(120.0);
    }

    void Destroy() {
        Print("Destroyed!");
    }
}
