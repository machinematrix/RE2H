#include "CommandHandler.h"

void CommandHandler::addHandler(std::string_view command, std::string_view argumentRegex, Callback callback)
{
	try {
		mHandlers[command.data()] = CallbackSlot(argumentRegex.data(), callback);
	}
	catch (const std::regex_error &e) {
		std::string msg("Could not add command handler: ");

		msg += e.what();
		throw CommandHandlerException(msg);
	}
}

void CommandHandler::callHandler(std::string_view command, std::string_view arguments)
{
	try
	{
		auto &slot = mHandlers.at(command.data());
		std::cmatch match;

		if (std::regex_match(arguments.data(), match, std::get<0>(slot)) && std::get<1>(slot)) {
			std::get<1>(slot)(match);
		}
		else
			throw CommandHandlerException("Bad arguments");
	}
	catch (const std::out_of_range&) {
		throw CommandHandlerException("Unknown command");
	}
}