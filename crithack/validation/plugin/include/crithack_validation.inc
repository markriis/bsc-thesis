#if defined _crithack_validation_included
  #endinput
#endif
#define _crithack_validation_included

public Extension __ext_crithack_validation =
{
    name = "crithack-validation-plugin",
    file = "crithack-validation-plugin.ext.2.tf2",
    autoload = 1,
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};

