#include "scene_object.h"

ScenicObject::ScenicObject(IrrlichtDevice *device, btDynamicsWorld *world, scene::IMesh *mesh, scene::ISceneNode *parent, core::vector3df position,
                           core::vector3df rotation, core::vector3df scale, video::ITexture *texture, btCollisionShape *shape)

{
    /// Build graphics object.
    graphicsObject=device->getSceneManager()->addMeshSceneNode(mesh, parent, -1, position, rotation, scale);
    graphicsObject->updateAbsolutePosition();

    graphicsObject->setMaterialFlag(video::EMF_LIGHTING, false);
    graphicsObject->setMaterialTexture(0, texture);

    /// Build collision object.
    int treeCollidesWith=PLAYER_COLLISION|FIREWORK_COLLISION|MONSTER_COLLISION;
    btCollisionShape *treeShape=shape;

    btScalar mass=0;
    btVector3 treeInertia;
    treeShape->calculateLocalInertia(mass, treeInertia);

    /// Setting up position and rotation.
    btVector3 pos(graphicsObject->getAbsolutePosition().X, graphicsObject->getAbsolutePosition().Y, graphicsObject->getAbsolutePosition().Z);
    btTransform trans;
    trans.setIdentity();

    btQuaternion btQuat;
    btQuat.setEulerZYX((graphicsObject->getRotation().X+90)*core::DEGTORAD, graphicsObject->getRotation().Y*core::DEGTORAD, graphicsObject->getRotation().Z*core::DEGTORAD);

    trans.setOrigin(pos);
    trans.setRotation(btQuat);

    /// Finishing up physics object.
    motionState=new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo treeRigidBodyCI(mass, motionState, treeShape, treeInertia);
    collisionObject=new btRigidBody(treeRigidBodyCI);
    collisionObject->setUserPointer((void *)graphicsObject);
    world->addRigidBody(collisionObject, SCENIC_COLLISION, treeCollidesWith);
    collisionObject->setGravity(btVector3(0.0f, 0.0f, 0.0f));
}

ScenicObject::~ScenicObject(){}

void ScenicObject::addChildNode(IrrlichtDevice *device, scene::IMesh *mesh, core::vector3df position,
                                core::vector3df rotation, core::vector3df scale, video::ITexture *texture)
{
    childNode=device->getSceneManager()->addMeshSceneNode(mesh, graphicsObject, -1, position, rotation, scale);
    childNode->setMaterialFlag(video::EMF_LIGHTING, false);
    childNode->setMaterialTexture(0, texture);
}

scene::IMeshSceneNode* ScenicObject::getGraphicsObject(){return graphicsObject;}

btRigidBody* ScenicObject::getPhysicsObject(){return collisionObject;}
