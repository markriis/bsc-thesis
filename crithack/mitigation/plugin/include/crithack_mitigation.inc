#if defined _crithack_mitigation_included
  #endinput
#endif
#define _crithack_mitigation_included

public Extension __ext_crithack_mitigation =
{
    name = "crithack-mitigation-plugin",
    file = "crithack-mitigation-plugin.ext.2.tf2",
    autoload = 1,
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};

