#ifndef PROGRAM_ENGINE_HPP
#define PROGRAM_ENGINE_HPP

#include <SFML\Graphics.hpp>
#include <vector>
#include <string>

class AppState;
class AppEngine
{
public:

	void Init(std::string title, int width = 800, int height = 500, bool fullscreen = false);
	void Cleanup();

	void ChangeState(AppState* state);
	void PushState(AppState* state);
	void PopState();

	void HandleEvents();
	void Update();
	void Draw();

	bool Running() { return m_running; }
	void Quit() { m_running = false; }

	sf::RenderWindow* window;

private:
	// the stack of states
	std::vector<AppState*> states;

	bool m_running;
	bool m_fullscreen;
};

#endif // !PROGRAM_ENGINE_HPP
