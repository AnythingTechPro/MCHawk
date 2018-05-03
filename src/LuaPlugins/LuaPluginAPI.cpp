﻿#include "LuaPluginAPI.hpp"

#include <boost/algorithm/string.hpp>

extern lua_State* L;

void LuaServer::Init(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
	.beginClass<Client>("Client")
	.addProperty("name", &Client::GetName)
	.addFunction("GetWorld", &Client::GetWorld)
	.endClass();

	luabridge::getGlobalNamespace(L)
	.beginClass<World>("World")
	.addFunction("Save", &World::Save)
	.addFunction("GetOption", &World::GetOption)
	.endClass();

	luabridge::getGlobalNamespace(L)
	.beginClass<LuaServer>("Server")
	.addStaticFunction("SendMessage", &LuaServer::LuaSendMessage)
	.addStaticFunction("BroadcastMessage", &LuaServer::LuaBroadcastMessage)
	.addStaticFunction("SystemWideMessage", &LuaServer::LuaSystemWideMessage)
	.addStaticFunction("SendBlock", &LuaServer::LuaSendBlock)
	.addStaticFunction("KickClient", &LuaServer::LuaKickClient)
	.addStaticFunction("GetClientByName", &LuaServer::LuaGetClientByName)
	.addStaticFunction("LoadPlugin", &LuaServer::LuaLoadPlugin)
	.addStaticFunction("RegisterEvent", &LuaServer::LuaRegisterEvent)
	.addStaticFunction("AddCommand", &LuaServer::LuaAddCommand)
	.addStaticFunction("PlaceBlock", &LuaServer::LuaPlaceBlock)
	.addStaticFunction("MapGetBlockType", &LuaServer::LuaMapGetBlockType)
	.addStaticFunction("SendKick", &LuaServer::LuaSendKick)
	.addStaticFunction("GetClients", &LuaServer::LuaGetClients)
	.addStaticFunction("GetWorlds", &LuaServer::LuaGetWorlds)
	.addStaticFunction("Shutdown", &LuaServer::LuaServerShutdown)
	.endClass();
}

void LuaServer::LuaSendMessage(Client* client, std::string message)
{
	Server::GetInstance()->SendWrappedMessage(client, message);
}

void LuaServer::LuaSystemWideMessage(std::string message)
{
	Server::GetInstance()->SendSystemWideMessage(message);
}

void LuaServer::LuaBroadcastMessage(std::string message)
{
	Server::GetInstance()->BroadcastMessage(message);
}

void LuaServer::LuaKickClient(Client* client, std::string reason)
{
	Server::GetInstance()->KickClient(client, reason);
}

void LuaServer::LuaSendBlock(Client* client, short x, short y, short z, int type)
{
	ClassicProtocol::SendBlock(client, Position(x, y, z), type);
}

Client* LuaServer::LuaGetClientByName(std::string name, bool exact)
{
	return Server::GetInstance()->GetClientByName(name, exact);
}

void LuaServer::LuaLoadPlugin(std::string filename)
{
	Server::GetInstance()->GetPluginHandler().QueuePlugin(filename);
}

void LuaServer::LuaRegisterEvent(int type, luabridge::LuaRef func)
{
	Server::GetInstance()->GetPluginHandler().RegisterEvent(type, func);
}

void LuaServer::LuaAddCommand(std::string name, std::string aliases, luabridge::LuaRef func, std::string docString,
unsigned argumentAmount, unsigned permissionLevel)
{
	if (!func.isFunction()) {
		std::cerr << "Failed adding Lua command " << name << ": function does not exist" << std::endl;
		return;
	}

	LuaCommand* command = new LuaCommand(name, func, docString, argumentAmount, permissionLevel);

	Server::GetInstance()->GetCommandHandler().Register(name, command, aliases);
}

void LuaServer::LuaPlaceBlock(Client* client, int type, short x, short y, short z)
{
	World* world = client->GetWorld();

	bool valid = world->GetMap().SetBlock(x, y, z, type);

	if (!valid) {
		int type = LuaServer::LuaMapGetBlockType(client, x, y, z);
		ClassicProtocol::SendBlock(client, Position(x, y, z), type);
		return;
	}

	world->SendBlockToClients(type, x, y, z);
}

int LuaServer::LuaMapGetBlockType(Client* client, short x, short y, short z)
{
	Map& map = client->GetWorld()->GetMap();
	return map.GetBlockType(x, y, z);
}

void LuaServer::LuaSendKick(Client* client, std::string reason)
{
	ClassicProtocol::SendKick(client, reason);
}

luabridge::LuaRef LuaServer::LuaGetClients()
{
	std::vector<Client*> clients = Server::GetInstance()->GetClients();

	auto table = make_luatable();
	int i = 1;
	for (auto& obj : clients) {
		table[i] = obj;
		++i;
	}

	return table;
}

luabridge::LuaRef LuaServer::LuaGetWorlds()
{
	std::map<std::string, World*> worlds = Server::GetInstance()->GetWorlds();

	auto table = make_luatable();
	int i = 1;
	for (auto& obj : worlds) {
		table[i] = obj.second;
		++i;
	}

	return table;
}

void LuaServer::LuaServerShutdown()
{
	Server::GetInstance()->Shutdown();
}

// Struct to table stuff
luabridge::LuaRef make_luatable()
{
	luabridge::LuaRef table(L);
	table = luabridge::newTable(L);

	return table;
}

luabridge::LuaRef cauthp_to_luatable(const struct ClassicProtocol::cauthp clientAuth)
{
	luabridge::LuaRef table(L);
	table = luabridge::newTable(L);

	std::string name((char*)clientAuth.name, 0, sizeof(clientAuth.name));

	// Remove 0x20 padding
	boost::trim_right(name);

	table["name"] = name;

	return table;
}

luabridge::LuaRef cmsgp_to_luatable(const struct ClassicProtocol::cmsgp clientMsg)
{
	luabridge::LuaRef table(L);
	table = luabridge::newTable(L);

	std::string message((char*)clientMsg.msg, 0, sizeof(clientMsg.msg));

	// Remove 0x20 padding
	boost::trim_right(message);

	table["message"] = message;

	return table;
}

luabridge::LuaRef cblockp_to_luatable(const struct ClassicProtocol::cblockp clientBlock)
{
	luabridge::LuaRef table(L);
	table = luabridge::newTable(L);

	table["type"] = clientBlock.type;
	table["mode"] = clientBlock.mode;
	table["x"] = clientBlock.x;
	table["y"] = clientBlock.y;
	table["z"] = clientBlock.z;

	return table;
}
