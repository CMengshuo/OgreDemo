#ifndef _Monster_h_
#define _Monster_h_
#include "Template.h"
#include "GameObject.h"

//------------------------------------MonsterManager------------------------------------------------------------------------
class MonsterManager : public MySingleton<MonsterManager>{
public:
	GameObject* addMonster(ELEMENT_TYPE type);
	void addMonster(GameObject* monster);
	GameObject* getMonsterByID(size_t id);
	GameObject* getMonsterByName(Ogre::String name);
	bool update(const Ogre::FrameEvent& evt);

protected:
	MonsterManager();
	virtual ~MonsterManager();
	void _disableMonster(size_t id); //do not delete monster, put them into recycle pool for later use
	GameObject* _checkRecyclePool(ELEMENT_TYPE type);

private:
	friend class MySingleton<MonsterManager>;
	std::map<size_t, GameObject*> m_monsterList;
	std::vector<GameObject*> m_recyclePool; // vector to store disabled monster
	Ogre::SceneManager* m_sceneMgr;
};

//------------------------------------------Abstract Monster class--------------------------------------------
class Monster: public GameObject{
public:
	Monster();
	virtual ~Monster() = 0;

protected:
	Ogre::String m_strAttckAction;
	Ogre::String m_strWalkAction;
	Ogre::String m_strWalkActionTop;
	Ogre::String m_strDeadAction;
	//SphereCollisionHandler* m_sphereEvasionCollider;
	SphereCollisionHandler* m_enemyCheckCollider;
	BoxCollisionHandler* m_evadeCheckCollider;
	ELEMENT_TYPE m_eEnemyType;
	Ogre::Real m_rAttackRange;
	Ogre::Real m_rAwareDegree;
	Ogre::Real m_rAwareRange;
	Ogre::Real m_rTolerentAttackAngle;
	Ogre::Real m_rAttackTime;
	Ogre::Real m_rAttackPower;
	bool m_bFadeOut;
	Ogre::Real m_rFadeOutFactor;

protected:
	virtual bool _checkWorldBound(float deta);
	virtual bool checkCollision(float deta);
	virtual void _executeAttack();
	virtual void _executeTrack();
	virtual void _executeWalk();
	virtual void _executeEvade();
	virtual void _executeDie();
	virtual void _executeIdle();
	virtual void _executeDying();
	virtual void _executeTurnBack();
	virtual bool _evasionCheck(float deta); //if need evasion
	virtual bool _enemyCheck(); //if enemy is around
	virtual bool _attackCheck(float deta); // if could attack
	virtual void _initialize();
	virtual void _colliderSetting();
	//virtual void _mergeSkeletonToMesh(); //Merge multiple skeletons to one single mesh

public:
	virtual bool update(const Ogre::FrameEvent& evt);
	virtual void processEvent(EVENT event, STATE state);
	virtual void reset();
};

//-----------------------------------Soilder Monster--------------------------------------------------------------------------

class SoilderMonster: public Monster{
public:
	SoilderMonster();
	virtual ~SoilderMonster();
	virtual void reset();
};

//----------------------------------JumpMonster Monster---------------------------------------------------------------------------

class JumpMonster:public Monster{
public:
	JumpMonster();
	virtual ~JumpMonster();
	virtual void reset();

protected:
	Ogre::String m_strRunAction;
};

#endif

