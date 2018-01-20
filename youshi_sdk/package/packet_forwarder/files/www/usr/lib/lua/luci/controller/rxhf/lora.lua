module("luci.controller.rxhf.lora", package.seeall)

function index()
	local uci = require("luci.model.uci").cursor()

	entry({"rxhf", "lora"}, alias("rxhf", "lora", "sdk"), _("Lora"), 50).index = true
	if nixio.fs.access("/etc/config/rxhf") then
		local use_lora_sdk = uci:get("rxhf", "use", "use_lora_sdk")
		for i, v in ipairs(uci:get("rxhf", "use", "lora_sdk")) do
			local name = uci:get("rxhf", v, "name")
			local menu = uci:get("rxhf", v, "menu")
			local index = uci:get("rxhf", v, "index")
			if name and menu and index then
				if use_lora_sdk == name then
					menu = menu .. " (enable)"
				end
				entry({"rxhf", "lora", name}, call("lora_sdk"), _(menu), tonumber(index))
			end
		end

		entry({"rxhf", "lora", "enable_lora"}, call("enable_lora"), nil).leaf = true
		entry({"rxhf", "lora", "load_config"}, call("load_config"), nil).leaf = true
		entry({"rxhf", "lora", "modify_config"}, call("modify_config"), nil).leaf = true
	end
end

function lora_sdk()
	local lora_sdk_ver = luci.http.formvalue("lora_sdk_ver")
	luci.template.render("rxhf_lora/sdk", {lora_sdk_ver=lora_sdk_ver})
end

function enable_lora()
	local x = luci.model.uci.cursor()
	local sys = require "luci.sys"

	local use_lora_sdk = luci.http.formvalue("use_lora_sdk")
	local use_lora_sdk_ver = luci.http.formvalue("use_lora_sdk_ver")
	local exec_start = x:get("rxhf", use_lora_sdk_ver, "exec_start")
	local exec_stop = x:get("rxhf", use_lora_sdk_ver, "exec_stop")

	if exec_start and exec_stop then
		sys.call("/etc/init.d/pfw stop > /dev/null")
		local use = x:get("rxhf", "use")
		if not use then
			x:set("rxhf", "use", "lora")
		end
		x:set("rxhf", "use", "use_lora_sdk", use_lora_sdk)
		x:set("rxhf", use_lora_sdk, "use_lora_sdk_ver", use_lora_sdk_ver)
		x:commit("rxhf")
		sys.call("/etc/init.d/pfw start > /dev/null")
		
		luci.http.prepare_content("text/plain")
		luci.http.write("finish")
	else
		luci.http.prepare_content("text/plain")
		luci.http.write("failure")
	end
end

function load_config()
	local fs  = require "nixio.fs"
	local pg_base_path = luci.http.formvalue("pg_base_path")
	local model = luci.http.formvalue("model")
	local configs = { }

	local list
	for list in fs.dir(pg_base_path .. model .. "/") do
		local data = {
			config = list
		}
		configs[#configs+1] = data
	end

    luci.http.prepare_content("application/json")
    luci.http.write_json(configs)
end

function split_last(str, c)
	local re_str = string.reverse(str)
	return string.reverse(string.sub(re_str, 1, string.find(re_str, c) - 1))
end

function modify_config()
	local json = require "luci.json"
	local x = luci.model.uci.cursor()
	local sys = require "luci.sys"

	local name = luci.http.formvalue("name")
	local use_lora_sdk_ver = luci.http.formvalue("use_lora_sdk_ver")
	local gateway_id = luci.http.formvalue("gateway_id")
	local server_addr = luci.http.formvalue("server_addr")
	local port_up = luci.http.formvalue("port_up")
	local port_down = luci.http.formvalue("port_down")
	local model = luci.http.formvalue("model")
	local use_config = luci.http.formvalue("use_config")

	if not name or not use_lora_sdk_ver then
		luci.http.prepare_content("text/plain")
		luci.http.write("failure")
		return
	end

	local use_lora_sdk = x:get("rxhf", "use", "use_lora_sdk")
	if use_lora_sdk and use_lora_sdk == name then
		sys.call("/etc/init.d/pfw stop > /dev/null")
	end

	local pg_base_path = x:get("rxhf", use_lora_sdk_ver, "pg_base_path")
	local file, data, file_name
	for i, v in ipairs(x:get("rxhf", use_lora_sdk_ver, "sdk_config")) do
		if split_last(v, "/") == "global_conf.json" and pg_base_path and string.len(model) ~= 0 and string.len(use_config) ~= 0 then
			sys.call("ln -sf " .. pg_base_path .. model .. "/" .. use_config .. " " .. v)
		end
		if split_last(v, "/") == "local_conf.json" then
			file = io.open(v, "r")
			file_name = v
		end
	end
	
	if file then
		local json_data = file:read("*a")
		data = json.decode(json_data)
		file:close()

		if string.len(gateway_id) ~= 0 then
			data['gateway_conf']['gateway_ID'] = gateway_id
		end
		if string.len(server_addr) ~= 0 then
			data['gateway_conf']['server_address'] = server_addr
		end
		if string.len(port_up) ~= 0 then
			data['gateway_conf']['serv_port_up'] = tonumber(port_up)
		end
		if string.len(port_down) ~= 0 then
			data['gateway_conf']['serv_port_down'] = tonumber(port_down)
		end

		file = io.open(file_name, "w")
		if file then
			file:write(json.encode(data))
			file:close()
		end
	end

	x:set("rxhf", name, "use_lora_sdk_ver", use_lora_sdk_ver)
	if model then
		x:set("rxhf", use_lora_sdk_ver, "model", model)
	end
	if use_config then
		x:set("rxhf", use_lora_sdk_ver, "use_config", use_config)
	end
	x:commit("rxhf")
	if use_lora_sdk and use_lora_sdk == name then
		sys.call("/etc/init.d/pfw start > /dev/null")
	end

	luci.http.prepare_content("text/plain")
	luci.http.write("finish")		
end