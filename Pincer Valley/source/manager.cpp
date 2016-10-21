#include "manager.h"

Manager::Manager(){for(int i=0; i<100; i++){fireworks[i]=NULL;}}
Manager::~Manager(){}

void Manager::addFirework(IrrlichtDevice *device, FireworkInfo info, btDynamicsWorld *world, video::ITexture *texture, core::vector3df pos, core::vector3df dir, float thrwStr)
{
    for(int i=0; i<100; i++)
    {
        if(fireworks[i]==NULL)
        {
            fireworks[i]=new Fireworks(device, info, world, texture, pos, dir, thrwStr);
            break;
        }
    }
}

void Manager::update(float dt)
{
    for(int i=0; i<100; i++)
    {
        if(fireworks[i]!=NULL)
        {
            FireworkState fs=fireworks[i]->getState();

            switch(fs)
            {
                case FLYING:
                fireworks[i]->checkHeight();
//                cout << "moving state!\n";
                break;

                case FIREWORK:
                fireworks[i]->countDown(dt);
//                cout << "firework state!\n";
                break;

                case CLEANUP:
                delete fireworks[i];
                fireworks[i]=NULL;
//                cout << "cleanup state!\n";
                break;

                default:
                break;
            }
        }
    }
}

btRigidBody* Manager::redrawFireworks(int i)
{
    if(fireworks[i]!=NULL){return fireworks[i]->getRigidBody();}
    else{return 0;}
}
