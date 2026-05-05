#if defined _fakelag_validation_included
  #endinput
#endif
#define _fakelag_validation_included

public Extension __ext_fakelag_validation =
{
    name = "fakelag-validation-plugin",
    file = "fakelag-validation-plugin.ext.2.tf2",
    autoload = 1,
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};
