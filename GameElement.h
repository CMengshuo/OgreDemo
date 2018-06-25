#pragma once
#include <OgreSceneManager.h>

class StateHandler;
enum STATE;
enum EVENT;

#define PLAYER_MASK 1<<1
#define MONSTER_MASK 1<<2
#define OBSTACLE_MASK 1<<3

#define  ROTATE_CONST 0.1
#define TERRAIN_CHECK_HEIGHT 5000
#define DETECT_SPAN 0.2

enum ELEMENT_TYPE{
	TYPE_NULL, 
	TYPE_NORMALMONSTER,
	TYPE_JUMPMONSTER,
	TYPE_PLAYER,
	TYPE_SWAPWALKER,
	TYPE_ATTACH_EFFECT,
	TYPE_TRACK_EFFECT,
	TYPE_NORMAL_EFFECT,
};

class GameElement{
public:
	GameElement(){}
	virtual ~GameElement(){};

protected:
	Ogre::SceneManager* m_sceneMgr;
	Ogre::SceneNode* m_sceneNode;
	STATE m_eCurrentMainState;
	StateHandler* m_stateHandler;
	Ogre::Vector3 m_vSpeed;
	bool m_bActive; // if element is active

public:
	virtual void sendEvent(EVENT event, STATE state) = 0;
	virtual void processEvent(EVENT event, STATE state) = 0;
	virtual Ogre::Vector3 getPosition(){return m_sceneNode->_getDerivedPosition();}
	virtual Ogre::Vector3 getDirection(){return (m_sceneNode->_getDerivedOrientation()*Ogre::Vector3::UNIT_Z).normalisedCopy();}
	virtual void setPosition(const Ogre::Vector3& pos){m_sceneNode->_setDerivedPosition(pos);}
	virtual Ogre::SceneNode* getNode(){return m_sceneNode;}
	virtual void setScale(const Ogre::Vector3& scale){m_sceneNode->setScale(scale);};
	virtual Ogre::Vector3 getSpeed(){return m_vSpeed;}
	virtual bool isActive(){return m_bActive;}
	virtual Ogre::String getName() = 0;
	virtual void perceiveWorld() = 0;
};