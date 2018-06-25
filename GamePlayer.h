#ifndef _GamePlayer_h_
#define _GamePlayer_h_

class GameManager;
#include "GameObject.h"
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

enum DIRECTION{
	DIRECTION_NULL,
	FORWARD,
	BACKFORWARD,
	LEFT,
	RIGHT,
};

enum ATTACKMODE{
	ATK_TRACK_MISSILE,
	ATK_NORMAL_MISSILE,
	ATK_COMBUSTION,
};

class GamePlayer: public GameObject{
public:
	GamePlayer();
	virtual ~GamePlayer();

protected:
	Ogre::SceneNode* m_camNode;
	Ogre::Quaternion* m_qToLeft;
	DIRECTION m_eCurrentDirection;//moving direction
	Ogre::Real m_rPitchSum; //sum of pitch degree
	BoxCollisionHandler* m_boxCollider;
	Ogre::Radian m_rdFOV;
	RayCollisionHandler* m_rayCamCollider;
	GameObject* m_objTarget;
	ATTACKMODE m_eCurrentAttackMode;
	int m_nNormMissleCount;
	int m_nTrackMissleCount;
	int m_nCombustCount;

//#if _DEBUG
	Ogre::ManualObject* m_manuObj;
	Ogre::SceneNode* m_manuNode;
//#endif

protected:
	virtual void _initialize(); //rewrite _initialize method
	virtual bool checkCollision(float deta); 
	virtual void _colliderSetting();
	virtual void _executeAttack(const OIS::MouseEvent &evt);

public:
	virtual bool keyPressed(const OIS::KeyEvent &arg);
	virtual bool keyReleased(const OIS::KeyEvent &arg);
	virtual bool mouseMoved(const OIS::MouseEvent &evt);
	virtual bool mouseReleased(const OIS::MouseEvent &evt,OIS::MouseButtonID id);
	virtual bool mousePressed(const OIS::MouseEvent &evt,OIS::MouseButtonID id);
	virtual bool update(const Ogre::FrameEvent& evt);
	virtual void sendEvent(EVENT event, STATE state);
	virtual void processEvent(EVENT event, STATE state);
	virtual Ogre::SceneNode* getCameraNode();
	virtual void notifyFOVData(Ogre::Radian r);
	virtual void reset();
	virtual void setDamage(Ogre::Real dmg);
	virtual void switchAttackMode(ATTACKMODE eAm);
	virtual int getNormMissleCount(){return m_nNormMissleCount;}
	virtual int getTrackMissleCount(){return m_nTrackMissleCount;}
	virtual int getCombustCount(){return m_nCombustCount;}
	virtual void refreshAmunition(int norm, int track, int combust);
	virtual void camProcess(int cmd, double time);
};

#endif