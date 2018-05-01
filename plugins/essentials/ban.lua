EssentialsPlugin.Ban_banList = {}

EssentialsPlugin.Ban_BanCommand = function(client, args)
	if (not PermissionsPlugin.CheckPermissionNotify(client, "admin")) then
		return
	end

	name = string.lower(args[1])

	player = Server.GetClientByName(name, true)
	if (player == nil) then
		Server.SendMessage(client, "&cPlayer " .. name .. " not found")
		return
	end

	Server.KickClient(player, "Banned")
	Server.BroadcastMessage("&e" .. client.name .. " has banned " .. name .. " from the server.")

	EssentialsPlugin.Ban_banList[name] = 1
	EssentialsPlugin.Ban_SaveBans()
end

EssentialsPlugin.Ban_UnbanCommand = function(client, args)
	if (not PermissionsPlugin.CheckPermissionNotify(client, "admin")) then
		return
	end

	if (EssentialsPlugin.Ban_banList[name] == nil) then
		Server.SendMessage(client, "&cPlayer &f" .. name .. " not banned")
		return
	end

	EssentialsPlugin.Ban_banList[name] = nil
	EssentialsPlugin.Ban_SaveBans()

	Server.SendMessage(client, "&eUnbanned player " .. name)
end

EssentialsPlugin.Ban_OnAuth = function(client, args)
	-- Use args.name because client.name isn't set yet
	name = string.lower(args.name)

	if (EssentialsPlugin.Ban_banList[name] ~= nil) then
		Server.KickClient(client, "Banned")
		Flags.NoDefaultCall = 1
	end
end

EssentialsPlugin.Ban_SaveBans = function()
	local f = io.open("bans.txt", "w")
	if f then
		for name,_ in pairs(EssentialsPlugin.Ban_banList) do
			f:write(name .. "\n")
		end

		f:close()
	end
end
EssentialsPlugin.Ban_LoadBans = function()
	local f = io.open("bans.txt", "r")
	if f then
		lines = {}
		for line in io.lines("bans.txt") do
			EssentialsPlugin.Ban_banList[line] = 1
		end

		f:close()
	else
		print("Couldn't open bans.txt")
	end
end

EssentialsPlugin.Ban_LoadBans()
