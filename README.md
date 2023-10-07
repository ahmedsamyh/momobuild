# Momobuild

Build system for C/CPP made for myself.

This isn't technically a build system i guess? Because from what i understand, [premake5](https://premake.github.io/) and [CMake](https://cmake.org/) are build system generators. And [msbuild](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) and [GNU Make](https://www.gnu.org/software/make/) are the actual build systems. Since this just calls premake5 and msbuild, i guess it's an automator of some sort...

# Todo
- [ ] customize which build system generator to use
- [ ] customize which compiler to use
- [ ] automatically initiate a premake5 template
- [ ] maybe make `cpp-proj` part of this?


## References
- [premake5 5.0.0-beta2](https://github.com/premake/premake-core/releases)
- [msbuild from vc build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
