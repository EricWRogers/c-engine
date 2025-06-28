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
        entity.color.x = Canis::Math::RandomFloat(0.0f, 1.0f);
        entity.color.y = Canis::Math::RandomFloat(0.0f, 1.0f);
        entity.color.z = Canis::Math::RandomFloat(0.0f, 1.0f);
        Print("color " + entity.color.ToString());
    }

    void Destroy() {
        Print("Destroyed! " + counter++);
    }
}
