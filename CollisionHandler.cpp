#include "CollisionHandler.h"
#include "GameElement.h"
#include <vector>
#include "GameManager.h"
#define DEFAULT_RANGE 100
#define DEFAULT_DEGREE 360

CollisionHandler::CollisionHandler()
{
	m_vPositionOffest = Ogre::Vector3::ZERO;
}

CollisionHandler::~CollisionHandler(){}

void CollisionHandler::setPositionOffset(const Ogre::Vector3& offset){
	m_vPositionOffest = offset;
}

void CollisionHandler::setQueryMask(Ogre::uint32 mask)
{

}

bool CollisionHandler::checkCollision(Ogre::String& targetName, const std::vector<ELEMENT_TYPE> &typeVec){return false;}
bool CollisionHandler::checkCollision(GameElement* target){return false;}

//----------------------------Sphere----------------------------------------------------

SphereCollisionHandler::SphereCollisionHandler(GameElement* elt,Ogre::uint32 mask,Ogre::uint32 typeMask){
	m_owner = elt;
	m_sceneMgr = GameManager::GetSingletonPtr()->getSceneManager();
	m_sphere = new Ogre::Sphere(m_owner->getPosition(),DEFAULT_RANGE);
	m_sceneQuery = m_sceneMgr->createSphereQuery(*m_sphere);
	m_sceneQuery->setQueryMask(mask);
	m_sceneQuery->setQueryTypeMask(typeMask);
	m_rAwareDegree = DEFAULT_DEGREE;
}

SphereCollisionHandler::~SphereCollisionHandler(){
	delete m_sphere;
	m_sceneMgr->destroyQuery(m_sceneQuery);
}


void SphereCollisionHandler::setSphere(const Ogre::Sphere& sp){
	m_sphere->setCenter(sp.getCenter());
	m_sphere->setRadius(sp.getRadius());
}


void SphereCollisionHandler::setCenter(const Ogre::Vector3& center){
	m_sphere->setCenter(center);
}

void SphereCollisionHandler::setRadius(Ogre::Real raduis){
	m_sphere->setRadius(raduis);
}

void SphereCollisionHandler::setAwareDegree(Ogre::Real degree){
	m_rAwareDegree = degree;
}

Ogre::Sphere* SphereCollisionHandler::getSphere(){
	return m_sphere;
}

bool SphereCollisionHandler::_isBlindSpot(const Ogre::Vector3& toTargetVec){
	Ogre::Vector3 currentDirection = m_owner->getDirection();
	Ogre::Real degree = currentDirection.angleBetween(toTargetVec.normalisedCopy()).valueDegrees();
	if (degree <= m_rAwareDegree/2)
		return false;

	return true;
}

bool SphereCollisionHandler::checkCollision(GameElement* target){
	m_sphere->setCenter(m_owner->getPosition()+m_vPositionOffest);
	m_sceneQuery->setSphere(*m_sphere);
	Ogre::MovableObject* object;

	Ogre::SceneQueryResult res = m_sceneQuery->execute();
	res.movables.sort();
	if (!res.movables.empty())
	{	
		Ogre::SceneQueryResultMovableList::iterator iter = res.movables.begin();
		while (iter != res.movables.end())
		{
			object = static_cast<Ogre::MovableObject*>(*iter);
			if (object->getName() == target->getName())
			{
				if (!_isBlindSpot(target->getNode()->_getDerivedPosition() - m_owner->getPosition()))
					return true;
			}
			iter++;
		}
	}
	return false;
}

void SphereCollisionHandler::setQueryMask(Ogre::uint32 mask){
	m_sceneQuery->setQueryMask(mask);
}

bool SphereCollisionHandler::checkCollision(Ogre::String& targetName, const std::vector<ELEMENT_TYPE> &typeVec){
	m_sphere->setCenter(m_owner->getPosition()+m_vPositionOffest);
	m_sceneQuery->setSphere(*m_sphere);
	Ogre::MovableObject* object;
	Ogre::StringVector nameVector;
	Ogre::String strName;
	targetName = "";

	Ogre::SceneQueryResult res = m_sceneQuery->execute();
	res.movables.sort();
	if (!res.movables.empty())
	{	
		Ogre::SceneQueryResultMovableList::iterator iter = res.movables.begin();
		while (iter != res.movables.end())
		{
			object = static_cast<Ogre::MovableObject*>(*iter);
			strName = object->getName();
			//sometimes the object collide with itself, gonna fix this later
			if (strName != m_owner->getName())
			{
				nameVector = Ogre::StringUtil::split(strName,"_");
				if (nameVector.size() >= 2)
				{
					ELEMENT_TYPE type = static_cast<ELEMENT_TYPE>(Ogre::StringConverter::parseLong(nameVector[0]));
					if (isValidTarget(type,typeVec))
					{
						if (!_isBlindSpot(object->getParentSceneNode()->_getDerivedPosition() - m_owner->getPosition()))
							targetName += strName + "@";
					}
				}	
			}
			iter++;
		}
	}

	if (targetName != "")
		return true;

	return false;
}

//----------------------------Box-------------------------------------------------------
BoxCollisionHandler::BoxCollisionHandler(GameElement* elt,Ogre::uint32 mask,Ogre::uint32 typeMask, bool useVirtualBox){
	m_owner = elt;
	m_sceneMgr = GameManager::GetSingletonPtr()->getSceneManager();
	m_box = m_owner->getNode()->_getWorldAABB();
	m_sceneQuery = m_sceneMgr->createAABBQuery(m_box);
	m_sceneQuery->setQueryMask(mask);
	m_sceneQuery->setQueryTypeMask(typeMask);
	m_vScale = Ogre::Vector3(1.0,1.0,1.0);
	m_bUseVirtualBox = useVirtualBox;
}

BoxCollisionHandler::~BoxCollisionHandler(){
	m_sceneMgr->destroyQuery(m_sceneQuery);
}

void BoxCollisionHandler::setScale(const Ogre::Vector3& scale){
	m_vScale = scale;
}

void BoxCollisionHandler::setQueryMask(Ogre::uint32 mask){
	m_sceneQuery->setQueryMask(mask);
}

Ogre::AxisAlignedBox& BoxCollisionHandler::getBox(){
	return m_box;
}

void BoxCollisionHandler::setBox(const Ogre::AxisAlignedBox& box)
{
	m_box = box;
}

bool BoxCollisionHandler::checkCollision(GameElement* target){
	if (!m_bUseVirtualBox)
	{
		m_box = m_owner->getNode()->_getWorldAABB();
	}

	if (m_vPositionOffest != Ogre::Vector3::ZERO)
	{
		Ogre::Vector3 max = m_box.getMaximum();
		Ogre::Vector3 min = m_box.getMinimum();
		m_box.setExtents(min,max);
	}

	m_box.scale(m_vScale);
	m_sceneQuery->setBox(m_box);

	Ogre::MovableObject* object;

	Ogre::SceneQueryResult res = m_sceneQuery->execute();
	res.movables.sort();
	if (!res.movables.empty())
	{	
		Ogre::SceneQueryResultMovableList::iterator iter = res.movables.begin();
		while (iter != res.movables.end())
		{
			object = static_cast<Ogre::MovableObject*>(*iter);
			if (object->getName() == target->getName())
			{
				return true;
			}
			iter++;
		}
	}
	return false;
}

bool BoxCollisionHandler::checkCollision(Ogre::String& targetName, const std::vector<ELEMENT_TYPE> &typeVec){
	if (!m_bUseVirtualBox)
	{
		m_box = m_owner->getNode()->_getWorldAABB();
	}

	if (m_vPositionOffest != Ogre::Vector3::ZERO)
	{
		Ogre::Vector3 max = m_box.getMaximum();
		Ogre::Vector3 min = m_box.getMinimum();
		m_box.setExtents(min,max);
	}

	m_box.scale(m_vScale);
	m_sceneQuery->setBox(m_box);

	Ogre::MovableObject* object;

	Ogre::StringVector nameVector;
	Ogre::String strName;
	targetName = "";

	Ogre::SceneQueryResult res = m_sceneQuery->execute();
	res.movables.sort();
	if (!res.movables.empty())
	{	
		Ogre::SceneQueryResultMovableList::iterator iter = res.movables.begin();
		while (iter != res.movables.end())
		{
			object = static_cast<Ogre::MovableObject*>(*iter);
			strName = object->getName();
			//sometimes the object collide with itself, gonna fix this later
			if (strName != m_owner->getName())
			{
				nameVector = Ogre::StringUtil::split(strName,"_");

				if (nameVector.size() >= 2)
				{
					ELEMENT_TYPE type = static_cast<ELEMENT_TYPE>(Ogre::StringConverter::parseLong(nameVector[0]));
					if (isValidTarget(type,typeVec))
					{
						targetName += strName + "@";
					}
				}
			}
			iter++;
		}
	}

	if (targetName != "")
		return true;

	return false;
}

//----------------------------------------ray-------------------------------------

RayCollisionHandler::RayCollisionHandler(GameElement* elt,Ogre::uint32 mask,Ogre::uint32 typeMask,Ogre::Real distance){
	m_owner = elt;
	m_sceneMgr = GameManager::GetSingletonPtr()->getSceneManager();
	m_vDirection = Ogre::Vector3::ZERO;
	m_ray = new Ogre::Ray(m_owner->getPosition(),m_owner->getDirection());
	m_sceneQuery = m_sceneMgr->createRayQuery(*m_ray);
	m_sceneQuery->setQueryMask(mask);
	m_sceneQuery->setQueryTypeMask(typeMask);
	m_rValidDistance = distance;
}

RayCollisionHandler::~RayCollisionHandler(){
	delete m_ray;
	m_sceneMgr->destroyQuery(m_sceneQuery);
}

void RayCollisionHandler::setQueryMask(Ogre::uint32 mask){
	m_sceneQuery->setQueryMask(mask);
}

void RayCollisionHandler::setDirection(const Ogre::Vector3& direction){
	m_vDirection = direction;
}

Ogre::Ray* RayCollisionHandler::getRay(){
	return m_ray;
}

bool RayCollisionHandler::checkCollision(GameElement* target){
	m_ray->setDirection(m_vDirection);
	m_ray->setOrigin(m_owner->getPosition()+m_vPositionOffest);
	m_sceneQuery->setRay(*m_ray);

	Ogre::MovableObject* object;

	Ogre::RaySceneQueryResult res = m_sceneQuery->execute();
	if (!res.empty())
	{	
		Ogre::RaySceneQueryResult::iterator iter = res.begin();
		while (iter != res.end())
		{
			object = static_cast<Ogre::MovableObject*>(iter->movable);
			
			if (m_rValidDistance != -1)
			{
				if (iter->distance > m_rValidDistance)
				{
					iter++;
					continue;
				}
			}

			if (object->getName() == target->getName())
			{
				return true;
			}
			iter++;
		}
	}
	return false;
}

bool RayCollisionHandler::checkCollision(Ogre::String& targetName, const std::vector<ELEMENT_TYPE> &typeVec){
	m_ray->setDirection(m_vDirection);
	m_ray->setOrigin(m_owner->getPosition()+m_vPositionOffest);
	m_sceneQuery->setRay(*m_ray);

	Ogre::StringVector nameVector;
	Ogre::String strName;

	Ogre::RaySceneQueryResult res = m_sceneQuery->execute();
	if (!res.empty())
	{	
		Ogre::RaySceneQueryResult::iterator iter = res.begin();
		while (iter != res.end())
		{
			strName = iter->movable->getName();
		
			if (m_rValidDistance != -1)
			{
				if (iter->distance > m_rValidDistance)
				{
					iter++;
					continue;
				}
			}

			//sometimes the object collide with itself, gonna fix this later
			if (strName != m_owner->getName())
			{
				nameVector = Ogre::StringUtil::split(strName,"_");
				if (nameVector.size() >= 2)
				{
					ELEMENT_TYPE type = static_cast<ELEMENT_TYPE>(Ogre::StringConverter::parseLong(nameVector[0]));
					if (isValidTarget(type,typeVec))
					{
						targetName += strName + "@";
					}
				}
			}
			iter++;
		}
	}

	if (targetName != "")
		return true;

	return false;
}

void RayCollisionHandler::setDetectDistance(Ogre::Real distance)
{
	m_rValidDistance = distance;
}

//-----------------------------------------TerrainCollisionHandler---------------------------------------
TerrainCollisionHandler::TerrainCollisionHandler(GameElement* elt, Ogre::TerrainGroup* tGroup){
	m_owner = elt;
	m_TerrainGroup = tGroup;
	m_ray = new Ogre::Ray(m_owner->getPosition(),Ogre::Vector3::NEGATIVE_UNIT_Y);
}

TerrainCollisionHandler::~TerrainCollisionHandler(){
	delete m_ray;
}

Ogre::Ray* TerrainCollisionHandler::getRay()
{
	return m_ray;
}

bool TerrainCollisionHandler::checkTerrainCollision(Ogre::Vector3& pos){
	Ogre::Vector3 OwnerPos = m_owner->getPosition();
	OwnerPos.y = TERRAIN_CHECK_HEIGHT;
	m_ray->setOrigin(OwnerPos + m_vPositionOffest);

	Ogre::TerrainGroup::RayResult res = m_TerrainGroup->rayIntersects(*m_ray);
	if (res.hit)
	{
		pos = res.position;
		return true;
	}
	return false;
}

bool CollisionHandler::isValidTarget(const ELEMENT_TYPE &type, const std::vector<ELEMENT_TYPE> &typeVec){
	std::vector<ELEMENT_TYPE>::size_type vecSize = typeVec.size();

	if (vecSize == 0)
		return true;

	for (std::vector<ELEMENT_TYPE>::size_type i = 0;i<vecSize;i++)
	{
		if (typeVec[i] == type)
			return true;
	}
	return false;
}