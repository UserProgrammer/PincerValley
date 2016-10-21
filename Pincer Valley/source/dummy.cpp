#include "dummy.h"

/// Function that outputs the cross product of two vectors.
core::vector3df Cross(core::vector3df a, core::vector3df b)
{
    core::vector3df c(core::vector3df(a.Y*b.Z-a.Z*b.Y, a.Z*b.X-a.X*b.Z, a.X*b.Y-b.X*a.Y));
    return c;
}

scene::IMeshSceneNode* Player::getNode(){return player;}

btRigidBody* Player::getPlayerRigidBody(){return btPlayer;}

scene::ICameraSceneNode* Player::getCamera(){return camera;}

core::vector3df Player::getForwardVector()
{
    float magnitude=sqrt(pow(vCamForward.X, 2)+pow(vCamForward.Y, 2)+pow(vCamForward.Z, 2));
    core::vector3df vector=vCamForward/magnitude;
    return vector;
}

void Player::setSelectedFW(Type t){selectedType=t;}

int Player::getSelectedFW(){return selectedType;}

void Player::setInventoryItem(int f, int n){invent.setFw(f, n);}

int Player::getInventoryItemValue(int f){return invent.fwType[f];}

Player::Player(IrrlichtDevice *device, btDynamicsWorld *world, scene::IMesh *mesh, video::ITexture *texture,
               core::vector3df forwardVector, core::vector3df upVector):velocity(100), health(100)
{
    /// Setting up player graphics object.
    player=device->getSceneManager()->addMeshSceneNode(mesh);
    player->setMaterialTexture(0, texture);
    player->setMaterialFlag(video::EMF_LIGHTING, false);

    /// Player scale and initial position.
    player->setPosition(core::vector3df(2500.0f, 400.0f, 2500.0f));

    /// Initializing selected firework type and firework types amount.
    selectedType=TYPE01;

    /// Pointer to bullet (player) rigid body.
    invent.setAll(0);

    /// Creating dummy collision body.
    int playerCollidesWith=WORLD_COLLISION|SCENIC_COLLISION;
    core::aabbox3df boundingBox=player->getTransformedBoundingBox();
    core::vector3d<f32> edges=player->getTransformedBoundingBox().getExtent();
    btVector3 btPlayerScale(edges.X*0.5f, edges.Y*0.5, edges.Z*0.5f);
    btCollisionShape *playerShape=new btBoxShape(btPlayerScale);
    btScalar mass=1;
    btVector3 playerInertia;
    playerShape->calculateLocalInertia(mass, playerInertia);

    /// Setting initial position
    core::vector3df position=player->getPosition();
    btVector3 btposition(position.X, position.Y, position.Z);
    btTransform playerTrans;
    playerTrans.setIdentity();
    playerTrans.setOrigin(btposition);
    playerMotionState=new btDefaultMotionState(playerTrans);

    /// RigidBody setup.
    btRigidBody::btRigidBodyConstructionInfo playerRigidBodyCI(mass, playerMotionState, playerShape, playerInertia);
    btPlayer=new btRigidBody(playerRigidBodyCI);
    btPlayer->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));

    btPlayer->setUserPointer((void *)player);
    btPlayer->setActivationState(DISABLE_DEACTIVATION);

    /// Adding rigid body to the world.
    world->addRigidBody(btPlayer, PLAYER_COLLISION, playerCollidesWith);

//    btPlayer->setGravity(btVector3(0.0f, 0.0f, 0.0f));

    /// (Bullet Physics) Initializing transform that will hold player's position and rotation in the world.
    trans.setIdentity();
    trans=btPlayer->getCenterOfMassTransform();

    /// Creating a new camera and attaching it to the dummy.
    camera=device->getSceneManager()->addCameraSceneNode(player, core::vector3df(player->getAbsolutePosition().X, player->getAbsolutePosition().Y+5.0f,
                                                                                  player->getAbsolutePosition().Z));
    camera->setTarget(core::vector3df(camera->getAbsolutePosition().X*50.0f, 0.0f, 0.0f));
    camera->setFarValue(10000);

    /// Creating a node in order to see where the camera attached to the dummy is placed.
    cameraReference=device->getSceneManager()->addSphereSceneNode(20, 32, camera);
    cameraReference->setMaterialTexture(0, device->getVideoDriver()->getTexture("./assets/EpikCubeTexture"));
    cameraReference->setMaterialFlag(video::EMF_LIGHTING, false);

    /// Setting intial values for the player vectors.
    vForward=forwardVector;
    vUp=upVector;
    vRight=Cross(vForward, vUp);

    /// Setting intial values for the camera vectors.
    vCamForward=camera->getTarget();
    vCamUp=vUp;
    vRight=Cross(vCamForward, vCamUp);
}

Player::~Player(){}

void Player::pRotation(core::vector2df cursorDelta)
{
    core::vector2df vectorXZ(sin(cursorDelta.Y), cos(cursorDelta.Y));
    trans=btPlayer->getCenterOfMassTransform();

    /// Rotating player rigid body.
    btQuaternion btPlayerRot=btPlayer->getOrientation();
    btQuaternion pRotation;
    pRotation.setEulerZYX(0.0f, core::DEGTORAD*cursorDelta.X, 0.0f);
    btPlayerRot*=pRotation;
    trans.setRotation(btPlayerRot);
    btPlayer->setCenterOfMassTransform(trans);

    /// Rotating camera node.
    camera->setRotation(core::vector3df(0.0f, 0.0f, camera->getRotation().Z-cursorDelta.Y));
    float rotY=player->getRotation().Y*core::DEGTORAD;
    float rotZ=player->getRotation().Z*core::DEGTORAD;

    /// Repositioning player's forward vector.
    float rot=rotZ+rotY*cos(rotZ);
//    float Rot=
    vForward.X=cos(rot);
    vForward.Z=-sin(rot);

    /// Calculating player's right vector.
    vRight=Cross(vForward, vUp);

    /// Repositioning camera's forward vector.
    vCamForward.X=vForward.X*cos(camera->getRotation().Z/360*2*3.1416/180)*50;
    vCamForward.Z=vForward.Z*cos(camera->getRotation().Z/360*2*3.1416/180)*50;
    vCamForward.Y=sin(camera->getRotation().Z/360*2*3.1416)*50;

    /// Calculating camera's right vector.
    vCamRight=Cross(vCamForward, vUp);
    /// Calculating camera's up vector.
    vCamUp=Cross(vCamRight, vCamForward)/50;
}

void Player::pMovement(f32 dt, Direction d)
{
    btVector3 frwrd(vForward.X, vForward.Y, vForward.Z);
    btVector3 right(vRight.X, vRight.Y, vRight.Z);

    switch(d)
    {
        case FORWARD:
        trans.setOrigin(btPlayer->getCenterOfMassTransform().getOrigin()+frwrd*velocity*dt);
        break;

        case RIGHT:
        trans.setOrigin(btPlayer->getCenterOfMassTransform().getOrigin()-right*velocity*dt);
        break;

        case BACKWARDS:
        trans.setOrigin(btPlayer->getCenterOfMassTransform().getOrigin()-frwrd*velocity*dt);
        break;

        case LEFT:
        trans.setOrigin(btPlayer->getCenterOfMassTransform().getOrigin()+right*velocity*dt);
        break;
    }
    /// updating rigid body's positions.
    btPlayer->setCenterOfMassTransform(trans);
}

void Player::cUpdateTarget(){camera->setTarget(camera->getAbsolutePosition()+vCamForward);}

float Player::getHealth(){return health;}
void Player::setHealth(float h){health=h;}
