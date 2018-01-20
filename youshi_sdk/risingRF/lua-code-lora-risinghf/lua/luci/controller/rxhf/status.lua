module("luci.controller.rxhf.status", package.seeall)

function index()
	entry({"rxhf", "status"}, alias("rxhf", "status", "overview"), _("Status"), 20).index = true
	entry({"rxhf", "status", "overview"}, template("rxhf_status/index"), _("Overview"), 1)
end