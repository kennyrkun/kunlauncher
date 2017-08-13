#ifndef APP_STATE_HPP
#define APP_STATE_HPP

#include "AppEngine.hpp"

/*class AppState
{
public:
	virtual void Init(AppEngine* app_) = 0;
	virtual void Cleanup() = 0;

	virtual void Pause() = 0;
	virtual void Resume() = 0;

	virtual void HandleEvents() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	// this isn't necessary
//	void ChangeState(AppEngine* app, AppState* state)
//	{
//		app->ChangeState(state);
//	}

//protected:
//	AppState() { }
};*/

class AppState2
{
public:
//	virtual void Init(AppEngine* app_) = 0;
//	virtual void Cleanup() = 0;

//	AppState2();
//	~AppState2();

	virtual void Pause() = 0;
	virtual void Resume() = 0;

	virtual void HandleEvents() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
};

#endif // !APP_STATE_HPP
