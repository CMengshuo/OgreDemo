#ifndef _GameObject_h_
#define _GameObject_h_

#include <OgreEntity.h>
#include <OgreAnimationState.h>
#include <OgreSkeleton.h>
#include <vector>
#include "CollisionHandler.h"
#include "StateHandler.h"
#include "GameElement.h"

//abstract class
class GameObject: public GameElement{
protected:
	Ogre::String m_strMeshName;
	Ogre::Entity* m_entity;
	static size_t ms_uID;
	size_t m_uMyID; 
	ELEMENT_TYPE m_eMyType;
	std::vector<ELEMENT_TYPE> m_vTargetType; //used when seek for certain type of objects
	TerrainCollisionHandler* m_terrainCollider; //collider used to test terrain collision
	bool m_bUpdating; //If the object needs update
	bool m_bRotating; //if the object is rotating
	bool m_bSupported; //if the player is supported by objects under it, like terrain
	GameElement* m_target;	//target to trace
	Ogre::Real m_rLastCheckTime; //Time since Last check for the collision
	Ogre::Vector3 m_vSpeed;
	Ogre::Real m_rVelocity;
	Ogre::Real m_rRotateVelocity;
	Ogre::AnimationState* m_animationState;
	Ogre::AnimationState* m_additionalAnimationState;
	Ogre::String m_strSkeletonName; 
	Ogre::Degree m_rotateDegreePerFrame;
	Ogre::Real m_rHitPoint;// indestructible when m_rHitPoint equals -1000 
	Ogre::Real m_rMaxHitPoint;

public:
	GameObject();
	virtual ~GameObject() = 0;
	virtual Ogre::String getName();
	virtual Ogre::Entity* getEntity();
	virtual ELEMENT_TYPE getType();
	virtual size_t getID();
	virtual void setVisible();
	virtual bool update(const Ogre::FrameEvent& evt);
	virtual bool gravityCheck();//Deal with gravity
	virtual void startAnimation(Ogre::String AnimationName, bool loop);	//Animation setting
	virtual void startAnimation(Ogre::String AnimationName, Ogre::String AdditionalAnimationName, bool loop);	//Animation setting
	virtual void stopAnimation();
	virtual bool checkCollision(float deta);
	virtual void sendEvent(EVENT event, STATE state);
	virtual void processEvent(EVENT event, STATE state);
	virtual void perceiveWorld(); //make elements know about terrain, or sky info
	virtual void setRotateDegreePerFrame(Ogre::Degree degree);
	virtual void setDamage(Ogre::Real dmg);
	virtual Ogre::Real getHitPoint();
	virtual Ogre::Real getMaxPoint();

protected:
	virtual void _initialize();
	virtual void _traceTarget(GameElement* target, float deta);
	virtual void _colliderSetting();
	virtual void _mergeSkeletonToMesh(); //Merge multiple skeletons to one single mesh
};

#endif