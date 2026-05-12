#if defined _fakelag_mitigation_included
  #endinput
#endif
#define _fakelag_mitigation_included

public Extension __ext_fakelag_mitigation =
{
    name = "fakelag-mitigation-plugin",
    file = "fakelag-mitigation-plugin.ext.2.tf2",
    autoload = 1,
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};

