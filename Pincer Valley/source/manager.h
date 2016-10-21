#ifndef INCLUDE_MANAGER_CLASS
#define INCLUDE_MANAGER_CLASS

#include <iostream>
#include <irrlicht.h>
#include "fireworks.h"

using namespace std;
using namespace irr;

class Manager
{
    public:
    Manager();
    ~Manager();

    void addFirework(IrrlichtDevice *device, FireworkInfo info, btDynamicsWorld *world,
                     video::ITexture *texture, core::vector3df pos, core::vector3df dir, float thrwStr);
    void update(float dt);
    btRigidBody *redrawFireworks(int i);

    private:
    Fireworks *fireworks[100];
};

#endif
