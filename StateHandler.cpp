#include <string.h>
#include "GameObject.h"

StateHandler::StateHandler(GameElement* elt)
{
	m_owner = elt;
	m_nIndex = 0;
	m_nIndexParallel = 0;
	m_nStackSize = DefaultStackSize;
	m_nStackSizeParallel = DefaultStackSize;

	m_mainStack = (STATE*) malloc(DefaultStackSize*sizeof(STATE));
	memset(m_mainStack,STATE_NULL,DefaultStackSize*sizeof(STATE));
	
	m_parallelStack = (STATE*) malloc(DefaultStackSize*sizeof(STATE));
	memset(m_parallelStack,STATE_NULL,DefaultStackSize*sizeof(STATE));
}

StateHandler::~StateHandler()
{
	free(m_mainStack);
	free(m_parallelStack);
}

void StateHandler::sendEvent(EVENT event, STATE state){
	switch (event)
	{
	case EVENT_NULL:
		break;
	case EVENT_POP:
		m_nIndex--;
		//Maintain one state at any time
		if (m_nIndex <= 0)
			break;
		m_mainStack[m_nIndex] = STATE_NULL;
		m_owner->processEvent(event,m_mainStack[m_nIndex-1]);
		break;
	case EVENT_CHANGE:
		m_mainStack[m_nIndex-1] = state;
		m_owner->processEvent(event,m_mainStack[m_nIndex-1]);
		break;
	case EVENT_PUSH:
		if (m_nIndex >= m_nStackSize)
		{	
			STATE* newAddress = (STATE*) realloc(m_mainStack,(m_nStackSize+Increment)*sizeof(STATE));

			if (!newAddress)
				return;

			m_mainStack = newAddress;

			memset(m_mainStack+m_nIndex,STATE_NULL,Increment*sizeof(STATE));
			m_nStackSize += Increment;
		}
		m_mainStack[m_nIndex] = state;
		m_owner->processEvent(event,m_mainStack[m_nIndex]);
		m_nIndex++;
		break;
	case EVENT_PARALLEL_POP:
		m_nIndexParallel--;
		//Maintain one state at any time
		if (m_nIndexParallel <= 0)
			break;
		m_parallelStack[m_nIndexParallel] = STATE_NULL;
		m_owner->processEvent(event,m_parallelStack[m_nIndexParallel-1]);
		break;
	case EVENT_PARALLEL_CHANGE:
		m_parallelStack[m_nIndexParallel-1] = state;
		m_owner->processEvent(event,m_parallelStack[m_nIndexParallel-1]);
		break;
	case EVENT_PARALLEL_PUSH:
		if (m_nIndexParallel >= m_nStackSizeParallel)
		{
			STATE* newAddress = (STATE*) realloc(m_parallelStack,(m_nStackSizeParallel+Increment)*sizeof(STATE));

			if (!newAddress)
				return;

			m_parallelStack = newAddress;

			memset(m_parallelStack+m_nIndexParallel,STATE_NULL,Increment*sizeof(STATE));
			m_nStackSizeParallel += Increment;
		}
		m_parallelStack[m_nIndexParallel] = state;
		m_owner->processEvent(event,m_parallelStack[m_nIndexParallel]);
		m_nIndexParallel++;
		break;
	default:
		break;
	}


//#if _DEBUG
	Ogre::String str;
	Ogre::String action;
	for (int i=0;i<m_nStackSize;i++)
	{
		switch(m_mainStack[i])
		{
		case STATE_WALK:
			action = "walk";
			break;
		case STATE_EVADE:
			action = "evade";
			break;
		case STATE_TRACE:
			action = "trace";
			break;
		case STATE_ATTACK:
			action = "attack";
			break;
		default:
			action = "";
			break;
		}

		str += action + "_";
	}

	Ogre::LogManager::getSingleton().logMessage(m_owner->getName() + "  current state: " + str);
//#endif
}

void StateHandler::clearAllStack()
{
	while (m_nIndex > 0)
	{
		m_mainStack[m_nIndex--] = STATE_NULL;
	}

	while (m_nIndexParallel > 0)
	{
		m_mainStack[m_nIndexParallel--] = STATE_NULL;
	}
}