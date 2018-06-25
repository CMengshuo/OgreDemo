#include "BasicApp.h"
#include <OgreConfigFile.h>
#include <OgreException.h>
#include <OgreViewport.h>
#include <OgreEntity.h>

BasicApp::BasicApp(void)
{
	m_ResourceCfg = Ogre::StringUtil::BLANK;
	m_PluginsCfg = Ogre::StringUtil::BLANK;
	m_Root = NULL;
	m_camera = NULL;
	m_sceneMgr = NULL;
	m_InputManager = NULL;
	m_Keyboard = NULL;
	m_Mouse = NULL;
	m_Window = NULL;
	m_camNode = NULL;
	m_camController = new myCamController();
	m_rCurrentCamTime = 0.0;
}

BasicApp::~BasicApp(void)
{
	Ogre::WindowEventUtilities::removeWindowEventListener(m_Window,this);
	windowClosed(m_Window);
	m_sceneMgr->clearScene();
	delete m_Root;
	delete m_camController;
}

bool BasicApp::initialConfiguration()
{
#if _DEBUG
	m_ResourceCfg = "resources_d.cfg";
	m_PluginsCfg = "plugins_d.cfg";
#else
	m_ResourceCfg = "resources.cfg";
	m_PluginsCfg = "plugins.cfg";
#endif

	m_Root = new Ogre::Root(m_PluginsCfg);

	Ogre::ConfigFile cf;
	cf.load(m_ResourceCfg);

	Ogre::String archName,typeName,secName;

	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();

		Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator iter = settings->begin();
		while(iter != settings->end())
		{
			typeName = iter->first;
			archName = iter->second;
			Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(archName,typeName,archName);
			iter++;		
		}
	}

	if (!(m_Root->restoreConfig() || m_Root->showConfigDialog()))
	{
		return false;
	}

	Ogre::RenderSystem* rs = m_Root->getRenderSystem();
	rs->setConfigOption("Full Screen","No");

	m_Window = m_Root->initialise(true,"WalkingMonster Window");
	m_sceneMgr = m_Root->createSceneManager(Ogre::ST_EXTERIOR_CLOSE);

	return true;
}

bool BasicApp::loadResources(){
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	return true;
}

bool BasicApp::_createCamera()
{
	Ogre::LogManager::getSingleton().logMessage("****Create Camera*****");
	m_camera = m_sceneMgr->createCamera("PlayCam");
	m_camera->setPosition(Ogre::Vector3(0,0,0));
	m_camera->lookAt(Ogre::Vector3(0,0,0));
	m_camera->setNearClipDistance(10);
	m_camera->setFarClipDistance(2000);

	m_camNode = m_sceneMgr->getRootSceneNode()->createChildSceneNode("CamNode",Ogre::Vector3(1683, 50, 2116));
	m_camNode->attachObject(m_camera);

	return true;
}

bool BasicApp::_createViewport()
{
	Ogre::LogManager::getSingleton().logMessage("****Create Viewport*****");
	Ogre::Viewport* vp = m_Window->addViewport(m_camera);
	vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
	m_camera->setAspectRatio(Ogre::Real(vp->getActualWidth())/Ogre::Real(vp->getActualHeight()));
	
	return true;
}

bool BasicApp::_createScene()
{
	Ogre::LogManager::getSingleton().logMessage("****Create Scene*****");

	initializeTerrain();
	initializeSky();

	return true;
}


bool BasicApp::initializeSky()
{
	m_sceneMgr->setSkyDome(true,"Examples/CloudySky",5,8,2000);
	return true;
}

bool BasicApp::initializeTerrain()
{
	Ogre::LogManager::getSingleton().logMessage("****Initializing Terrain*****");

	Ogre::Light* mainLight = m_sceneMgr->createLight("MainLight");
	mainLight->setPosition(30.0f,100.0f,50.0f);

	Ogre::Light *directionLight = m_sceneMgr->createLight("dirLight");
	Ogre::Vector3 lightDir(0.8,0.3,-0.8);
	lightDir.normalise();
	directionLight->setType(Ogre::Light::LT_DIRECTIONAL);
	directionLight->setDirection(lightDir);
	directionLight->setPosition(100, 500, 100);
	directionLight->setDiffuseColour(Ogre::ColourValue::White);
	directionLight->setSpecularColour(Ogre::ColourValue(0,6,0.4,0.3));

	m_sceneMgr->setAmbientLight(Ogre::ColourValue(0.2,0.2,0.2));

	m_TerrainGloabls = OGRE_NEW Ogre::TerrainGlobalOptions();
	m_TerrainGroup = OGRE_NEW Ogre::TerrainGroup(m_sceneMgr, Ogre::Terrain::ALIGN_X_Z, 513,1000.0f);
	m_TerrainGroup->setFilenameConvention(Ogre::String("OgreDeom"), Ogre::String("dat"));
	m_TerrainGroup->setOrigin(Ogre::Vector3::ZERO);

	configureTerrainDefaults(directionLight);

	defineTerrain(0,0); //only have one terrain texture here
	m_TerrainGroup->loadAllTerrains(true); //load the defined terrain instances

	if (m_bTerrainImported)
	{
		Ogre::TerrainGroup::TerrainIterator iter = m_TerrainGroup->getTerrainIterator();
		while (iter.hasMoreElements())
		{
			Ogre::Terrain* t = iter.getNext()->instance;
			initBlendMaps(t);
		}
	}
	m_TerrainGroup->freeTemporaryResources();

	return true;
}

bool BasicApp::createFrameListener()
{
	Ogre::LogManager::getSingleton().logMessage("****Initializing OIS*****");
	OIS::ParamList pl;
	size_t winHwd = 0;

	std::ostringstream winHwdStr;

	m_Window->getCustomAttribute("WINDOW",&winHwd);
	winHwdStr << winHwd;

	pl.insert(std::make_pair(std::string("WINDOW"),winHwdStr.str()));

	m_InputManager = OIS::InputManager::createInputSystem(pl);

	m_Keyboard = static_cast<OIS::Keyboard*>(m_InputManager->createInputObject(OIS::OISKeyboard,true));
	m_Mouse = static_cast<OIS::Mouse*>(m_InputManager->createInputObject(OIS::OISMouse,true));

	m_Keyboard->setEventCallback(this);
	m_Mouse->setEventCallback(this);

	return true;
}

void BasicApp::go(){
	initialConfiguration();

	Ogre::TextureManager::getSingletonPtr()->setDefaultNumMipmaps(5);

	loadResources();
	createFrameListener();
	_createCamera();
	_createScene();
	_createViewport();

	windowResized(m_Window);

	Ogre::WindowEventUtilities::addWindowEventListener(m_Window, this);
	m_Root->addFrameListener(this);

	//open a training window before game starts
	m_camController->runTraining();
	//m_camController->openCamera();
	m_Root->startRendering();

	//when the rendering loop ends, clean up
	destoryScene();
}

bool BasicApp::frameRenderingQueued(const Ogre::FrameEvent& evt){
	if (m_Window->isClosed())
	{
		return false;
	}
	
	m_Keyboard->capture();
	m_Mouse->capture();

	//m_camNode->translate(mDirection*evt.timeSinceLastFrame,Ogre::Node::TS_LOCAL);

	//testTerrainCollision();
	
	return true;
}

void BasicApp::testTerrainCollision()
{
	Ogre::Vector3 camPos = m_camNode->getPosition();
	Ogre::Ray camRay(Ogre::Vector3(camPos.x,5000,camPos.z),Ogre::Vector3::NEGATIVE_UNIT_Y);

	Ogre::TerrainGroup::RayResult rayResult = m_TerrainGroup->rayIntersects(camRay);

	if (rayResult.hit)
	{
		Ogre::Real intersectHeight = rayResult.position.y;
		if ((intersectHeight + 10.0) > camPos.y)
		{
			camPos.y = intersectHeight + 10.0;
			m_camNode->setPosition(camPos);
		}
	}
}

void BasicApp::windowClosed(Ogre::RenderWindow* rw)
{
	if (rw == m_Window)
	{
		if (m_InputManager)
		{
			m_InputManager->destroyInputObject(m_Keyboard);
			m_InputManager->destroyInputObject(m_Mouse);
			OIS::InputManager::destroyInputSystem(m_InputManager);
		}
	}
}

bool BasicApp::keyPressed(const OIS::KeyEvent &arg)
{
	int key = arg.key;
	switch(key)
	{
	//case OIS::KC_ESCAPE:
		//m_bShutdown = true;
		//break;
	//case OIS::KC_UP:
	//case OIS::KC_W:
	//	mDirection.z = -mMove;
	//	break;

	//case OIS::KC_S:
	//case OIS::KC_DOWN:
	//	mDirection.z = mMove;
	//	break;

	//case OIS::KC_A:
	//case OIS::KC_LEFT:
	//	mDirection.x = -mMove;
	//	break;

	//case OIS::KC_D:
	//case OIS::KC_RIGHT:
	//	mDirection.x = mMove;
	//	break;

	case OIS::KC_R : 
		Ogre::PolygonMode pm;

		switch (m_camera->getPolygonMode())
		{
		case Ogre::PM_SOLID:
			pm = Ogre::PM_WIREFRAME;
			break;
		case Ogre::PM_WIREFRAME:
			pm = Ogre::PM_POINTS;
			break;
		default:
			pm = Ogre::PM_SOLID;
		}

		m_camera->setPolygonMode(pm);
		break;
	default:
		break;
	}

	return true;
}

bool BasicApp::keyReleased(const OIS::KeyEvent &arg)
{
	int key = arg.key;
	/*switch(key)
	{
	case OIS::KC_UP:
	case OIS::KC_W:
	mDirection.z = 0;
	break;

	case OIS::KC_S:
	case OIS::KC_DOWN:
	mDirection.z = 0;
	break;

	case OIS::KC_A:
	case OIS::KC_LEFT:
	mDirection.x = 0;
	break;

	case OIS::KC_D:
	case OIS::KC_RIGHT:
	mDirection.x = 0;
	break;

	default:
	break;
	}*/

	return true;
}

bool BasicApp::destoryScene()
{
	OGRE_DELETE m_TerrainGloabls;
	OGRE_DELETE m_TerrainGroup;
	return true;
}

bool BasicApp::mouseMoved(const OIS::MouseEvent &evt)
{
	//m_camNode->yaw(Ogre::Degree(-mRotate*evt.state.X.rel),Ogre::Node::TS_WORLD);
	//m_camNode->pitch(Ogre::Degree(-mRotate*evt.state.Y.rel),Ogre::Node::TS_LOCAL);

	return true;
}

bool BasicApp::mousePressed(const OIS::MouseEvent &evt,OIS::MouseButtonID id)
{
	return true;
}

bool BasicApp::mouseReleased(const OIS::MouseEvent &evt,OIS::MouseButtonID id)
{
	return true;
}

void BasicApp::windowResized(Ogre::RenderWindow* rw)
{
	unsigned width,height,depth;
	int top,left;

	rw->getMetrics(width,height,depth,left,top);
	
	const OIS::MouseState &ms = m_Mouse->getMouseState();
	ms.width = width;
	ms.height = height;
}

void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
	img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	if (flipX)
		img.flipAroundX();
	if (flipY)
		img.flipAroundY();
}

bool BasicApp::defineTerrain(long x, long y)
{
	Ogre::String filename = m_TerrainGroup->generateFilename(x,y);
	if (Ogre::ResourceGroupManager::getSingleton().resourceExists(m_TerrainGroup->getResourceGroup(),filename))
	{
		m_TerrainGroup->defineTerrain(x,y);
	}
	else{
		Ogre::Image img;
		getTerrainImage(x % 2 != 0,  y % 2 != 0, img);
		m_TerrainGroup->defineTerrain(x, y, &img);
		m_bTerrainImported = true;
	}

	return true;
}

bool BasicApp::configureTerrainDefaults(Ogre::Light* light)
{
	m_TerrainGloabls->setMaxPixelError(8); //Configure globals

	m_TerrainGloabls->setCompositeMapDistance(3000); // testing composite map

	m_TerrainGloabls->setLightMapDirection(light->getDerivedDirection());
	m_TerrainGloabls->setCompositeMapAmbient(m_sceneMgr->getAmbientLight());
	m_TerrainGloabls->setCompositeMapDiffuse(light->getDiffuseColour());

	//configure default setting for import
	Ogre::Terrain::ImportData& defaultimp = m_TerrainGroup->getDefaultImportSettings();
	defaultimp.terrainSize = 513;
	defaultimp.worldSize = 1000.0f;
	defaultimp.inputScale = 50;
	defaultimp.minBatchSize = 33;
	defaultimp.maxBatchSize = 65;

	//textures
	defaultimp.layerList.resize(3);
	defaultimp.layerList[0].worldSize = 100;
	defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
	defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");

	defaultimp.layerList[1].worldSize = 100;
	defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
	defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");

	defaultimp.layerList[2].worldSize = 200;
	defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
	defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");


	return true;
}

bool BasicApp::initBlendMaps(Ogre::Terrain* terrain)
{
	Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
	Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
	Ogre::Real minHeight0 = 70;
	Ogre::Real fadeDist0 = 40;
	Ogre::Real minHeight1 = 70;
	Ogre::Real fadeDist1 = 15; 
	float* pBlend0 = blendMap0->getBlendPointer();
	float* pBlend1 = blendMap1->getBlendPointer();
	for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
	{
		for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
		{
			Ogre::Real tx, ty;

			blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
			Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
			Ogre::Real val = (height - minHeight0) / fadeDist0;
			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
			*pBlend0++ = val;

			val = (height - minHeight1) / fadeDist1;
			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
			*pBlend1++ = val;
		}
	}
	blendMap0->dirty();
	blendMap1->dirty();
	blendMap0->update();
	blendMap1->update();


	return true;
}