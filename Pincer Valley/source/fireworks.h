#ifndef INCLUDE_FIREWORKS_CLASS
#define INCLUDE_FIREWORKS_CLASS

#include <iostream>
#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "collision_filter.h"

using namespace std;
using namespace irr;

enum FireworkState{FLYING=0, FIREWORK=1, CLEANUP=2};

struct FireworkInfo
{

    int minPart, maxPart;
    video::SColor minColor, maxColor;
    int minLifetime, maxLifetime;
    core::dimension2df minSize, maxSize;
    int angle;
    video::ITexture *particleTexture;

    FireworkInfo(){}
    FireworkInfo(int minP, int maxP, video::SColor minC, video::SColor maxC,
                 int minL, int maxL, core::dimension2df minS, core::dimension2df maxS,
                 int a, video::ITexture *texture)
    {
        minPart=minP;
        maxPart=maxP;
        minColor=minC;
        maxColor=maxC;
        minLifetime=minL;
        maxLifetime=maxL;
        minSize=minS;
        maxSize=maxS;
        angle=a;
        particleTexture=texture;
    }
};

class Fireworks
{
    public:
    Fireworks(IrrlichtDevice *device, FireworkInfo info, btDynamicsWorld *world, video::ITexture *texture, core::vector3df pos, core::vector3df dir, float thrwStr);
    ~Fireworks();

    FireworkState getState();
    btRigidBody *getRigidBody();
    void checkHeight();
    void countDown(float dt);

    private:
    /// Binary state machine, state holder.
    FireworkState state;
    /// Collision filter.
    int fireworkCollidesWith;
    /// Current height of the shell object.
    float height;
    /// Firework burst.
    float burst;
    /// Duration of the firework before being destroyed.
    float lifetime;
    /// Forward direction for movement.
    core::vector3df direction;
    /// Graphics object representing the fireworks.
    scene::IMeshSceneNode *shell;
    /// Shell motion state.
    btDefaultMotionState *motionState;
    /// Shell rigid body.
    btRigidBody *shellRigidBody;
    /// Particle system's emitter.
    scene::IParticlePointEmitter *emm;
    /// Particle system.
    scene::IParticleSystemSceneNode *parSys;
};

#endif
