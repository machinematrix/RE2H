#include "CommandHandler.h"

void CommandHandler::addHandler(std::string_view command, Callback callback)
{
	try {
		mHandlers[command.data()] = callback;
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
		if (auto &slot = mHandlers.at(command.data()))
			slot(arguments);
		else
			throw CommandHandlerException("Invalid handler");
	}
	catch (const std::out_of_range&) {
		throw CommandHandlerException("Unknown command");
	}
}