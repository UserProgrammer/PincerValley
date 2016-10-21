#include <iostream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "collision_filter.h"
#include "event_receiver.h"
#include "scene_object.h"
#include "fireworks.h"
#include "manager.h"
#include "dummy.h"
#include "spawn.h"
#include "debugDraw.cpp"

#define LOCK_CURSOR true

using namespace std;
using namespace irr;
using namespace scene;
using namespace video;
using namespace gui;
using namespace core;

void UpdateRender(btRigidBody *TObject);
void getHeightfieldData(int terrainSize, ITerrainSceneNode *node, f32 *heightDataPtr, f32 *minHeight, f32 *maxHeight);

int main()
{
    ofstream os("infolog.txt");

    if(os.bad()){
        std::cout << "Error creating file.\n";
        return 1;
    }

    IrrlichtDevice *nDevice=createDevice(EDT_NULL);
    dimension2d<u32> halfRes=nDevice->getVideoModeList()->getDesktopResolution();
    nDevice->drop();

    os << "fullRes Width:  " << halfRes.Width << std::endl;
    os << "fullRes Height: " << halfRes.Height << std::endl;

    halfRes.Width=halfRes.Width/2;
    halfRes.Height=halfRes.Height/2;

    os << "halfRes Width:  " << halfRes.Width << std::endl;
    os << "halfRes Height: " << halfRes.Height << std::endl;
    system("pause");

    IrrlichtDevice *device=createDevice(EDT_OPENGL, dimension2d<u32>(halfRes.Width*2, halfRes.Height*2), 16, false, false, false, 0);
    device->setWindowCaption(L"Game Pre-Alpha");

    /// Initializing Irrlicht.
    ISceneManager *smgr=device->getSceneManager();
    IVideoDriver *driver=device->getVideoDriver();
    IGUIEnvironment *guienv=device->getGUIEnvironment();

    /// Bullet default setup.
    btBroadphaseInterface *m_broadphase=new btDbvtBroadphase();

    btDefaultCollisionConfiguration *m_collisionConfiguration=new btDefaultCollisionConfiguration();
    btCollisionDispatcher *m_dispatcher=new btCollisionDispatcher(m_collisionConfiguration);

    btSequentialImpulseConstraintSolver *solver=new btSequentialImpulseConstraintSolver;

    btDynamicsWorld *m_dynamicsWorld=new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, solver, m_collisionConfiguration);
    btVector3 worldGravity(0.0f, -50.0f, 0.0f);
    m_dynamicsWorld->setGravity(worldGravity);

    /// Set rand() seed.
    srand(time(NULL));

    /// Declaring delta time variables.
    f32 deltaTime;
    u32 then=device->getTimer()->getTime();
    position2d<s32> cursorPosition(halfRes.Width, halfRes.Height);
    float sensitivity=0.08f;
    float throwStrength=0.0f;
    bool attacking=false;

    vector2d<u32> desktopResolution=device->getVideoModeList()->getDesktopResolution();
    vector2d<s32> screenCenter;
    screenCenter.X=desktopResolution.X/2;
    screenCenter.Y=desktopResolution.Y/2;

    /// Adding a terrain mesh to the game via heightmap.
    ITexture *heightMapTexture=driver->getTexture("./assets/map_image_resize.jpg");
    ITerrainSceneNode *map=smgr->addTerrainSceneNode(heightMapTexture->getName(), 0 , -1, vector3df(0.0f, 0.0f, 0.0f), vector3df(0.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f));
    map->setMaterialTexture(0, driver->getTexture("./assets/terrain_texture_map.jpg"));
    map->setMaterialFlag(EMF_LIGHTING, false);

    /// Retrieving heightfield height data from mesh declared above.
    int worldCollidesWith=PLAYER_COLLISION|FIREWORK_COLLISION|MONSTER_COLLISION;
    int arraySize=heightMapTexture->getSize().Width;
    f32 minHeight=0.0f, maxHeight=0.0f;
    f32 heightData[arraySize*arraySize];
    f32 *heightDataPtr=&heightData[0];

    getHeightfieldData(arraySize, map, heightDataPtr, &minHeight, &maxHeight);

    btCollisionShape *groundShape=new btHeightfieldTerrainShape(arraySize, arraySize, heightDataPtr, 1.f, minHeight, maxHeight, 1, PHY_FLOAT, false);
    map->setScale(vector3df(10.0f, 2.6f, 10.0f));
    groundShape->setLocalScaling(btVector3(map->getScale().X-0.1f, map->getScale().Y-0.1f, map->getScale().Z-0.1f));//(btVector3(9.86f, 3.0f, 9.86f));

    /// Ground motion state.
    btTransform transform;
    vector3df irrTerrainTrans=map->getTerrainCenter();
    btVector3 initPos(irrTerrainTrans.X, irrTerrainTrans.Y, irrTerrainTrans.Z);
    transform.setIdentity();
    transform.setOrigin(initPos);
    btDefaultMotionState *groundMotionState=new btDefaultMotionState(transform);

    /// Ground rigid body.
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody *groundRigidBody=new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setUserPointer((void *)map);

    /// Adding ground rigid body to the world.
    m_dynamicsWorld->addRigidBody(groundRigidBody, WORLD_COLLISION, worldCollidesWith);

    /// Adding skydome.
    IMeshSceneNode *skydome=smgr->addMeshSceneNode(smgr->getMesh("./assets/skydome.DAE"));
    skydome->setMaterialFlag(EMF_LIGHTING, false);
    skydome->setMaterialTexture(0, driver->getTexture("./assets/t_skydome.jpg"));
    skydome->setPosition(vector3df(2360.0f, 0.0f, 2360.0f));
    skydome->setRotation(vector3df(-90.0f, 0.0f, 0.0f));
    skydome->setScale(vector3df(50.0f, 50.0f, 50.0f));

    /// Adding water to the world.
    IAnimatedMesh *waterMesh=smgr->addHillPlaneMesh("waterMesh", dimension2d<f32>(150.0f, 150.0f), dimension2d<u32>(35, 35), 0, 0,
                                                    dimension2d<f32>(0.0f, 0.0f), dimension2d<f32>(10.0f, 10.0f));

    ISceneNode *waterNode=smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 5, 300, 14);
    waterNode->setPosition(vector3df(2560.0f, 240.0f, 2560.0f));

    waterNode->setMaterialTexture(0, driver->getTexture("./assets/river_stones_texture.psd"));
	waterNode->setMaterialTexture(1, driver->getTexture("./assets/water.jpg"));
	waterNode->setMaterialType(EMT_REFLECTION_2_LAYER);
	waterNode->setMaterialFlag(EMF_LIGHTING, false);

    /// Setting up player.
    Player player(device, m_dynamicsWorld, smgr->getMesh("./assets/untitled.obj"), driver->getTexture("./assets/player_texture.jpg"),
                  vector3df(1.0f, 0.0f, 0.0f), vector3df(0.0f, 1.0f, 0.0f));
    /// Player setup done!

    /// Event receiver setup.
    SAppContext context;
    context.device=device;
    EventReceiver receiver(context);
    device->setEventReceiver(&receiver);

    device->getCursorControl()->setPosition(position2df(0.5f, 0.5f));

    /// Boolean used to switch between player's first person perspective and the flying camera.
    bool FP_POV=true;
    /// FPS Camera for looking around the map.
    ICameraSceneNode *FPSCamera=smgr->addCameraSceneNodeFPS(0, 150.f, 1.f);
    FPSCamera->setFarValue(10000);
    device->getCursorControl()->setVisible(false);
    smgr->setActiveCamera(player.getCamera());

    /// Setting up debugDraw.
    DebugDraw debugDraw(device);
    debugDraw.setDebugMode(btIDebugDraw::DBG_DrawWireframe |
                            //btIDebugDraw::DBG_DrawAabb |
                            btIDebugDraw::DBG_DrawContactPoints |
                            //btIDebugDraw::DBG_DrawText |
                            //btIDebugDraw::DBG_DrawConstraintLimits |
                            btIDebugDraw::DBG_DrawConstraints);
    m_dynamicsWorld->setDebugDrawer(&debugDraw);

    irr::video::SMaterial debugMat;
    debugMat.Lighting=false;

    bool debug_draw_bullet=false;

    /// Defining the diferent firework types.
    FireworkInfo firework01(3200, 3400, SColor(255, 0, 0, 0), SColor(255, 255, 255, 255), 500, 500,
                            dimension2df(10.0f, 10.0f), dimension2df(20.0f, 20.0f), 360, driver->getTexture("./assets/firework_particles/red_particle.png"));
    FireworkInfo firework02(3200, 3400, SColor(255, 0, 0, 0), SColor(255, 255, 255, 255), 500, 500,
                            dimension2df(10.0f, 10.0f), dimension2df(20.0f, 20.0f), 360, driver->getTexture("./assets/firework_particles/green_particle.png"));
    FireworkInfo firework03(3200, 3400, SColor(255, 0, 0, 0), SColor(255, 255, 255, 255), 500, 500,
                            dimension2df(10.0f, 10.0f), dimension2df(20.0f, 20.0f), 360, driver->getTexture("./assets/firework_particles/blue_particle.png"));
    FireworkInfo firework04(3200, 3400, SColor(255, 0, 0, 0), SColor(255, 255, 255, 255), 500, 500,
                            dimension2df(10.0f, 10.0f), dimension2df(20.0f, 20.0f), 360, driver->getTexture("./assets/firework_particles/purple_particle.png"));
    /// Creating firework manager.
    Manager mngr;

    /// Creating randomly positioned trees.

    int size=40, ray=800, spacing=150;
    bool addTree=false;
    vector3df center(1500.0f, 290.0f, 2500.0f);
    ScenicObject *forest01[size];
    srand((unsigned)time(0));
    btCollisionShape *treeShape=new btBoxShape(btVector3(10, 100, 10));

    /// Set all array elements to NULL.
    for(int i=0; i<size;i++){forest01[i]=NULL;}

    /// Add object to the first element in the array.
    forest01[0]=new ScenicObject(device, m_dynamicsWorld, smgr->getMesh("./assets/forest_tree.DAE"), 0, getPointInCircle(center, ray),
                              vector3df(-90.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f), driver->getTexture("./assets/tree_texture.jpg"), treeShape);
    /// Adding tree branches as child node.
    forest01[0]->addChildNode(device, smgr->getMesh("./assets/forest_tree_branches.DAE"), vector3df(0.0f, 0.0f, 0.0f),
                           vector3df(0.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f), driver->getTexture("./assets/tree_texture.jpg"));

    /// in the first parameter the array element count starts with the second element.
    for(int i=1; i<size; addTree=true)
    {
        /// Get position for new object.

        vector3df point=getPointInCircle(center, ray);
        cout << point.X << endl;
        cout << point.Z << endl;

        for(int n=0; n<size; n++)
        {
            if(forest01[n]!=NULL)
            {
                int dist=sqrt(pow(point.X-forest01[n]->getGraphicsObject()->getPosition().X, 2) + pow(point.Z-forest01[n]->getGraphicsObject()->getPosition().Z, 2));
                if(dist<spacing){addTree=false;}
            }
        }

        if(addTree==true)
        {
            forest01[i]=new ScenicObject(device, m_dynamicsWorld, smgr->getMesh("./assets/forest_tree.DAE"), 0, point,
                              vector3df(-90.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f), driver->getTexture("./assets/tree_texture.jpg"), treeShape);
            forest01[i]->addChildNode(device, smgr->getMesh("./assets/forest_tree_branches.DAE"), vector3df(0.0f, 0.0f, 0.0f),
                           vector3df(0.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f), driver->getTexture("./assets/tree_texture.jpg"));
           cout << i;
            i++;
        }
    }

    size=40;
    ray=700;
    spacing=120;
    addTree=false;
    center=vector3df(3550.0f, 290.0f, 3800.0f);
    ScenicObject *forest02[size];

    /// Set all array elements to NULL.
    for(int i=0; i<size;i++){forest02[i]=NULL;}

    /// Add object to the first element in the array.
    forest02[0]=new ScenicObject(device, m_dynamicsWorld, smgr->getMesh("./assets/forest_tree.DAE"), 0, getPointInCircle(center, ray),
                              vector3df(-90.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f), driver->getTexture("./assets/tree_texture.jpg"), treeShape);
    /// Adding tree branches as child node.
    forest02[0]->addChildNode(device, smgr->getMesh("./assets/forest_tree_branches.DAE"), vector3df(0.0f, 0.0f, 0.0f),
                           vector3df(0.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f), driver->getTexture("./assets/tree_texture.jpg"));

    /// in the first parameter the array element count starts with the second element.
    for(int i=1; i<size; addTree=true)
    {
        /// Get position for new object.
        vector3df point=getPointInCircle(center, ray);

        for(int n=0; n<size; n++)
        {
            if(forest02[n]!=NULL)
            {
                int dist=sqrt(pow(point.X-forest02[n]->getGraphicsObject()->getPosition().X, 2) + pow(point.Z-forest02[n]->getGraphicsObject()->getPosition().Z, 2));
                if(dist<spacing){addTree=false;}
            }
        }

        if(addTree==true)
        {
            forest02[i]=new ScenicObject(device, m_dynamicsWorld, smgr->getMesh("./assets/forest_tree.DAE"), 0, point,
                              vector3df(-90.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f), driver->getTexture("./assets/tree_texture.jpg"), treeShape);
            forest02[i]->addChildNode(device, smgr->getMesh("./assets/forest_tree_branches.DAE"), vector3df(0.0f, 0.0f, 0.0f),
                           vector3df(0.0f, 0.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f), driver->getTexture("./assets/tree_texture.jpg"));
           cout << i;
            i++;
        }
    }

    /// Monster Spawners
    SpawnEssentials ess(device, m_dynamicsWorld);
    Spawn spawn01(ess, smgr->getMesh("./assets/spawn_mesh.DAE"), driver->getTexture("./assets/spawn_texture.jpg"), vector3df(1412.0f, 300.0f, 1512.0f));
    Spawn spawn02(ess, smgr->getMesh("./assets/spawn_mesh.DAE"), driver->getTexture("./assets/spawn_texture.jpg"), vector3df(1412.0f, 300.0f, 3800.0f));


    /// Character animated arm.
    IAnimatedMeshSceneNode *rightArm=smgr->addAnimatedMeshSceneNode(smgr->getMesh("./assets/arm_ani_test.b3d"), player.getNode(), 42);
    rightArm->setLoopMode(true);
    rightArm->setFrameLoop(11, 15);
    rightArm->setAnimationSpeed(15);
    rightArm->setMaterialFlag(EMF_LIGHTING, false);
    rightArm->setMaterialTexture(0, driver->getTexture("./assets/Right_Arm_Texture.psd"));
    rightArm->setPosition(vector3df(-23.0f, -20.0f, 0.0f));
    rightArm->setRotation(vector3df(0.0f, 90.0f, 0.0f));
    rightArm->setScale(vector3df(0.8, 0.8, 0.8));

    AnimationEndCallBack rightArmCallback;
    rightArmCallback.setAttackPtr(&attacking);
    rightArm->setAnimationEndCallback(&rightArmCallback);

    IMeshSceneNode *sword=smgr->addMeshSceneNode(smgr->getMesh("./assets/Sword.DAE"), rightArm->getJointNode("Right_Palm_Bone"));
    sword->setMaterialFlag(EMF_LIGHTING, false);
    sword->setMaterialTexture(0, driver->getTexture("./assets/Sword_Texture_Map.psd"));
    sword->setPosition(vector3df(-8.0f, -5.0f, 0.0f));
    sword->setRotation(vector3df(180.0f, 0.0f, 25.0f));

    /// Setting up village.
    IMeshSceneNode *villageCenter=smgr->addSphereSceneNode(10);
    villageCenter->setPosition(vector3df(1500.0f, 300.0f, 3500.0f));
    villageCenter->updateAbsolutePosition();

    /// First arched building.
    btCollisionShape *archedBuildingShape=new btBoxShape(btVector3(90, 100, 130));
    ScenicObject archedBuilding(device, m_dynamicsWorld, smgr->getMesh("./assets/village/arched_building.DAE"), villageCenter,
                                vector3df(-700.0f, 70.0f, 0.0f), vector3df(-90.0f, 20.0f, 0.0f), vector3df(2.0f, 2.0f, 2.0f),
                                driver->getTexture("./assets/village/textures/arched_building_texture.jpg"), archedBuildingShape);

    /// First house type 01.
    btCollisionShape *house_01_BuildingShape=new btBoxShape(btVector3(100, 50, 50));
    ScenicObject house_01_Building(device, m_dynamicsWorld, smgr->getMesh("./assets/village/house_building_01.DAE"), villageCenter,
                                vector3df(0.0f, 0.0f, 0.0f), vector3df(0.0f, 0.0f, 0.0f), vector3df(2.0f, 2.0f, 2.0f),
                                driver->getTexture("./assets/village/textures/house_building_01_texture.jpg"), house_01_BuildingShape);

    /// First house type 02.
    btCollisionShape *house_02_BuildingShape=new btBoxShape(btVector3(45, 100, 45));
    ScenicObject house_02_Building(device, m_dynamicsWorld, smgr->getMesh("./assets/village/house_building_02.DAE"), villageCenter,
                                vector3df(-500.0f, 0.0f, 300.0f), vector3df(-90.0f, 140.0f, 0.0f), vector3df(1.0f, 1.0f, 1.0f),
                                driver->getTexture("./assets/village/textures/house_building_02_texture.jpg"), house_02_BuildingShape);

    IMeshSceneNode *branch=smgr->addMeshSceneNode(smgr->getMesh("./assets/forest_tree_branch.DAE"));
    branch->setScale(vector3df(1.0f, 1.0f, 1.0f));

    while(device->run())
    {
        /// Delta time update.
        const f32 now=device->getTimer()->getTime();
        deltaTime=(f32)(now-then)/1000.f;
        then=now;

        /// Intput events go here.
        if(receiver.IsKeyDown(KEY_ESCAPE))
        {
            device->closeDevice();
            break;
        }

        if(receiver.isCursorMoving())
        {
            cursorPosition=device->getCursorControl()->getPosition();
            os << "current cursor position\n";
            os << "\t X - " << cursorPosition.X << std::endl;
            os << "\t Y - " << cursorPosition.Y << std::endl;

            if(LOCK_CURSOR)
            {
                if((cursorPosition.X==halfRes.Width && cursorPosition.Y==halfRes.Height) ||
                    (cursorPosition.X==0 && cursorPosition.Y==0)){receiver.resetCursorMove();}
                else
                {
//                    cursorPosition.X-=halfRes.Width;
//                    cursorPosition.Y-=halfRes.Height;
                    device->getCursorControl()->setPosition(position2d<f32>(0.5f, 0.5f));
                    os << "cursor position after reset\n";
                    os << "\t X - " << cursorPosition.X << std::endl;
                    os << "\t Y - " << cursorPosition.Y << std::endl;

                    /// Add the player update function here.
                    player.pRotation(vector2df((cursorPosition.X-halfRes.Width)*sensitivity, (cursorPosition.Y-halfRes.Height)*sensitivity));
                    receiver.resetCursorMove();
                }
            }
        }

        if(receiver.IsKeyPressed(KEY_KEY_C)==1)
        {
            if(FP_POV)
            {
                smgr->setActiveCamera(FPSCamera);
                FP_POV=false;
            }
            else
            {
                smgr->setActiveCamera(player.getCamera());
                FP_POV=true;
            }
        }

        if(receiver.IsKeyDown(KEY_KEY_W)){player.pMovement(deltaTime, FORWARD);}
		if(receiver.IsKeyDown(KEY_KEY_S)){player.pMovement(deltaTime, BACKWARDS);}

		if(receiver.IsKeyDown(KEY_KEY_A)){player.pMovement(deltaTime, LEFT);}
		if(receiver.IsKeyDown(KEY_KEY_D)){player.pMovement(deltaTime, RIGHT);}

        if(receiver.IsKeyDown(KEY_KEY_1)){player.setSelectedFW(TYPE01);}
        if(receiver.IsKeyDown(KEY_KEY_2)){player.setSelectedFW(TYPE02);}
        if(receiver.IsKeyDown(KEY_KEY_3)){player.setSelectedFW(TYPE03);}
        if(receiver.IsKeyDown(KEY_KEY_4)){player.setSelectedFW(TYPE04);}

		if(receiver.IsKeyDown(KEY_PLUS)){player.setInventoryItem(player.getSelectedFW(), player.getInventoryItemValue(player.getSelectedFW())+1);}
		if(receiver.IsKeyDown(KEY_MINUS)){player.setInventoryItem(player.getSelectedFW(), player.getInventoryItemValue(player.getSelectedFW())-1);}

        if(receiver.IsMouseButtonPressed(M_LEFT)==1)
        {
            if(!attacking)
            {
                attacking=true;
                rightArm->setLoopMode(false);
                rightArm->setFrameLoop(15, 25);
                for(int i=0; i<MONSTER_CAP; i++)
                {
                    if(spawn01.getMonsters(i)!=NULL)
                    {
                        bool intersect=sword->getTransformedBoundingBox().intersectsWithBox(spawn01.getMonsters(i)->getNode()->getTransformedBoundingBox());
                        if(intersect)
                        {
                            spawn01.getMonsters(i)->setDamageTexture(driver->getTexture("./assets/damage_texture.jpg"));
                            spawn01.getMonsters(i)->removeVitality(5);
                        }
                    }

                    if(spawn02.getMonsters(i)!=NULL)
                    {
                        bool intersect=sword->getTransformedBoundingBox().intersectsWithBox(spawn02.getMonsters(i)->getNode()->getTransformedBoundingBox());
                        if(intersect)
                        {
                            spawn02.getMonsters(i)->setDamageTexture(driver->getTexture("./assets/damage_texture.jpg"));
                            spawn02.getMonsters(i)->removeVitality(5);
                        }
                    }
                }
            }
        }

		if(receiver.IsMouseButtonDown(M_RIGHT))
		{
		    throwStrength+=10.0f;
        }
		if(receiver.IsMouseButtonPressed(M_RIGHT)==0)
		{
		    int selectedFirework=player.getSelectedFW();
		    if(player.getInventoryItemValue(player.getSelectedFW())>0)
		    {
                switch(selectedFirework)
                {
                    case 0:
                    mngr.addFirework(device, firework01, m_dynamicsWorld, driver->getTexture("./assets/spawn_texture.jpg"),
                    player.getCamera()->getTarget(), player.getForwardVector(), throwStrength);
                    break;

                    case 1:
                    mngr.addFirework(device, firework02, m_dynamicsWorld, driver->getTexture("./assets/spawn_texture.jpg"),
                    player.getCamera()->getTarget(), player.getForwardVector(), throwStrength);
                    break;

                    case 2:
                    mngr.addFirework(device, firework03, m_dynamicsWorld, driver->getTexture("./assets/spawn_texture.jpg"),
                    player.getCamera()->getTarget(), player.getForwardVector(), throwStrength);
                    break;

                    case 3:
                    mngr.addFirework(device, firework04, m_dynamicsWorld, driver->getTexture("./assets/spawn_texture.jpg"),
                    player.getCamera()->getTarget(), player.getForwardVector(), throwStrength);
                    break;

                    default:
                    break;
                }

                player.setInventoryItem(selectedFirework, player.getInventoryItemValue(selectedFirework)-1);
		    }
            throwStrength=0;
        }

		receiver.resetKeyPressed();
		receiver.resetMousePressed();

        m_dynamicsWorld->stepSimulation(1/60.f, 10);

        /// Update player's camera target.
        player.cUpdateTarget();

        mngr.update(deltaTime);

        /// Sychronize the player's graphics body with the physics body.
        UpdateRender(player.getPlayerRigidBody());

        /// Sychronize monster's graphics and physics bodies.

        /// Redraw firework.
        for(int i=0; i<100; i++){if(mngr.redrawFireworks(i)!=false){UpdateRender(mngr.redrawFireworks(i));}}

        /// Spawner update function
        spawn01.updateMonsters(device, m_dynamicsWorld, &player, smgr->getMesh("./assets/animated_zombie.b3d"),
                             driver->getTexture("./assets/zombie_texture.jpg"), deltaTime);

        spawn02.updateMonsters(device, m_dynamicsWorld, &player, smgr->getMesh("./assets/animated_zombie.b3d"),
                             driver->getTexture("./assets/zombie_texture.jpg"), deltaTime);

        ///Updating player's health bar.
        float health=player.getHealth();
        float healthBarWidth=(500*health)/100;

        /// Clearing GUI Environment each frame so the 2d images and text aren't painted over each other.
        guienv->clear();

        /// Draw everything in the scene.
        driver->beginScene(true, true, SColor(0, 255, 255, 255));
        smgr->drawAll();

        driver->draw3DLine(player.getCamera()->getAbsolutePosition(), player.getCamera()->getAbsolutePosition()+player.getForwardVector(), SColor(255, 255, 0, 0));

        /// User Interface.

        driver->draw2DImage(driver->getTexture("./assets/health_bar.jpg"), position2d<s32>(683-(healthBarWidth/2), 20),
                            rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(healthBarWidth, 20)),
                            0, SColor(255,255,255,255), true);

        driver->draw2DImage(driver->getTexture("./assets/r1Bag.png"), position2d<s32>(1306, 10), rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(50, 50)),
                            0, SColor(255,255,255,255), true);
        guienv->addStaticText(stringw(player.getInventoryItemValue(0)).c_str(), rect<s32>(1325, 60, 1375, 70));
        driver->draw2DImage(driver->getTexture("./assets/r2Bag.png"), position2d<s32>(1306, 80), rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(50, 50)),
                            0, SColor(255,255,255,255), true);
        guienv->addStaticText(stringw(player.getInventoryItemValue(1)).c_str(), rect<s32>(1325, 130, 1375, 140));
        driver->draw2DImage(driver->getTexture("./assets/r3Bag.png"), position2d<s32>(1306, 150), rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(50, 50)),
                            0, SColor(255,255,255,255), true);
        guienv->addStaticText(stringw(player.getInventoryItemValue(2)).c_str(), rect<s32>(1325, 200, 1375, 210));
        driver->draw2DImage(driver->getTexture("./assets/r4Bag.png"), position2d<s32>(1306, 220), rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(50, 50)),
                            0, SColor(255,255,255,255), true);
        guienv->addStaticText(stringw(player.getInventoryItemValue(3)).c_str(), rect<s32>(1325, 270, 1375, 280));

        switch(player.getSelectedFW())
        {
            case 0:
            driver->draw2DImage(driver->getTexture("./assets/output.png"), position2d<s32>(1306, 10), rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(50, 50)),
                                0, SColor(255,255,255,255), true);
            break;

            case 1:
            driver->draw2DImage(driver->getTexture("./assets/output.png"), position2d<s32>(1306, 80), rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(50, 50)),
                                0, SColor(255,255,255,255), true);
            break;

            case 2:
            driver->draw2DImage(driver->getTexture("./assets/output.png"), position2d<s32>(1306, 150), rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(50, 50)),
                                0, SColor(255,255,255,255), true);
            break;

            case 3:
            driver->draw2DImage(driver->getTexture("./assets/output.png"), position2d<s32>(1306, 220), rect<s32>(position2d<s32>(0, 0), dimension2d<s32>(50, 50)),
                                0, SColor(255,255,255,255), true);
            break;

            default:
            break;
        }
        guienv->drawAll();

        if (debug_draw_bullet)
        {
            driver->setMaterial(debugMat);
            driver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);
            m_dynamicsWorld->debugDrawWorld();
        }

//        integer-=1*deltaTime;
//        if(integer<=0)
//        {
//            switch(debug_draw_bullet)
//            {
//                case true:
//                debug_draw_bullet=false;
//                integer=200;
//                break;
//
//                case false:
//                debug_draw_bullet=true;
//                integer=4;
//                break;
//            }
//        }
        driver->endScene();

    }
}

/// Passes bullet's orientation to irrlicht
void UpdateRender(btRigidBody *TObject)
{
	ISceneNode *node=static_cast<ISceneNode *>(TObject->getUserPointer());

    // Get node center
//    aabbox3df dummyBox=node->getTransformedBoundingBox();
//    vector3d<f32> edges=node->getTransformedBoundingBox().getExtent();

	// Set position
	btVector3 Point=TObject->getCenterOfMassPosition();
	node->setPosition(vector3df((f32)Point[0], (f32)Point[1], (f32)Point[2]));

    // Set rotation
    vector3df Euler;
    const btQuaternion &TQuat=TObject->getOrientation();
    quaternion q(TQuat.getX(), TQuat.getY(), TQuat.getZ(), TQuat.getW());
    q.toEuler(Euler);
    Euler *=RADTODEG;
    node->setRotation(Euler);
}

/// Returns heightfield data necessary to create bullet's terrain shape colision object.
void getHeightfieldData(int terrainSize, ITerrainSceneNode *node, f32 *heightDataPtr, f32 *minHeight, f32 *maxHeight)
{
    vector3df scale=node->getScale();
    aabbox3d<f32> box=node->getBoundingBox();
    const vector3df size=box.getExtent()/scale;
    cout << "bounding box extent: " << size.X << "  " << size.Y << "  " << size.Z << endl;
    *minHeight=((ITerrainSceneNode*)node)->getHeight(0, 0);
    *maxHeight=*minHeight;

    const f32 stepWidthX=size.X/terrainSize;
    const f32 stepWidthZ=size.Z/terrainSize;

    vector3df minEdge=box.MinEdge/scale;
    vector3df maxEdge=box.MaxEdge/scale;

    for(f32 z=minEdge.Z; z<maxEdge.Z; z+=stepWidthZ)
    {
        for(f32 x=minEdge.X; x<maxEdge.X; x+=stepWidthX)
        {
            const f32 curVal=((ITerrainSceneNode*)node)->getHeight(x, z);

            heightDataPtr++;
            *heightDataPtr=curVal;

            if(curVal>*maxHeight){*maxHeight=curVal;}
            if(curVal<*minHeight){*minHeight=curVal;}
        }
    }
}
