#ifndef NEWS_UPLOAD_STATE_HPP
#define NEWS_UPLOAD_STATE_HPP

#include "VerticalScrollbar.hpp"

#include "../SFUI-Whorehouse/AppState.hpp"
#include "MultilineInputBox.hpp"

#include <SFUI/Layouts/Menu.hpp>
#include <SFUI/InputBox.hpp>

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// TODO: include scrollbar
// TODO: also add scrollbar to applistate

class NewsUploadState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

private:
	AppEngine* app;

	bool uploadComplete = false;

	void uploadNewsFile();

	std::vector<std::string> messages;
};

#endif // !NEWS_UPLOAD_STATE_HPP
