#ifndef _StateHandler_h_
#define _StateHandler_h_

#define DefaultStackSize 5
#define Increment 3

class GameElement;

enum STATE{
	STATE_NULL,
	STATE_WALK,
	STATE_EVADE, 
	STATE_ATTACK,
	STATE_TRACE,
	STATE_IDLE,
	STATE_DEAD,
	STATE_DYING,
	STATE_FLYING,
	STATE_SWAP,
	STATE_ATTACHING,
	STATE_TURNBACK,
};

enum EVENT{
	EVENT_NULL, //Main stack event
	EVENT_POP,
	EVENT_PUSH,
	EVENT_CHANGE, //Replace stack top element
	EVENT_PARALLEL_PUSH, //Parallel stack event
	EVENT_PARALLEL_POP,
	EVENT_PARALLEL_CHANGE,
};

class StateHandler{
private:
	STATE* m_mainStack;
	STATE* m_parallelStack;
	int m_nIndex;
	int m_nIndexParallel;
	int m_nStackSize;
	int m_nStackSizeParallel;
	GameElement* m_owner; //owner of this handler

public:
	StateHandler(GameElement* elt);
	virtual ~StateHandler();
	STATE getTopMainState();
	STATE getTopParallelState();
	void sendEvent(EVENT evnt,STATE state);
	void clearAllStack();
};

#endif