/*
	This source file is part of Rigs of Rods
	Copyright 2005-2012 Pierre-Michel Ricordel
	Copyright 2007-2012 Thomas Fischer
	Copyright 2013-2014 Petr Ohlidal

	For more information, see http://www.rigsofrods.com/

	Rigs of Rods is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 3, as
	published by the Free Software Foundation.

	Rigs of Rods is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Rigs of Rods. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SkyXManager.h"

#include "Actor.h"
#include "AppContext.h"
#include "CameraManager.h"
#include "GameContext.h"
#include "GfxScene.h"
#include "HydraxWater.h"
#include "Terrain.h"
#include "TerrainGeometryManager.h"

using namespace Ogre;
using namespace RoR;

SkyXManager::SkyXManager(Ogre::String configFile)
{
	InitLight();

	//Ogre::ResourceGroupManager::getSingleton().addResourceLocation("..\\resource\\SkyX\\","FileSystem", "SkyX",true); //Temp

	mBasicController = new SkyX::BasicController();
	mSkyX = new SkyX::SkyX(App::GetGfxScene()->GetSceneManager(), mBasicController);

	mCfgFileManager = new SkyX::CfgFileManager(mSkyX, mBasicController, App::GetCameraManager()->GetCamera());
	mCfgFileManager->load(configFile);

	mSkyX->create();

	RoR::App::GetAppContext()->GetOgreRoot()->addFrameListener(mSkyX);
	RoR::App::GetAppContext()->GetRenderWindow()->addListener(mSkyX);
}

SkyXManager::~SkyXManager()
{
    RoR::App::GetAppContext()->GetRenderWindow()->removeListener(mSkyX);
    mSkyX->remove();

    mSkyX = nullptr;

    delete mBasicController;
    mBasicController = nullptr;
}

Vector3 SkyXManager::getMainLightDirection()
{
	if (mBasicController != nullptr)
		return mBasicController->getSunDirection();
	return Ogre::Vector3(0.0,0.0,0.0);
}

Light *SkyXManager::getMainLight()
{
	return mLight1;
}

bool SkyXManager::update(float dt)
{
	UpdateSkyLight();
	mSkyX->update(dt);
	return true;
}


bool SkyXManager::UpdateSkyLight()
{
	Ogre::Vector3 lightDir = -getMainLightDirection();
	const Ogre::Vector3 camPos = App::GetCameraManager()->GetCameraNode()->_getDerivedPosition();
	Ogre::Vector3 sunPos = camPos - lightDir*mSkyX->getMeshManager()->getSkydomeRadius(App::GetCameraManager()->GetCamera());

	// Calculate current color gradients point
	float point = (-lightDir.y + 1.0f) / 2.0f;

    if (App::GetGameContext()->GetTerrain()->getHydraxManager ()) 
    {
        App::GetGameContext()->GetTerrain()->getHydraxManager ()->GetHydrax ()->setWaterColor (mWaterGradient.getColor (point));
        App::GetGameContext()->GetTerrain()->getHydraxManager ()->GetHydrax ()->setSunPosition (sunPos*0.1);
    }
		

	mLight0 = App::GetGfxScene()->GetSceneManager()->getLight("Light0");
	mLight1 = App::GetGfxScene()->GetSceneManager()->getLight("Light1");

	mLight0->getParentSceneNode()->setPosition(sunPos*0.02);
	mLight1->getParentSceneNode()->setDirection(lightDir);
    if (App::GetGameContext()->GetTerrain()->getWater())
    {
        App::GetGameContext()->GetTerrain()->getWater()->WaterSetSunPosition(sunPos*0.1);
    }

	//setFadeColour was removed with https://github.com/RigsOfRods/rigs-of-rods/pull/1459
/*	Ogre::Vector3 sunCol = mSunGradient.getColor(point);
	mLight0->setSpecularColour(sunCol.x, sunCol.y, sunCol.z);
	if (App::GetGameContext()->GetTerrain()->getWater()) App::GetGameContext()->GetTerrain()->getWater()->setFadeColour(Ogre::ColourValue(sunCol.x, sunCol.y, sunCol.z));
	*/
	Ogre::Vector3 ambientCol = mAmbientGradient.getColor(point);
	mLight1->setDiffuseColour(ambientCol.x, ambientCol.y, ambientCol.z);
	mLight1->getParentSceneNode()->setPosition(100,100,100);

	if (mBasicController->getTime().x > 12)
	{
		if (mBasicController->getTime().x > mBasicController->getTime().z)
			mLight0->setVisible(false);
		else
			mLight0->setVisible(true);
	} 
    else
    {
        if (mBasicController->getTime ().x < mBasicController->getTime ().z)
            mLight0->setVisible (false);
        else
            mLight0->setVisible (true);
    }
	
    if (round (mBasicController->getTime ().x) != mLastHour)
    {
        TerrainGeometryManager* gm = App::GetGameContext()->GetTerrain()->getGeometryManager ();
        if (gm)
            gm->updateLightMap ();

        mLastHour = round (mBasicController->getTime ().x);
    }

	return true;
}

bool SkyXManager::InitLight()
{
	// Water
	mWaterGradient = SkyX::ColorGradient();
	mWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.779105)*0.4, 1));
	mWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.729105)*0.3, 0.8));
	mWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.679105)*0.25, 0.6));
	mWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.679105)*0.2, 0.5));
	mWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.679105)*0.1, 0.45));
	mWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.679105)*0.025, 0));
	// Sun
	mSunGradient = SkyX::ColorGradient();
	mSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.8,0.75,0.55)*1.5, 1.0f));
	mSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.8,0.75,0.55)*1.4, 0.75f));
	mSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.8,0.75,0.55)*1.3, 0.5625f));
	mSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.6,0.5,0.2)*1.5, 0.5f));
	mSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.5,0.5,0.5)*0.25, 0.45f));
	mSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.5,0.5,0.5)*0.01, 0.0f));
	// Ambient
	mAmbientGradient = SkyX::ColorGradient();
	mAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*1, 1.0f));
	mAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*1, 0.6f));
	mAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.6, 0.5f));
	mAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.3, 0.45f));
	mAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.1, 0.35f));
	mAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.05, 0.0f));

	App::GetGfxScene()->GetSceneManager()->setAmbientLight(ColourValue(0.35,0.35,0.35)); //Not needed because terrn2 has ambientlight settings

	// Light
	mLight0 = App::GetGfxScene()->GetSceneManager()->createLight("Light0");
	mLight0->setDiffuseColour(1, 1, 1);
	mLight0->setCastShadows(false);

	mLight1 = App::GetGfxScene()->GetSceneManager()->createLight("Light1");
	mLight1->setType(Ogre::Light::LT_DIRECTIONAL);

        Ogre::SceneNode* mLight0_snode = RoR::App::GetGfxScene()->GetSceneManager()->getRootSceneNode()->createChildSceneNode();
        mLight0_snode->attachObject(mLight0);

        Ogre::SceneNode* mLight1_snode = RoR::App::GetGfxScene()->GetSceneManager()->getRootSceneNode()->createChildSceneNode();
        mLight1_snode->attachObject(mLight1);

	return true;
}

size_t SkyXManager::getMemoryUsage()
{
	//TODO
	return 0;
}

void SkyXManager::freeResources()
{
	//TODO
}
