#ifndef PROGRAM_ENGINE_HPP
#define PROGRAM_ENGINE_HPP

#include <SFML\Graphics.hpp>
#include <vector>
#include <string>

//class AppState;
class AppState2;
/*class AppEngine
{
public:
	void Init(std::string title, int width = 525, int height = 325);
	void Cleanup();

	void ChangeState(AppState* state);
	void PushState(AppState* state);
	void PopState();

	void HandleEvents();
	void Update();
	void Draw();

	bool Running() { return running; }
	void Quit() { running = false; }

	sf::RenderWindow* window;

private:
	std::vector<AppState*> states;

	bool running;
	bool fullscreen;
};*/

class AppEngine2
{
public:
	AppEngine2(std::string title, int width = 525, int height = 325);
	~AppEngine2();

	void ChangeState(AppState2* state);
	void PushState(AppState2* state);
	void PopState();

	void HandleEvents();
	void Update();
	void Draw();

	bool Running() { return running; }
	void Quit() { running = false; }

	sf::RenderWindow* window;

	std::vector<AppState2*> states;

private:

	bool running;
	bool fullscreen;
};

#endif // !PROGRAM_ENGINE_HPP
