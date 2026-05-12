#define DEBUG

#define PLUGIN_NAME           "Thesis - Fakelag Mitigation"
#define PLUGIN_AUTHOR         "markriis"
#define PLUGIN_DESCRIPTION    "Mitigation plugin for Bachelor's thesis"
#define PLUGIN_VERSION        "1.0"
#define PLUGIN_URL            ""

#include <sourcemod>
#include <sdktools>
#include <fakelag_mitigation>

#pragma semicolon 1


public Plugin myinfo =
{
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
};

public OnPluginStart()
{
	PrintToServer("[fakelag-mitigation] plugin loaded");
}
