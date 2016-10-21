#include "fireworks.h"

Fireworks::Fireworks(IrrlichtDevice *device, FireworkInfo info, btDynamicsWorld *world, video::ITexture *texture, core::vector3df pos, core::vector3df dir, float thrwStr):
lifetime(5), burst(0.2)
{
    /// Creating firework sphere shell.
    shell=device->getSceneManager()->addSphereSceneNode(10);
    shell->setMaterialTexture(0, texture);
    shell->setMaterialFlag(video::EMF_LIGHTING, false);
    shell->setRotation(core::vector3df(0.0f, 0.0f, 0.0f));
    shell->setPosition(pos);

    /// Setting up rigid body for fireworks shell.
    fireworkCollidesWith=WORLD_COLLISION|SCENIC_COLLISION;
    btCollisionShape *shellShape=new btSphereShape(10);
    btScalar mass=2;
    btVector3 shellInertia;
    shellShape->calculateLocalInertia(mass, shellInertia);
    btVector3 initPos(shell->getPosition().X, shell->getPosition().Y, shell->getPosition().Z);
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(initPos);
    motionState=new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo shellRigidBodyCI(mass, motionState, shellShape, shellInertia);
    shellRigidBody=new btRigidBody(shellRigidBodyCI);
    shellRigidBody->setUserPointer((void *)shell);
    cout << "dir - X: " << dir.X << " Y: " << dir.Y << " Z: " << dir.Z << endl;
    shellRigidBody->setLinearVelocity(btVector3(dir.X, dir.Y, dir.Z)*thrwStr);
    world->addRigidBody(shellRigidBody, FIREWORK_COLLISION, fireworkCollidesWith);

    /// Particle system setup.
    parSys=device->getSceneManager()->addParticleSystemSceneNode();
    emm=parSys->createPointEmitter(dir, info.minPart, info.maxPart, info.minColor, info.maxColor,
                                   info.minLifetime, info.maxLifetime, info.angle, info.minSize, info.maxSize);
    parSys->setMaterialFlag(video::EMF_LIGHTING, false);
    parSys->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
    parSys->setMaterialTexture(0, info.particleTexture);
//    parSys->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);

    /// Particle gravity affector.
    scene::IParticleGravityAffector *gravAff=parSys->createGravityAffector(core::vector3df(0.0f, -10.0f, 0.0f), 10000);
    parSys->addAffector(gravAff);
    parSys->setEmitter(0);
    /// Creating physics object and adding it to the world simulator.

    /// Setting initial state and height.
    state=FLYING;
}

Fireworks::~Fireworks()
{
    parSys->setEmitter(0);
    shell->remove();
    parSys->remove();
}

FireworkState Fireworks::getState(){return state;}

void Fireworks::checkHeight()
{
    int newHeight=shell->getPosition().Y;
    if(height>newHeight)
    {
        parSys->setPosition(shell->getPosition());
        shell->setVisible(false);
        parSys->setEmitter(emm);
        emm->drop(); /// Deleting emitter since it's no longer needed.
        state=FIREWORK;
    }
    else{height=newHeight;}
}

btRigidBody* Fireworks::getRigidBody(){return shellRigidBody;}

void Fireworks::countDown(float dt)
{
    if(burst<=0){parSys->setEmitter(NULL);}
    else{burst-=1*dt;}
    lifetime<=0?state=CLEANUP:lifetime-=1*dt;
}
