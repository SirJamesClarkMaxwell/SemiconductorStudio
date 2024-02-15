include "utils.lua"



workspace "GUI"
	configurations { "Debug", "Release" }
	platforms { "x86", "x86_64" }

	location "./"

	srunt = "off"

	include "./Components/build.lua"
	include "./FittingTest/build.lua"
	include "./Sandbox/build.lua"
	include "./Vendor/build.lua"

