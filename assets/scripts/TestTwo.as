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
    }

    void Destroy() {
        Print("Destroyed! " + counter++);
    }
}
