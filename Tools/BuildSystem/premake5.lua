project "Premake"
	kind "Utility"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin/Intermediates/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{wks.location}/**premake5.lua"
	}

	postbuildmessage "Aurora Global: Regenerating project files with Premake5!"
	postbuildcommands
	{
		"\"%{prj.location}PremakeBin\\premake5\" %{_ACTION} --file=\"%{wks.location}premake5.lua\""
	}