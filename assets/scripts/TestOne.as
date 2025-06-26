class TestOne {
    void Create() {
        Print("Created!");
    }

    void Start() {
        Print("Started!");
    }

    void Update(float dt) {
        Print("Updating with dt: " + dt);
    }

    void Destroy() {
        Print("Destroyed!");
    }
}
