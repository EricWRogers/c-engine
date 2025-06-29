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

        //Print("color " + entity.color.ToString());

        array<Canis::Entity@>@ glassBlocks = world.GetEntitiesWithTag("glass");
        for (uint i = 0; i < glassBlocks.length(); i++)
        {
            //glassBlocks[i].color = vec3(0.0f,0.0f,1.0f);
            glassBlocks[i].color.x = 0.0f;
            glassBlocks[i].color.y = 0.0f;
            glassBlocks[i].color.z = 1.0f;
        }
        
        Canis::Entity@ glassBlock = world.GetEntityWithName("glass_0");
        glassBlock.color.x = Canis::Math::RandomFloat(0.0f, 1.0f);
        glassBlock.color.y = Canis::Math::RandomFloat(0.0f, 1.0f);
        glassBlock.color.z = Canis::Math::RandomFloat(0.0f, 1.0f);
    }

    void Destroy() {
        Print("Destroyed! " + counter++);
    }
}
