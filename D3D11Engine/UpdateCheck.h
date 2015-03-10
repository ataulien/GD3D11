#pragma once
class UpdateCheck
{
public:
	UpdateCheck(void);
	~UpdateCheck(void);

	/** Checks for update, returns new version URL if found */
	static std::string CheckForUpdate();

	/** Runs the updater and waits for it to finish */
	static void RunUpdater();
};

