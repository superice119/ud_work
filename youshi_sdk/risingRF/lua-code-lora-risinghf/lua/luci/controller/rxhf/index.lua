module("luci.controller.rxhf.index", package.seeall)

function index()
	local root = node()
	if not root.target then
		root.target = alias("rxhf")
		root.index = true
	end

	local page   = node("rxhf")
	page.target  = firstchild()
	page.title   = _("risinghf")
	page.order   = 10
	page.sysauth = {"root", "admin"}
	page.sysauth_authenticator = "htmlauth"
	page.ucidata = true
	page.index = true

	-- Empty services menu to be populated by addons
	entry({"rxhf", "services"}, firstchild(), _("Services"), 40).index = true

	entry({"rxhf", "logout"}, call("action_logout"), _("Logout"), 90)
end

function action_logout()
	local dsp = require "luci.dispatcher"
	local utl = require "luci.util"
	local sid = dsp.context.authsession

	if sid then
		utl.ubus("session", "destroy", { ubus_rpc_session = sid })

		dsp.context.urltoken.stok = nil

		luci.http.header("Set-Cookie", "sysauth=%s; expires=%s; path=%s/" %{
			sid, 'Thu, 01 Jan 1970 01:00:00 GMT', dsp.build_url()
		})
	end

	luci.http.redirect(luci.dispatcher.build_url())
end