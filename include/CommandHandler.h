#ifndef __COMMANDHANDLER__
#define __COMMANDHANDLER__
#include <map>
#include <tuple>
#include <regex>
#include <functional>
#include <string_view>
#include <stdexcept>

class CommandHandler
{
public:
	using Callback = std::function<void(const std::cmatch&)>;
	using CallbackSlot = std::tuple<std::regex, Callback>;

	void addHandler(std::string_view command, std::string_view argumentRegex, Callback callback);
	void callHandler(std::string_view command, std::string_view arguments);

private:
	std::unordered_map<std::string, CallbackSlot> mHandlers;
};

using CommandHandlerException = std::runtime_error;

#endif