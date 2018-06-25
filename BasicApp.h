//------------------------------------------------------------------------------------------------
//I set up game framework here. But this class will not deal with game logic 
//Game logic will be defined in the GameManager class
//Considering build a environment manager to make the environment setup easier
//	-----------------------------------------------------------------------------------------------
#ifndef _BasicApp_h_
#define _BasicApp_h_

#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreWindowEventUtilities.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include "myCamController.h"

#define CAM_CHECK_SPAN 0.08

class BasicApp: public Ogre::WindowEventListener, public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener
{
protected:
	Ogre::Root* m_Root;
	Ogre::RenderWindow* m_Window;
	Ogre::SceneManager* m_sceneMgr;
	Ogre::Camera* m_camera;
	Ogre::SceneNode* m_camNode;

	Ogre::String m_ResourceCfg;
	Ogre::String m_PluginsCfg;

	OIS::Keyboard* m_Keyboard;
	OIS::Mouse* m_Mouse;
	OIS::InputManager* m_InputManager;

	//Terrain variables
	Ogre::TerrainGlobalOptions* m_TerrainGloabls;
	Ogre::TerrainGroup* m_TerrainGroup;
	bool m_bTerrainImported;

	myCamController* m_camController;
	double m_rCurrentCamTime;//time since last checks camera

public:
	BasicApp(void);
	virtual ~BasicApp(void);
	virtual OIS::Mouse* GetMouse() { return m_Mouse; }
	void go();

protected:
	virtual bool initialConfiguration(); 	//_initialize render window, Ogre root and render system from configuration files. 
	virtual bool loadResources();
	virtual bool _createScene();
	virtual bool _createCamera();
	virtual bool _createViewport();
	virtual bool createFrameListener(); //create input system and moue, keyboard
	//windowEventListener
	virtual void windowClosed(Ogre::RenderWindow* rw);
	virtual void windowResized(Ogre::RenderWindow* rw);
	//KeyboardListener
	virtual bool keyPressed(const OIS::KeyEvent &arg);
	virtual bool keyReleased(const OIS::KeyEvent &arg);
	//MouseListener
	virtual bool mouseMoved(const OIS::MouseEvent &evt);
	virtual bool mouseReleased(const OIS::MouseEvent &evt,OIS::MouseButtonID id);
	virtual bool mousePressed(const OIS::MouseEvent &evt,OIS::MouseButtonID id);
	//FrameEventListener
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual bool destoryScene();
	virtual bool initializeTerrain(); //_initialize Terrain rendering
	virtual bool defineTerrain(long x, long y);
	virtual bool initBlendMaps(Ogre::Terrain* terrain);
	virtual bool configureTerrainDefaults(Ogre::Light* light);
	virtual bool initializeSky(); //_initialize sky rendering
	virtual void testTerrainCollision(); //test collision in non-game mode
};


#endif