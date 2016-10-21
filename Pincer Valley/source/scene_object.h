#ifndef INCLUDE_SCENE_OBJECT_CLASS
#define INCLUDE_SCENE_OBJECT_CLASS

#include <iostream>
#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "collision_filter.h"

using namespace std;
using namespace irr;

class ScenicObject
{
    public:
    ScenicObject(IrrlichtDevice *device, btDynamicsWorld *world, scene::IMesh *mesh, scene::ISceneNode *parent, core::vector3df position,
                 core::vector3df rotation, core::vector3df scale, video::ITexture *texture, btCollisionShape *shape);
    ~ScenicObject();

    void addChildNode(IrrlichtDevice *device, scene::IMesh *mesh, core::vector3df position,
                 core::vector3df rotation, core::vector3df scale, video::ITexture *texture);

    scene::IMeshSceneNode *getGraphicsObject();
    btRigidBody *getPhysicsObject();

    private:
    scene::IMeshSceneNode *graphicsObject;
    scene::IMeshSceneNode *childNode;
    btRigidBody *collisionObject;
    btDefaultMotionState *motionState;
};

#endif
