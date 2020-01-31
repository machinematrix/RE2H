#ifndef __COMMANDHANDLER__
#define __COMMANDHANDLER__
#include <map>
#include <tuple>
#include <functional>
#include <string_view>
#include <string>
#include <stdexcept>

class CommandHandler
{
public:
	using Callback = std::function<void(std::string_view)>;
	using CallbackSlot = std::tuple<Callback>;

	void addHandler(std::string_view command, Callback callback);
	void callHandler(std::string_view command, std::string_view arguments);

private:
	std::unordered_map<std::string, Callback> mHandlers;
};

using CommandHandlerException = std::runtime_error;

#endif