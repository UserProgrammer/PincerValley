#ifndef INCLUDE_ZOMBIE_CLASS
#define INCLUDE_ZOMBIE_CLASS

#include <iostream>
#include <cmath>
#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "collision_filter.h"

using namespace std;
using namespace irr;

enum State{SPAWN=0, IDLE=1, MOVING=2, CHASING=3, ATTACKING=4, DYING=5, BURY=6};

/// Monster class declaration.
class Monster;

class AnimationEndCallBack:public scene::IAnimationEndCallBack
{
    public:
    AnimationEndCallBack();
    ~AnimationEndCallBack();

    void setMonsterPointer(Monster *m);
    void setAttackPtr(bool *a);

    void OnAnimationEnd(scene::IAnimatedMeshSceneNode *node);

    private:
    Monster *monsterPtr;
    bool *attackPtr;
};

core::vector3df getPointInCircle(core::vector3df center, int ray);

/// Monster class header definition.
class Monster
{
    public:
    Monster(IrrlichtDevice *device, btDynamicsWorld *world, scene::IAnimatedMesh *monsterMesh, video::ITexture *monsterTexture, scene::IMeshSceneNode *parent);
    ~Monster();

    void setAnimationCallback(Monster *m);

    scene::IAnimatedMeshSceneNode *getNode();
    btRigidBody *getRigidBody();

    void removeVitality(int vt);
    int getVitality();

    void setState(State st);
    int getState();

    void setRest(float r);
    float getRest();

    void setDestination(core::vector3df d);
    core::vector3df getDestination();

    void setForwardVector(core::vector3df vec);
    core::vector3df getForwardVector();

    void setDamageTexture(video::ITexture *damageTexture);
    void damageTextureCountDown(video::ITexture *texture, float dt);
    bool getDamageTextureOn();

    void moveMonster(float dt);

    void SynchPosition();

    private:
    /// Health Points.
    int vitality;
    /// Keeps track of what the monster physics object collides with.
    int monsterCollidesWith;
    /// Current state of the monster.
    State state;
    /// Speed of translation.
    float velocity;
    /// Variable keeps track of how much time is left before the monster moves again.
    float resting;
    /// Boolean which tells if the damage texture is on.
    bool damageTextureOn;
    /// Amount of time before the monster's texture is reverted back to normal.
    float damageTextureTimer;
    /// Destination that the monsters must reach when in the moving state.
    core::vector3df destination;
    /// Monster forward vector.
    core::vector3df forwardVect;
    /// Used to triger events at the end of an animation.
    AnimationEndCallBack callBack;
    /// Graphics Object
    scene::IAnimatedMeshSceneNode *monster;
    /// Monster motion state.
    btDefaultMotionState *motionState;
    /// Physics Object.
    btRigidBody *monsterRigidBody;
    /// Physics world.
    btDynamicsWorld *dynamicsWorld;
};
#endif
