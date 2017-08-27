#ifndef APP_STATE_HPP
#define APP_STATE_HPP

#include "AppEngine.hpp"

class AppState
{
public:
	virtual void Init(AppEngine* app_) = 0;
	virtual void Cleanup() = 0;

	virtual void Pause() = 0;
	virtual void Resume() = 0;

	virtual void HandleEvents() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	void ChangeState(AppState* state)
	{
		app->ChangeState(state);
	}

protected:
	AppState() { }

private:
	AppEngine* app;
};

#endif // !APP_STATE_HPP
