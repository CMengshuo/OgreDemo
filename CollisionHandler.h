#ifndef _CollisionHandler_h_
#define _CollisionHandler_h_

//Simple collision process
#include <OgreSceneManager.h>
#include <Terrain/OgreTerrainGroup.h>

class GameElement;
enum ELEMENT_TYPE;

//abstract class
class CollisionHandler{
public:
	CollisionHandler();
	virtual ~CollisionHandler() = 0;
	//If it is colliding with the selected target
	virtual bool checkCollision(GameElement* target);
	//If it is colliding certain type of object, get the object's name. If there are many objects, jointed by @
	//check with any type of object's collision if type equals 0
	virtual bool checkCollision(Ogre::String& targetName, const std::vector<ELEMENT_TYPE> &typeVec);
	virtual void setPositionOffset(const Ogre::Vector3& offset);
	virtual bool isValidTarget(const ELEMENT_TYPE &type, const std::vector<ELEMENT_TYPE> &typeVec);
	virtual void setQueryMask(Ogre::uint32 mask);

protected:
	Ogre::SceneManager* m_sceneMgr;
	GameElement* m_owner;
	//offset vector which used to fix the position of either origin of ray or center of sphere or box
	Ogre::Vector3 m_vPositionOffest;
};

class SphereCollisionHandler: public CollisionHandler{
public:
	SphereCollisionHandler(GameElement* elt, Ogre::uint32 mask = 0xFFFFFFFF, Ogre::uint32 typeMask = 0xFFFFFFFF);
	virtual ~SphereCollisionHandler();
	Ogre::Sphere* getSphere();
	void setRadius(Ogre::Real raduis);
	void setCenter(const Ogre::Vector3& center);
	virtual bool checkCollision(GameElement* target);
	virtual bool checkCollision(Ogre::String& targetName, const std::vector<ELEMENT_TYPE> &typeVec);
	virtual void setAwareDegree(Ogre::Real degree);
	virtual void setQueryMask(Ogre::uint32 mask);
	virtual void setSphere(const Ogre::Sphere& sp);

protected:
	//The degree which is valid for detecting objects on horizontal planes
	Ogre::Real m_rAwareDegree;
	Ogre::Sphere* m_sphere;
	Ogre::SphereSceneQuery* m_sceneQuery;

protected:
	bool _isBlindSpot(const Ogre::Vector3& toTargetVec);
};

class RayCollisionHandler: public CollisionHandler{
public:
	//check collisions within the distance given. infinite distance in default setting
	RayCollisionHandler(GameElement* elt, Ogre::uint32 mask = 0xFFFFFFFF, Ogre::uint32 typeMask = 0xFFFFFFFF, Ogre::Real distance = -1);
	virtual ~RayCollisionHandler();
	virtual bool checkCollision(GameElement* target);
	virtual bool checkCollision(Ogre::String& targetName, const std::vector<ELEMENT_TYPE> &typeVec);
	virtual void setQueryMask(Ogre::uint32 mask);
	virtual void setDetectDistance(Ogre::Real distance);
	virtual void setDirection(const Ogre::Vector3& direction);
	virtual Ogre::Ray* getRay();

protected:
	Ogre::Ray* m_ray;
	Ogre::RaySceneQuery* m_sceneQuery;
	Ogre::Vector3 m_vDirection;
	Ogre::Real m_rValidDistance; //returned collision results are based on valid distance, default distance equals -1. Detect anything in the path
};

class BoxCollisionHandler: public CollisionHandler{
public:
	//if useVirtualBox is true, always set the box before calling checkCollision function. If useVirtualBox is false, the collider will use object's bounding box to do collision check
	BoxCollisionHandler(GameElement* elt, Ogre::uint32 mask = 0xFFFFFFFF, Ogre::uint32 typeMask = 0xFFFFFFFF, bool useVirtualBox = false);
	virtual ~BoxCollisionHandler();
	Ogre::AxisAlignedBox& getBox();
	virtual bool checkCollision(GameElement* target);
	virtual bool checkCollision(Ogre::String& targetName, const std::vector<ELEMENT_TYPE> &typeVec);
	virtual void setScale(const Ogre::Vector3& scale);
	virtual void setBox(const Ogre::AxisAlignedBox& box);
	virtual void setQueryMask(Ogre::uint32 mask);

protected:
	Ogre::AxisAlignedBoxSceneQuery* m_sceneQuery;
	Ogre::AxisAlignedBox m_box;
	Ogre::Vector3 m_vScale;
	bool m_bUseVirtualBox; 
};


class TerrainCollisionHandler: public CollisionHandler{
public:
	TerrainCollisionHandler(GameElement* elt, Ogre::TerrainGroup* tGroup);
	virtual ~TerrainCollisionHandler();
	//If it is colliding with terrain, get the colliding position
	bool checkTerrainCollision(Ogre::Vector3& pos);
	Ogre::Ray* getRay();
	
protected:
	Ogre::TerrainGroup* m_TerrainGroup;
	Ogre::Ray* m_ray;
};

#endif