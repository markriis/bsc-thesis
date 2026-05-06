#if defined _backtrack_validation_included
  #endinput
#endif
#define _backtrack_validation_included

public Extension __ext_backtrack_validation =
{
    name = "backtrack-validation-plugin",
    file = "backtrack-validation-plugin.ext.2.tf2",
    autoload = 1,
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};

