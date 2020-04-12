CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_REQUIRED_STANDARD ON)
SET(CMAKE_DEBUG_POSTFIX "-d")

SET(PROJECT_NAME "kunlauncher")
PROJECT(${PROJECT_NAME} LANGUAGES CXX)

SET(HEADERS ./AllAppsListState.hpp ./AnimationManager.hpp ./App.hpp ./AppEngine.hpp ./AppState.hpp ./AppWindow.hpp ./ColorInputBox.hpp ./Download.hpp ./Globals.hpp ./HomeState.hpp ./InitialiseState.hpp Interpolate.hpp LauncherUpdater.hpp ./MessageBox.hpp ./MyApp.hpp ./MyAppListState.hpp ./Navbar.hpp ./News.hpp ./ProgressBar.hpp ./ResourceManager.hpp ./SettingsParser.hpp ./SettingsState.hpp ./StoreApp.hpp ./Theme.hpp ./ThreadManager.hpp ./VerticalScrollbar.hpp)
SET(SOURCES ./AllAppsListState.cpp ./AnimationManager.cpp ./App.cpp ./AppEngine.cpp ./AppWindow.cpp ./ColorInputBox.cpp ./Download.cpp ./Globals.cpp ./HomeState.cpp ./InitialiseState.cpp ./Interpolate.cpp LauncherUpdater.cpp main.cpp ./MessageBox.cpp ./MyApp.cpp ./MyAppListState.cpp ./Navbar.cpp ./News.cpp ./ProgressBar.cpp ./ResourceManager.cpp ./SettingsParser.cpp ./SettingsState.cpp ./StoreApp.cpp ./Theme.cpp ./ThreadManager.cpp ./VerticalScrollbar.cpp)
ADD_EXECUTABLE(${PROJECT_NAME} ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ../../SFML/include)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} sfml-graphics sfml-audio sfml-window sfml-network sfml-system -L"../../SFML/lib")

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ../../SFUI/include)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} sfui -L"../../SFUI/lib")

TARGET_LINK_LIBRARIES(${PROJECT_NAME} stdc++fs)

if (UNIX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} GL)
endif(UNIX)
