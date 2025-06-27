class TestOne {
    void Create() {
        Print("Created!");
    }

    void Start() {
        Print("Started!");
    }

    void Update(float dt) {
        Print("Updating with not fps dt: " + dt);
        Print("DeltaTime: " + g_Time.get_deltaTime());
        //Print("FPS: " + g_Time.fps);
    }

    void Destroy() {
        Print("Destroyed!");
    }
}
