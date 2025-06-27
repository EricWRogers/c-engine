class Time {
    static float get_deltaTime() { return Time_deltaTime(); }
    static float get_fps() { return Time_fps(); }
}

class TestOne {
    void Create() {
        Print("Created!");
    }

    void Start() {
        Print("Started!");
    }

    void Update(float dt) {
        Print("Updating with not fps dt: " + dt);
        //        Print("Updating with fps: " + Time.fps);

    }

    void Destroy() {
        Print("Destroyed!");
    }
}
