#pragma once
#include <memory>
#include <vector>
#include "AConsole.h"
#include <unordered_map>
#include <Windows.h>
#include "BaseScreen.h"
#include <string>


typedef std::string String;
const String MAIN_CONSOLE = "MAIN_CONSOLE";
const String MARQUEE_CONSOLE = "MARQUEE_CONSOLE";
const String SCHEDULING_CONSOLE = "SCHEDULING_CONSOLE";
const String MEMORY_CONSOLE = "MEMORY_CONSOLE";

class ConsoleManager
{
public:
	typedef std::unordered_map<String, std::shared_ptr<AConsole>> ConsoleTable;

	static ConsoleManager* getInstance();
	static void initialize();
	static void destroy();

	void drawConsole() const;
	void process() const;
	void switchConsole(String consoleName);

	void registerScreen(std::shared_ptr<BaseScreen> screenRef);
	void switchToScreen(String screenName);
	void unregisterScreen(String screenName);

	void returnToPreviousConsole();
	void exitApplication();
	bool isRunning() const;

	HANDLE getConsoleHandle() const;

	void setCursorPointer(int posX, int posY) const;

private:
	ConsoleManager();
	~ConsoleManager() = default;
	ConsoleManager(ConsoleManager const&) {};
	ConsoleManager& operator=(ConsoleManager const&) {};
	static ConsoleManager* sharedInstance;

	ConsoleTable consoletable;
	std::shared_ptr<AConsole> currentConsole;
	std::shared_ptr<AConsole> previousConsole;

	HANDLE consoleHandle;
	bool running = true;
};