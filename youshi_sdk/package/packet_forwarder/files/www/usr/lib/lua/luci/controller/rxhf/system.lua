module("luci.controller.rxhf.system", package.seeall)

function index()
	entry({"rxhf", "system"}, alias("rxhf", "system", "system"), _("System"), 40).index = true
	entry({"rxhf", "system", "system"}, cbi("rxhf_system/system"), _("System"), 1)
	entry({"rxhf", "system", "clock_status"}, call("action_clock_status"))
	entry({"rxhf", "system", "admin"}, cbi("rxhf_system/admin"), _("Administration"), 2)
	entry({"rxhf", "system", "reboot"}, call("action_reboot"), _("Reboot"), 90)
	entry({"rxhf", "system", "reset"}, call("action_reset"), _("Reset"), 91)
end

function action_clock_status()
	local set = tonumber(luci.http.formvalue("set"))
	if set ~= nil and set > 0 then
		local date = os.date("*t", set)
		if date then
			luci.sys.call("date -s '%04d-%02d-%02d %02d:%02d:%02d'" %{
				date.year, date.month, date.day, date.hour, date.min, date.sec
			})
		end
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json({ timestring = os.date("%c") })
end

function action_reboot()
	local reboot = luci.http.formvalue("reboot")
	luci.template.render("rxhf_system/reboot", {reboot=reboot})
	if reboot then
		luci.sys.reboot()
	end
end

function action_reset()
	local reset = luci.http.formvalue("reset")
	luci.template.render("rxhf_system/reset", {reset=reset})
	if reset then
		luci.util.exec("jffs2reset -y && reboot &")
	end
end