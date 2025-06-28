class Script {
    Canis::Entity@ entity;
    Canis::World@ world;

    void SetEntity(Canis::Entity@ _entity) {
        @entity = _entity;
    }

    void SetWorld(Canis::World@ _world) {
        @world = _world;
    }
}
