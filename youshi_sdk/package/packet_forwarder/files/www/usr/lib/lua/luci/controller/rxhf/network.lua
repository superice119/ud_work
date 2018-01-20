module("luci.controller.rxhf.network", package.seeall)

function index()
	local uci = require("luci.model.uci").cursor()

	entry({"rxhf", "network"}, alias("rxhf", "network", "mode"), _("Network"), 30).index = true

	if nixio.fs.access("/etc/config/network") and nixio.fs.access("/etc/config/wireless") then
		entry({"rxhf", "network", "mode"}, template("rxhf_network/mode"), _("Mode"), 1)
		entry({"rxhf", "network", "get_site_survey"}, call("get_site_survey"), nil).leaf = true
		entry({"rxhf", "network", "switch_mode"}, call("switch_mode"), nil).leaf = true
		entry({"rxhf", "network", "confirm_fail"}, call("confirm_fail"), nil).leaf = true
	end

	if nixio.fs.access("/etc/config/network") then
		entry({"rxhf", "network", "ethernet"}, template("rxhf_network/ethernet"), _("Ethernet"), 2)
		entry({"rxhf", "network", "switch_proto"}, call("switch_proto"), nil).leaf = true
	end

	if nixio.fs.access("/etc/config/network") then
		entry({"rxhf", "network", "lan_config"}, template("rxhf_network/lan_config"), _("LAN config"), 3)
		entry({"rxhf", "network", "modify_lan_ip"}, call("modify_lan_ip"), nil).leaf = true
	end

	if nixio.fs.access("/etc/config/wireless") then
		local has_device
		uci:foreach("wireless", "wifi-device", 
			function(s)
				has_device = true
				return false
			end)

		if has_device then
			entry({"rxhf", "network", "region"}, template("rxhf_network/region"), _("Region"), 4)
			entry({"rxhf", "network", "switch_region"}, call("switch_region"), nil).leaf = true
		end
	end

	entry({"rxhf", "network", "diagnostics"}, template("rxhf_network/diagnostics"), _("Diagnostics"), 60)
	entry({"rxhf", "network", "diag_ping"}, call("diag_ping"), nil).leaf = true
	entry({"rxhf", "network", "diag_nslookup"}, call("diag_nslookup"), nil).leaf = true
	entry({"rxhf", "network", "diag_traceroute"}, call("diag_traceroute"), nil).leaf = true
	entry({"rxhf", "network", "diag_ping6"}, call("diag_ping6"), nil).leaf = true
	entry({"rxhf", "network", "diag_traceroute6"}, call("diag_traceroute6"), nil).leaf = true
end

function diag_command(cmd, addr)
	if addr and addr:match("^[a-zA-Z0-9%-%.:_]+$") then
		luci.http.prepare_content("text/plain")

		local util = io.popen(cmd % addr)
		if util then
			while true do
				local ln = util:read("*l")
				if not ln then break end
				luci.http.write(ln)
				luci.http.write("\n")
			end

			util:close()
		end

		return
	end

	luci.http.status(500, "Bad address")
end

function diag_ping(addr)
	diag_command("ping -c 5 -W 1 %q 2>&1", addr)
end

function diag_traceroute(addr)
	diag_command("traceroute -q 1 -w 1 -n %q 2>&1", addr)
end

function diag_nslookup(addr)
	diag_command("nslookup %q 2>&1", addr)
end

function diag_ping6(addr)
	diag_command("ping6 -c 5 %q 2>&1", addr)
end

function diag_traceroute6(addr)
	diag_command("traceroute6 -q 1 -w 2 -n %q 2>&1", addr)
end

function switch_proto()
	local x = luci.model.uci.cursor()
	local protocol = luci.http.formvalue("protocol")

	if protocol == "dhcp" then
		x:delete("network", "wan", "ipaddr");
		x:delete("network", "wan", "netmask");
		x:delete("network", "wan", "gateway");
		x:delete("network", "wan", "dns");
		x:set("network", "wan", "proto", protocol)
		x:set("network", "wan", "hostname", "rhf2s024")
	else
		local address = luci.http.formvalue("address")
		local netmask = luci.http.formvalue("netmask")
		local gateway = luci.http.formvalue("gateway")
		local dns = luci.http.formvalue("dns")

		x:delete("network", "wan", "hostname")
		x:set("network", "wan", "proto", protocol)
		x:set("network", "wan", "ipaddr", address)
		x:set("network", "wan", "netmask", netmask)
		x:set("network", "wan", "gateway", gateway)
		x:set("network", "wan", "dns", dns)
	end
	x:commit("network")
	luci.util.exec("/etc/init.d/network restart")
	luci.http.prepare_content("text/plain")
	luci.http.write("finish")
end

function switch_mode()
	local oldmode = luci.http.formvalue("oldmode")
	local mode = luci.http.formvalue("mode")

	if mode == "apsta" then
		local apssid = luci.http.formvalue("apssid")
		local apkey = luci.http.formvalue("apkey")
		local stassid = luci.http.formvalue("stassid")
		local stakey = luci.http.formvalue("stakey")

		if apkey == "" then
			apkey = "null"
		end
		if stakey == "" then
			stakey = "null"
		end
		os.execute("/usr/bin/net_mode " .. oldmode .. " " .. mode .. " " .. apssid .. " " .. apkey .. " " .. stassid .. " " .. stakey)
	elseif mode == "ap" then
		local apssid = luci.http.formvalue("apssid")
		local apkey = luci.http.formvalue("apkey")

		if apkey == "" then
			apkey = "null"
		end
		os.execute("/usr/bin/net_mode " .. oldmode .. " " .. mode .. " " .. apssid .. " " .. apkey)
	elseif mode == "pppoe" then
		local apssid = luci.http.formvalue("apssid")
		local apkey = luci.http.formvalue("apkey")
		local username = luci.http.formvalue("username")
		local password = luci.http.formvalue("password")

		if apkey == "" then
			apkey = "null"
		end
		os.execute("/usr/bin/net_mode " .. oldmode .. " " .. mode .. " " .. apssid .. " " .. apkey .. " " .. username .. " " .. password)
	end

	luci.http.prepare_content("text/plain")
	luci.http.write("finish")
end

function get_site_survey()
    local rv = { }
    local l

    luci.sys.exec("iwpriv ra0 set SiteSurvey=1")
    luci.sys.exec("sleep 1")

    for l in luci.util.execi("iwpriv ra0 get_site_survey") do
        if l:match("^[0-9]") then
            local _ch = l:match("^(%d+)")
            local _bssid = l:match("(%x%x:%x%x:%x%x:%x%x:%x%x:%x%x)")
            local _sec, _signal, _mode, _extch = l:match("%x%x:%x%x:%x%x:%x%x:%x%x:%x%x%s*([%u%d/]+)%s*(%d+)%s*([%l%d/]+)%s*(%u+)%s+")

            local tmp1, ssid_start = l:find("^(%d+)")
            local ssid_end, tmp2   = l:find("(%x%x:%x%x:%x%x:%x%x:%x%x:%x%x)")
            local _ssid = l:sub(ssid_start+1, ssid_end-1):match("^%s*(.-)%s*$")

            if _ssid == "" then
                _ssid = nil
            end

            if _ch and _ssid and _bssid and _sec and _signal and _mode and _extch then
                local data = {
                    ch         = _ch,
                    ssid       = _ssid,
                    security   = _sec,
                    signal      = _signal
                }
                rv[#rv+1] = data
            end
        end
    end

    if #rv > 0 then
        luci.http.prepare_content("application/json")
        luci.http.write_json(rv)
        return
    end

    luci.http.status(404, "No such device")
end

function modify_lan_ip()
	local x = luci.model.uci.cursor()
	local lan_ip = luci.http.formvalue("lan_ip")

	if lan_ip then
		x:set("network", "lan", "ipaddr", lan_ip)
		x:commit("network")
		luci.sys.reboot()

		luci.http.prepare_content("text/plain")
		luci.http.write("finish")
	else
		luci.http.prepare_content("text/plain")
		luci.http.write("failure")
	end
end

-- 0	1-11
-- 1	1-13
-- 2	10-11
-- 3	10-13
-- 4	14
-- 5	1-14
-- 6	3-9
-- 7	5-13

-- 1	1-13 CN CHINA
-- 0	1-11 TW TAIWAN
-- 5	1-14 JP JAPAN
-- 1	1-13 FR EUROPE
-- 0	1-11 CA NORTH AMERICA
-- 1	1-13 AU AUSTRALIA
-- 1	1-13 VE VENEZUELA
-- 6	3-9	 IL LSRAEL
function switch_region()
	local x = luci.model.uci.cursor()
	local country = luci.http.formvalue("country")
	local region
	
	x:set("wireless", "radio0", "country", country)
	if country == "CN" then
		region = 1
	elseif country == "TW" then
		region = 0
	elseif country == "JP" then
		region = 5
	elseif country == "FR" then
		region = 1
	elseif country == "CA" then
		region = 0
	elseif country == "AU" then
		region = 1
	elseif country == "VE" then
		region = 1
	elseif country == "IL" then
		region = 6
	end
	x:set("wireless", "radio0", "region", region)
	x:commit("wireless")
	luci.sys.reboot()

	luci.http.prepare_content("text/plain")
	luci.http.write("finish")
end

function confirm_fail()
	local x = luci.model.uci.cursor()
	local fail = luci.http.formvalue("fail")

	x:set("rxhf", "mode", "fail", fail)
	x:commit("rxhf")

	luci.http.prepare_content("text/plain")
	luci.http.write("finish")
end