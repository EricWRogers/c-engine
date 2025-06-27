class TestTwo {
    Canis::Entity@ entity;

    void SetEntity(Canis::Entity@ e) {
        @entity = e;
    }
    
    int counter = 1;

    void Create() {
        Print("Created! " + counter++);
    }

    void Start() {
        Print("Started! " + counter++);
    }

    void Update(float dt) {
        Canis::Window.SetName("TestTwo");
        entity.transform.rotation.y += dt;
    }

    void Destroy() {
        Print("Destroyed! " + counter++);
    }
}
