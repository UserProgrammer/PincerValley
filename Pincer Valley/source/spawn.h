#ifndef SPAWN_CLASS_INCLUDED
#define SPAWN_CLASS_INCLUDED

#define MONSTER_CAP 5

#include <iostream>
#include <irrlicht.h>
#include "monster.h"
#include "dummy.h"

using namespace irr;

struct SpawnEssentials
{
    IrrlichtDevice *device;
    btDynamicsWorld *dynamicsWorld;

    SpawnEssentials(IrrlichtDevice *dev, btDynamicsWorld *world)
    {
        device=dev;
        dynamicsWorld=world;
    }
};

class Spawn
{
    public:
    Spawn(SpawnEssentials ess, scene::IMesh *spawnMesh, video::ITexture *spawnTexture, core::vector3df position);
    ~Spawn();

    //void addMonster(scene::IAnimatedMesh *monsterMesh, video::ITexture *monsterTexture);
    void updateMonsters(IrrlichtDevice *dev, btDynamicsWorld *dynamicsWorld, Player *player,
                         scene::IAnimatedMesh *monsterMesh, video::ITexture *monsterTexture, float dt);

    Monster *getMonsters(int i);

    private:
    /// Checks if player is within monster's "view area".
    void isPlayerNear(int element, scene::IMeshSceneNode *p);
    /// Checks if any monsters needs to be destroyed.
    void isDead(int element, Player *p);
    /// Current live monster count in the spawner.
    unsigned int currMonsters;
    /// Cooldown period after spawning a new monster.
    float cooldown;
    SpawnEssentials *essentials;
    /// Graphics Object
    scene::IMeshSceneNode *spawnNode;
    /// Physics Object.
    // Pointer array where zombie objects will be referenced.
    Monster *monster[MONSTER_CAP];
};
#endif
