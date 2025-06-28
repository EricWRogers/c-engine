#include "Script.as"

class TestTwo : Script {
    int counter = 1;
    float startingHeight = 0.0f;
    float timeElaps = 0.0f;

    void Create() {
        timeElaps = Canis::Math::RandomFloat(0.0f, 1.0f);
        startingHeight = entity.transform.position.y;
    }

    void Start() {
        Print("Started! " + counter++);
    }

    void Update(float dt) {
        timeElaps += dt;
        entity.transform.position.y = sin(timeElaps);

        Canis::Window.SetName("TestTwo");

        entity.color.x = Canis::Math::RandomFloat(0.0f, 1.0f);
        entity.color.y = Canis::Math::RandomFloat(0.0f, 1.0f);
        entity.color.z = Canis::Math::RandomFloat(0.0f, 1.0f);

        Print("color " + entity.color.ToString());
        
        Canis::Entity@ glassBlock = world.GetEntityWithTag("glass");
        glassBlock.color.x = Canis::Math::RandomFloat(0.0f, 1.0f);
        glassBlock.color.y = Canis::Math::RandomFloat(0.0f, 1.0f);
        glassBlock.color.z = Canis::Math::RandomFloat(0.0f, 1.0f);

    }

    void Destroy() {
        Print("Destroyed! " + counter++);
    }
}
