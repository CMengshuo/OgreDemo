#pragma once 

#ifndef NULL
#define NULL 0
#endif

template<class T>
class MySingleton
{
protected:
	MySingleton(){}
	virtual ~MySingleton()
	{
		__cleanUp();
	}

public:
	static T* GetSingletonPtr()
	{
		if (__pInstance == 0)
		{
			__pInstance = new T();
		}
		return __pInstance;
	}
private:
	MySingleton(const MySingleton &); //prevent copy
	MySingleton & operator = (const MySingleton &); //prevent copy
	void __cleanUp();
	static T* __pInstance;
};

template <class T> 
T* MySingleton<T>::__pInstance = 0;

template <class T>
void MySingleton<T>::__cleanUp()
{
	if (__pInstance != 0)
	{
		delete __pInstance;
		__pInstance = 0;
	}
}