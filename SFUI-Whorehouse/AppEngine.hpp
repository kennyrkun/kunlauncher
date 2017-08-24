#ifndef PROGRAM_ENGINE_HPP
#define PROGRAM_ENGINE_HPP

#include <SFML\Graphics.hpp>
#include <vector>
#include <string>

class AppState3;
class AppEngine3
{
public:

	void Init(std::string title, int width = 525, int height = 325, bool fullscreen = false);
	void Cleanup();

	void ChangeState(AppState3* state);
	void PushState(AppState3* state);
	void PopState();

	void HandleEvents();
	void Update();
	void Draw();

	bool Running() { return m_running; }
	void Quit() { m_running = false; }

	sf::RenderWindow* window;

private:
	// the stack of states
	std::vector<AppState3*> states;

	bool m_running;
	bool m_fullscreen;
};

#endif // !PROGRAM_ENGINE_HPP
