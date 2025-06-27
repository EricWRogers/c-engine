class Time {
    static float get_deltaTime() { return Time_deltaTime(); }
    static float get_fps() { return Time_fps(); }
}

class TestTwo {
    int counter = 1;

    void Create() {
        Print("Created! " + counter++);
    }

    void Start() {
        Print("Started! " + counter++);
    }

    void Update(float dt) {
        Print("Updating with dt: " + dt + " " + counter++);
        //Print("Updating with fps: " + Time.fps);
    }

    void Destroy() {
        Print("Destroyed! " + counter++);
    }
}
