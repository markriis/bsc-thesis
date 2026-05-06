#define DEBUG

#define PLUGIN_NAME           "Thesis - Backtrack Hypothesis Validation"
#define PLUGIN_AUTHOR         "markriis"
#define PLUGIN_DESCRIPTION    "Validation plugin for Bachelor's thesis"
#define PLUGIN_VERSION        "1.0"
#define PLUGIN_URL            ""

#include <sourcemod>
#include <sdktools>
#include <backtrack_validation>

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
	PrintToServer("[backtrack-validation] plugin loaded");
}
