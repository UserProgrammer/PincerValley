#include "monster.h"

/// AnimationEndCallBack class definition.
AnimationEndCallBack::AnimationEndCallBack(){}
AnimationEndCallBack::~AnimationEndCallBack(){}

void AnimationEndCallBack::setMonsterPointer(Monster *m){monsterPtr=m;}

void AnimationEndCallBack::setAttackPtr(bool *a){attackPtr=a;}

void AnimationEndCallBack::OnAnimationEnd(scene::IAnimatedMeshSceneNode *node)
{
    switch(node->getID())
    {
        /// right arm node.
        case 42:
        node->setLoopMode(true);
        node->setFrameLoop(11, 15);
        *attackPtr=false;
        break;

        /// left arm node.
        case 16:
        break;

        /// monster node.
        case 666:
            switch(monsterPtr->getState())
            {
                case SPAWN:
                {
                    monsterPtr->getNode()->setLoopMode(true);
                    monsterPtr->getNode()->setFrameLoop(20, 100);
                    monsterPtr->setState(MOVING);
                }
                break;

                case DYING:
                {
                    monsterPtr->setState(BURY);
                }
                break;

                default:
                break;
            }
        break;

        default:
        break;
    }
}

core::vector3df getPointInCircle(core::vector3df center, int ray)
{
    float rotation=rand()/(float)RAND_MAX*2*3.1416;
    float distance=rand()/(float)RAND_MAX*ray;

    float x = cos(rotation)*distance+center.X;
    float z = sin(rotation)*distance+center.Z;

    return core::vector3df(x, center.Y, z);
}

/// Monster class definition.
Monster::Monster(IrrlichtDevice *device, btDynamicsWorld *world, scene::IAnimatedMesh *monsterMesh, video::ITexture *monsterTexture, scene::IMeshSceneNode *parent)
:vitality(2), state(SPAWN), velocity(20), resting(0), damageTextureOn(false), damageTextureTimer(0), destination(core::vector3df(0.0f, 0.0f, 0.0f))
{
    dynamicsWorld=world;
    /// Setting up graphics object
    monster=device->getSceneManager()->addAnimatedMeshSceneNode(monsterMesh, 0, 666);
    monster->setMaterialTexture(0, device->getVideoDriver()->getTexture(monsterTexture->getName()));
    monster->setMaterialFlag(video::EMF_LIGHTING, false);
    monster->setPosition(parent->getPosition());

    /// Set destination, forward vector and graphic object's rotation.
    destination=getPointInCircle(monster->getPosition(), 200); /// <-- Gets a random point within a predefined circle.

    setForwardVector(destination);

    float rot=atan2(forwardVect.Z, forwardVect.X)*core::RADTODEG;

    /// Rotating monster according to the forward vector.
    monster->setRotation(core::vector3df(0.0f, rot+90, 0.0f));

    /// Get monster node bounding box.
    core::vector3df edges=monster->getTransformedBoundingBox().getExtent();

    /// Setting up physics object.
    monsterCollidesWith=WORLD_COLLISION|SCENIC_COLLISION|PLAYER_COLLISION;
    btCollisionShape *monsterShape=new btCapsuleShape(8, 50);
    btScalar mass=2;
    btVector3 monsterInertia;
    monsterShape->calculateLocalInertia(mass, monsterInertia);

    /// Setting initial position and rotation.
    btTransform trans;
    trans.setIdentity();
    btVector3 initPos(monster->getPosition().X, monster->getPosition().Y+(75/2), monster->getPosition().Z);

    btQuaternion btQuat;
    btQuat.setEulerZYX(0.0f, rot*core::DEGTORAD, 0.0f);

    trans.setOrigin(initPos);
    trans.setRotation(btQuat);

    motionState=new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo monsterRigidBodyCI(mass, motionState, monsterShape, monsterInertia);
    monsterRigidBody=new btRigidBody(monsterRigidBodyCI);
    monsterRigidBody->setUserPointer((void *)monster);
    world->addRigidBody(monsterRigidBody, MONSTER_COLLISION, monsterCollidesWith);
    monsterRigidBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    monsterRigidBody->setActivationState(DISABLE_DEACTIVATION);
//    monsterRigidBody->setGravity(btVector3(0, 0, 0));

    /// Node animation setup.
    monster->setLoopMode(false);
    monster->setFrameLoop(170, 310);
}

Monster::~Monster()
{
    damageTextureOn=false;
    monster->remove();
    monsterRigidBody->setUserPointer((void *)NULL);
    delete monsterRigidBody->getMotionState();
    dynamicsWorld->removeRigidBody(monsterRigidBody);
    delete monsterRigidBody;
}

void Monster::setAnimationCallback(Monster *m)
{
    /// Setting up animation end callback.
    callBack.setMonsterPointer(m);
    monster->setAnimationEndCallback(&callBack);
}

scene::IAnimatedMeshSceneNode* Monster::getNode(){return monster;}

btRigidBody* Monster::getRigidBody(){return monsterRigidBody;}

int Monster::getVitality(){return vitality;}

void Monster::removeVitality(int vt){vitality-=vt;}

int Monster::getState(){return state;}

void Monster::setState(State st){state=st;}

void Monster::setRest(float r){resting=r;}

float Monster::getRest(){return resting;}

void Monster::setDestination(core::vector3df d){destination=d;}

core::vector3df Monster::getDestination(){return destination;}

void Monster::setForwardVector(core::vector3df destination)
{
    /// Calculating unit vector and saving it as the forward vector of the monster.
    float magnitude=(sqrt(pow(destination.X - monster->getPosition().X, 2)+pow(destination.Z - monster->getPosition().Z, 2)));

    /// Since the monster's forward vector only matters on the XZ plane, the Y axis shall remain 0.
    forwardVect=core::vector3df((destination.X - monster->getPosition().X)/magnitude, 0, (destination.Z - monster->getPosition().Z)/magnitude);
}

core::vector3df Monster::getForwardVector(){return forwardVect;}

void Monster::setDamageTexture(video::ITexture *damageTexture)
{
    monster->setMaterialTexture(0, damageTexture);
    damageTextureOn=true;
    damageTextureTimer=0.5;
}

void Monster::damageTextureCountDown(video::ITexture *texture, float dt)
{
    if(damageTextureTimer<=0)
    {
        monster->setMaterialTexture(0, texture);
        damageTextureOn=false;
    }
    else{damageTextureTimer-=1*dt;}
}

bool Monster::getDamageTextureOn(){return damageTextureOn;}

void Monster::moveMonster(float dt)
{
    btVector3 frwrd(forwardVect.X, forwardVect.Y, forwardVect.Z);
    monsterRigidBody->setLinearVelocity(frwrd*velocity);
}

void Monster::SynchPosition()
{
    core::vector3df edges=monster->getTransformedBoundingBox().getExtent();

    /// Set position.
    btVector3 point=monsterRigidBody->getCenterOfMassPosition();

    monster->setPosition(core::vector3df((f32)point[0], (f32)point[1]-(edges.Y/2), (f32)point[2]));

    /// Set rigid body rotation.
    btVector3 forward=monsterRigidBody->getLinearVelocity();

    float rot=atan2(forward.z(), forward.x())*core::RADTODEG;

    /// Set graphics body rotation.
    monster->setRotation(core::vector3df(0, 90-rot, 0));
}
