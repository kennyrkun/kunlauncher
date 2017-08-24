#ifndef APP_STATE_HPP
#define APP_STATE_HPP

#include "AppEngine.hpp"

class AppState3
{
public:
	virtual void Init(AppEngine3* app_) = 0;
	virtual void Cleanup() = 0;

	virtual void Pause() = 0;
	virtual void Resume() = 0;

	virtual void HandleEvents() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	void ChangeState(AppState3* state)
	{
		app->ChangeState(state);
	}

protected:
	AppState3() { }

private:
	AppEngine3* app;
};

#endif // !APP_STATE_HPP
