#include "CommandHandler.h"

void CommandHandler::addHandler(std::string_view command, Callback callback)
{
	mHandlers[command.data()] = callback;
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