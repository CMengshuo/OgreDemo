#ifndef _EffectManager_h_
#define _EffectManager_h_

//Game effect handler(particles, mostly)
#include <OgreParticle.h>
#include "Template.h"
#include "GameElement.h"

class GameObject;
enum EFFECT_RESOURCE;
enum STATE;
enum EVENT;
class TerrainCollisionHandler;
class SphereCollisionHandler;

//-----------------------------------------------------GameEffect-----------------------------------------------------------
class GameEffect: public GameElement{
public:
	GameEffect(EFFECT_RESOURCE ersc, const Ogre::Vector3& startPos, Ogre::SceneManager* sceneMgr,Ogre::String tepName);
	virtual ~GameEffect();

protected:
	Ogre::ParticleSystem* m_particles;
	Ogre::String m_strTemplateName;
	static size_t ms_uID;
	size_t m_uMyID;
	EFFECT_RESOURCE m_eMyResource;
	Ogre::Vector3 m_vSpeed;
	Ogre::Real m_rVelocity;
	Ogre::Vector3 m_vRoateAxis;
	Ogre::Real m_rRotateVelocity;
	TerrainCollisionHandler* m_terrainCollider;
	SphereCollisionHandler* m_sphereCollider;
	Ogre::Real m_rDyingTime;
	Ogre::Real m_rTimeToLive;
	ELEMENT_TYPE m_eMyType;
	std::vector<ELEMENT_TYPE> m_vTargetType;

protected:
	virtual void _executeDie();
	virtual void _executeDying();
	virtual bool _checkCollision();

public:
	virtual void play();
	virtual void update(float deta = 0.0f);
	virtual void stop();
	virtual void setRoate(const Ogre::Vector3& axis, const Ogre::Real& wRadian);
	virtual void setSpeed(const Ogre::Vector3& direction, const Ogre::Real& v);
	virtual void reset();
	virtual void sendEvent(EVENT event, STATE state);
	virtual void processEvent(EVENT event, STATE state);
	virtual Ogre::String getName();
	virtual size_t getID();
	virtual ELEMENT_TYPE getType();
	virtual EFFECT_RESOURCE getResourceName();
	virtual void perceiveWorld();
	virtual bool isTerrainHit(); // if hit the ground
	virtual void setTimeToLive(Ogre::Real time);
	virtual void setStartPos(const Ogre::Vector3& pos);
};

//-----------------------------AttachedEffect-------------------------------------------------------------------------------
class AttachedEffect : public GameEffect{
public:
	AttachedEffect(EFFECT_RESOURCE ersc, const Ogre::Vector3& startPos, Ogre::SceneManager* sceneMgr,Ogre::String tepName, GameElement* elt, Ogre::Vector3 offset);
	virtual ~AttachedEffect();
	virtual void update(float deta = 0.0f);

protected:
	Ogre::Vector3 m_vSpeed;
	Ogre::Vector3 m_vOffset;
	GameElement* m_target;

public:
	virtual void setOffset(const Ogre::Vector3& vec);
	virtual void setTarget(GameElement* elt);
};

//-----------------------------TrackEffect----------------------------------------------------------------------
class TrackEffect : public GameEffect{
public:
	TrackEffect(EFFECT_RESOURCE ersc, const Ogre::Vector3& startPos, Ogre::SceneManager* sceneMgr,Ogre::String tepName, GameElement* elt);
	virtual ~TrackEffect();
	virtual void update(float deta = 0.0f);
	virtual void setTarget(GameElement* elt);

protected:
	Ogre::Vector3 m_vSpeed;
	GameElement* m_target;

protected:
	virtual void _trackTarget();
};


//-----------------------------EffectManager-----------------------------------------------------------------------------
class EffectManager : public MySingleton<EffectManager>{
public:
	void registerEffect(EFFECT_RESOURCE ersc,Ogre::String tepName);
	//Either created statically or kinetically, the status of movement would not change over time. 
	size_t addNormalEffect(EFFECT_RESOURCE ersc, const Ogre::Vector3& startPos);
	//Attached to given nodes. 
	size_t addAttachedEffect(EFFECT_RESOURCE ersc, GameElement* target, const Ogre::Vector3& offset);
	//Created in given position, and tracks target.
	size_t addTrackEffect(EFFECT_RESOURCE ersc, const Ogre::Vector3& startPos, GameElement* target);
	//If it is successfully stopped, return true. Stopped effect would go into m_RecyclePool
	void stopEffectByIndex(size_t index);

	void update(float deta); // update effect

	void setEffectSpeed(const size_t id, const Ogre::Vector3& vec, const Ogre::Real& v);
	void setEffectRotate(const size_t id, const Ogre::Vector3& axis, const Ogre::Real& w);
	void setEffectScale(const size_t id, const Ogre::Vector3& scale);
	//set the time that effect could live. The effect would only stop by event on default setting
	void setEffectLivingTime(const size_t id, const Ogre::Real time);

protected:
	EffectManager();
	virtual ~EffectManager();
	friend class MySingleton<EffectManager>;
	GameEffect* _checkRecyclePool(EFFECT_RESOURCE ersc, ELEMENT_TYPE etype);//check recycle pool for resources to avoid memory allocation.
	Ogre::String _getTemplateNameByType(EFFECT_RESOURCE ersc);
	GameEffect* _getPlayingEffectByIndex(size_t index);

protected:
	Ogre::SceneManager* m_sceneMgr;
	//map of effect names to resource names
	std::map<EFFECT_RESOURCE, Ogre::String> m_ResourceMap;
	//pool of inactive effects, for later use
	std::vector<GameEffect*> m_RecyclePool;
	//current playing effects in the system
	std::map<size_t,GameEffect*> m_PlayList;
};

#endif