#if defined _backtrack_mitigation_included
  #endinput
#endif
#define _backtrack_mitigation_included

public Extension __ext_backtrack_mitigation =
{
    name = "backtrack-mitigation-plugin",
    file = "backtrack-mitigation-plugin.ext.2.tf2",
    autoload = 1,
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};

