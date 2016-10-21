#ifndef INCLUDE_DUMMY_CLASS
#define INCLUDE_DUMMY_CLASS

#include <iostream>
#include <math.h>
#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "collision_filter.h"
using namespace std;
using namespace irr;

enum Direction{FORWARD=0, LEFT=1, BACKWARDS=2, RIGHT=4};
enum Type{TYPE01=0, TYPE02=1, TYPE03=2, TYPE04=3};

core::vector3df Cross(core::vector3df a, core::vector3df b);

struct Inventory
{
    int fwType[4];
    void setAll(int n){for(int i=0; i<4; i++){fwType[i]=n;}}
    void setFw(int f, int n){fwType[f]=n;}
};

class Player
{
    public:
    Player(IrrlichtDevice *device, btDynamicsWorld *world, scene::IMesh *mesh,
           video::ITexture *texture, core::vector3df forwardVector, core::vector3df upVector);
    ~Player();

    /// Set player arms.
    void setArms(scene::IAnimatedMeshSceneNode *leftArm, core::vector2di leftIdle, scene::IAnimatedMeshSceneNode *rightArm, core::vector2di rightIdle);

    /// get player's mesh node.
    scene::IMeshSceneNode *getNode();

    /// get the player's rigid body.
    btRigidBody *getPlayerRigidBody();

    /// get the player's camera object.
    scene::ICameraSceneNode *getCamera();

    /// get camera forward vector
    core::vector3df getForwardVector();

    /// set selected firework.
    void setSelectedFW(Type t);

    /// get selected firework.
    int getSelectedFW();

    /// Set value in inventory item. !!! Probably wont be necessary !!!
    void setInventoryItem(int f, int n);

    /// Get value in inventory item.
    int getInventoryItemValue(int f);

    /// This function updates the camera and player rotation as well as repositioning the camera and player vectors.
    void pRotation(core::vector2df cursorDelta);

    /// This function is used to move the player through the map.
    void pMovement(f32 dt, Direction d);

    /// update player's camera target to reposition the forward vector.
    void cUpdateTarget();

    /// Update player camera target.
    void pUpdate();

    float getHealth();
    void setHealth(float h);

    private:
    /// Irrlicht player scene node.
    scene::IMeshSceneNode *player;
    /// Animated left arm.
    scene::IAnimatedMeshSceneNode *leftArm;
    /// Animated right arm.
    scene::IAnimatedMeshSceneNode *rightArm;
    /// Bullet rigid body pointer to player rigid body.
    btRigidBody *btPlayer;
    /// Rigid body motion state.
    btDefaultMotionState *playerMotionState;
    /// Bullet transform used to rotate and translate rigis body.
    btTransform trans;
    /// Irrlicht Camera scene node.
    scene::ICameraSceneNode *camera;
    /// Temporary node for visualizing camera.
    scene::IMeshSceneNode *cameraReference;
    /// boolean keeps track of the player's attacking action.
    bool attacking;
    /// Movement speed.
    float velocity;
    /// Player's health.
    float health;
    /// Inventory.
    Inventory invent;
    /// Selected firework type.
    Type selectedType;
    /// forward, right and up vectors for the player.
    core::vector3df vForward, vRight, vUp;
    /// forward, right and up vectors for the camera.
    core::vector3df vCamForward, vCamRight, vCamUp;
};

#endif
