# Analysis and Mitigation of Source Engine Exploits in Team Fortress 2

Research workspace for Source engine / Team Fortress 2 server-side exploit validation & mitigation work.

This repository is organized around reproducible server-side experiments, using a SourceMod C++ extension for native server instrumentation, and a small SourcePawn plugin as the SourceMod-facing bridge.

## Layout

```text
bachelors-thesis/
  README.md

  <exploit-name>/
    validation/
      extension/
        SourceMod C++ validator extension source
        AMBuildScript
        AMBuilder
        configure.py                        -> configure script bundled with example extension
                                               to define dependency paths
      mitigation/
        SourceMod C++ mitigation extension source
        AMBuildScript
        AMBuilder
        configure.py


      plugin/
        thesis-<exploit-name>-validator.sp  -> loading extension
        include/
          <exploit-name>_validation.inc     -> compile-time metadata for sourcepawn
                                              (defining a dependency for the plugin)

```

## Used WSL Setup
```sh
# essentials
sudo apt update
sudo apt install -y build-essential \
                    clang gcc-multilib g++-multilib \
                    python3 python3-pip python3-venv python3-full \
                    git 

# ambuild (for compiling extension)
mkdir -p ~/tools ~/.venvs
cd ~/tools
git clone https://github.com/alliedmodders/ambuild
python3 -m venv ~/.venvs/ambuild
~/.venvs/ambuild/bin/python -m pip install -U pip setuptools wheel
~/.venvs/ambuild/bin/python -m pip install ~/tools/ambuild

# installing SDKs (dependencies listed in https://wiki.alliedmods.net/Writing_extensions#Manually_configuring_a_project)
mkdir ~/src
mkdir ~/src/alliedmodders

git clone https://github.com/alliedmodders/hl2sdk-manifests.git
git clone --recursive https://github.com/alliedmodders/sourcemod
git clone https://github.com/alliedmodders/metamod-source.git mmsource
git clone -b tf2 https://github.com/alliedmodders/hl2sdk.git hl2sdk-tf2

# example for creating a validation extension
# SourceMod's sample_ext was used as a starting template
cp -r ~/src/alliedmodders/sourcemod/public/sample_ext ~/src/<exploit-name>-validation
cd ~/src/<exploit-name>-validation
## remove unused dirs as development was done in VSCode
rmdir msvc8 msvc9 msvc12
mkdir build-linux
cd build-linux
## configuring project 
python3 ../configure.py \
    --hl2sdk-root ~/src/alliedmodders \
    --hl2sdk-manifest-path ~/src/alliedmodders/hl2sdk-manifests \
    --sm-path ~/src/alliedmodders/sourcemod \
    --mms-path ~/src/alliedmodders/mmsource \
    --sdks tf2 --targets x86_64 --enable-debug
# test build
source ~/.venvs/ambuild/bin/activate
ambuild
# builds to build-linux/package/addons/sourcemod/extensions/x64/<exploit-name>-validation-plugin.ext.2.tf2.so
```

Developing on VS Code required defined include paths & environment variables for intellisense, these were used:
```
/home/riis/src/alliedmodders/sourcemod/public
/home/riis/src/alliedmodders/sourcemod/public/extensions
/home/riis/src/alliedmodders/sourcemod/sourcepawn/include
/home/riis/src/alliedmodders/sourcemod/public/amtl
/home/riis/src/alliedmodders/sourcemod/public/amtl/amtl
/home/riis/src/alliedmodders/mmsource/core
/home/riis/src/alliedmodders/mmsource/core/sourcehook
/home/riis/src/alliedmodders/hl2sdk-tf2/public
/home/riis/src/alliedmodders/hl2sdk-tf2/public/engine
/home/riis/src/alliedmodders/hl2sdk-tf2/public/mathlib
/home/riis/src/alliedmodders/hl2sdk-tf2/public/vstdlib
/home/riis/src/alliedmodders/hl2sdk-tf2/public/tier0
/home/riis/src/alliedmodders/hl2sdk-tf2/public/tier1
/home/riis/src/alliedmodders/hl2sdk-tf2/public/toolframework
/home/riis/src/alliedmodders/hl2sdk-tf2/public/game/server
/home/riis/src/alliedmodders/hl2sdk-tf2/game/shared
/home/riis/src/alliedmodders/hl2sdk-tf2/common
```
```
_LINUX
LINUX
POSIX
SOURCEMOD_BUILD
SOURCE_ENGINE=12
SE_TF2=12
TF_DLL
```

## Extension Development

IDA Free was used to inspect TF2 Linux server binaries, including symbol names, vtables, call sites, and offsets. Runtime addresses were calculated as:

```text
runtime_address = module_base + ida_rva
``` 

Several experiments use [virtual function table patching](https://www.codereversing.com/archives/596). This is useful when the target behavior is reached through virtual dispatch, but it requires care:
- the offset must match the exact deployed server binary
- the correct runtime vtable must be patched, often a derived class vtable
- Linux `mprotect()` requires page-aligned addresses
- Linux x86_64 uses the System V ABI, not Windows `__thiscall` or `__fastcall` (this was a surprise)
This approach only applies when the target is reached through virtual dispatch. Non-virtual functions, direct calls, static functions, and free functions require a different hook strategy.

## Deployment
The compiled extension is deployed to
```text
<tf2 server root>/tf/addons/sourcemod/extensions/x64/<exploit-name>-validation-plugin.ext.2.tf2.so
```
  
A SourcePawn include file is used at compile time:
```text
<tf2 server root>/tf/addons/sourcemod/scripting/include/<exploit-name>_validation.inc
```
```sourcepawn
public Extension __ext_fakelag_validation =
{
    name = "<exploit-name>-validation-plugin",
    file = "<exploit-name>-validation-plugin.ext.2.tf2",
    autoload = 1,
    required = 1,
};
```
This metadata is compiled into the `.smx`. When SourceMod loads the plugin, it sees the required extension and autoloads the `.so`.   
  
SourcePawn plugins are not compiled at runtime by the server. The `.sp` source must be compiled with `spcomp64`:
```sh
cd <tf2 server root>/tf/addons/sourcemod/scripting
./spcomp64 thesis-<exploit-name>-validator.sp
mv thesis-<exploit-name>-validator.smx ../plugins/
```

Runtime files:
```text
<tf2 server root>/tf/addons/sourcemod/extensions/x64/<exploit-name>-validation-plugin.ext.2.tf2.so
<tf2 server root>/tf/addons/sourcemod/plugins/thesis-<exploit-name>-validator.smx
```

## Runtime Commands (in tf2's server CLI)
Loading an extension manually
```
sm exts load <exploit-name>-validation-plugin.ext.2.tf2
```
Other extension commands
```
sm exts list
sm exts reload <index>
sm exts unload <index>
```

Loading & unloading a plugin
```
sm plugins load thesis-<exploit-name>-validator
sm plugins unload <index>
```

## Why The Plugin Acts As A Bridge

The C++ extension performs native server instrumentation. It can inspect server memory, resolve Source SDK structures, hook virtual functions, and log native state that SourcePawn cannot represent safely.

The SourcePawn plugin is intentionally small. Its main purpose is to give SourceMod a normal plugin dependency relationship:

```text
.sp plugin
  #include <fakelag_validation>
      ↓
.inc dependency block compiled into .smx
      ↓
SourceMod loads .smx
      ↓
SourceMod sees required extension metadata
      ↓
SourceMod autoloads .so extension
```

## Logging
Logging in the extension was done via native C++ & provided APIs by SourceMod, eg:
```cpp
smutils->LogMessage( myself, "message here" );
```

## Useful Links
- [SourceMod Overview](https://wiki.alliedmods.net/SourceMod)
- [Writing Sourcemod Extensions](https://wiki.alliedmods.net/Writing_extensions)
- [Autoloading SourceMod Extensions](https://wiki.alliedmods.net/Writing_extensions#Automatic_Loading)
