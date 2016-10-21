#include "spawn.h"

Spawn::Spawn(SpawnEssentials ess, scene::IMesh *spawnMesh, video::ITexture *spawnTexture, core::vector3df position):currMonsters(0), cooldown(0)
{
    essentials=&ess;

    /// Setting up graphics object.
    spawnNode=essentials->device->getSceneManager()->addMeshSceneNode(spawnMesh);
    spawnNode->setMaterialTexture(0, essentials->device->getVideoDriver()->getTexture(spawnTexture->getName()));
    spawnNode->setMaterialFlag(video::EMF_LIGHTING, false);
    spawnNode->setPosition(position);

    /// Setting monster array elements to null.
    for(int i=0; i<MONSTER_CAP; i++){monster[i]=NULL;}
}

Spawn::~Spawn(){}

void Spawn::updateMonsters(IrrlichtDevice *dev, btDynamicsWorld *dynamicsWorld, Player *player,
                           scene::IAnimatedMesh *monsterMesh, video::ITexture *monsterTexture, float dt)
{
    /// This cycle updates each monster that isn't dead (empty/NULL).
    for(int i=0; i<MONSTER_CAP; i++)
    {
        if(monster[i]!=NULL)
        {
            /// Change damage texture.
            if(monster[i]->getDamageTextureOn()){monster[i]->damageTextureCountDown(monsterTexture, dt);}

            switch(monster[i]->getState())
            {
                // Spawning state.
                case 0:
                break;

                // Idle state.
                case 1:
                {
                    /// Checks if the monster is ready to die.
                    isDead(i, player);
                    isPlayerNear(i, player->getNode());

                    if(monster[i]!=NULL)
                    {
                        if(monster[i]->getVitality()<=0) // If the monster's health is less than or equal to zero...
                        {
                            monster[i]->setState(DYING);
                            monster[i]->getNode()->setLoopMode(false);
                            monster[i]->getNode()->setFrameLoop(100, 160);
                        }
                    }

                    if(monster[i]->getRest()<=0)
                    {
                        core::vector3df destination=getPointInCircle(monster[i]->getNode()->getPosition(), 200);
                        monster[i]->setDestination(destination);
                        monster[i]->setForwardVector(destination);

                        monster[i]->getNode()->setLoopMode(true);
                        monster[i]->getNode()->setFrameLoop(20, 100);
                        monster[i]->setState(MOVING);
                    }
                    else
                    {
                        float r=monster[i]->getRest();
                        monster[i]->setRest(r-=1*dt);
                    }
                }
                break;

                // Moving state.
                case 2:
                {
                    /// Checks if the monster is ready to die.
                    isDead(i, player);
                    isPlayerNear(i, player->getNode());

                    float distance = pow(monster[i]->getDestination().X-monster[i]->getNode()->getPosition().X, 2) + pow(monster[i]->getDestination().Z-monster[i]->getNode()->getPosition().Z, 2);
                    if(distance < 1)
                    {
                        monster[i]->getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
                        monster[i]->setRest(5);
                        monster[i]->getNode()->setLoopMode(true);
                        monster[i]->getNode()->setFrameLoop(0, 0);
                        monster[i]->setState(IDLE);
                    }
                    else
                    {
                        monster[i]->moveMonster(dt);
                        monster[i]->SynchPosition();
                    }
                }
                break;

                // Chasing state.
                case 3:
                {
                    isDead(i, player);
                    float distance=pow(player->getNode()->getPosition().X-monster[i]->getNode()->getPosition().X, 2) +
                                    pow(player->getNode()->getPosition().Z-monster[i]->getNode()->getPosition().Z, 2);

                    if(distance < 500000)
                    {
                        monster[i]->setDestination(player->getNode()->getPosition());
                        monster[i]->setForwardVector(player->getNode()->getPosition());
                        monster[i]->moveMonster(dt);
                        monster[i]->SynchPosition();
                    }
                    else{monster[i]->setState(MOVING);}

                    if(player->getNode()->getTransformedBoundingBox().intersectsWithBox(monster[i]->getNode()->getTransformedBoundingBox()))
                    {monster[i]->setState(ATTACKING);}
                }
                break;

                // Attacking state.
                case 4:
                isDead(i, player);
                if(!player->getNode()->getTransformedBoundingBox().intersectsWithBox(monster[i]->getNode()->getTransformedBoundingBox())){monster[i]->setState(CHASING);}
                else
                {
                    float hp=player->getHealth();
                    hp-=(5.0f/1.0f)*dt;
                    player->setHealth(hp);
                }
                break;

                // Bury state.
                case 6:
                delete monster[i];
                monster[i]=NULL;
                break;

                default:
                break;
            }
        }
    }

    /// Spawn new monsters in the pool if there is enough space
    if(cooldown<=0)
    {
        cooldown=20;
        for(int i=0; i<MONSTER_CAP; i++)
        {
            if(monster[i]==NULL)
            {
                monster[i]=new Monster(dev, dynamicsWorld, monsterMesh, monsterTexture, spawnNode);
                monster[i]->setAnimationCallback(monster[i]);
                break;
            }
        }
    }
    else
    {
        /**lower countdown value**/
        cooldown=cooldown-1*dt;
    }
}

Monster* Spawn::getMonsters(int i)
{
    return monster[i];
}

void Spawn::isDead(int element, Player *p)
{
    if(monster[element]!=NULL)
    {
        if(monster[element]->getVitality()<=0) // If the monster's health is less than or equal to zero...
        {
            monster[element]->getRigidBody()->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
            monster[element]->setState(DYING);
            monster[element]->getNode()->setLoopMode(false);
            monster[element]->getNode()->setFrameLoop(100, 160);
            int random=rand()%4;
            p->setInventoryItem(random, p->getInventoryItemValue(random)+1);
        }
    }
}

void Spawn::isPlayerNear(int element, scene::IMeshSceneNode *p)
{
    float distance=pow(p->getPosition().X-monster[element]->getNode()->getPosition().X, 2)+pow(p->getPosition().Z-monster[element]->getNode()->getPosition().Z, 2);
    if(distance < 500000)
    {
        monster[element]->getNode()->setLoopMode(true);
        monster[element]->getNode()->setFrameLoop(20, 100);
        monster[element]->setState(CHASING);
    }
}
