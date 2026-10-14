# Windows-view check: Immersive First Person

Base: tag `v2.1.2`.

Method:

`diff -u <(git show v2.1.2:native/src/Main.cpp | unifdef -U__APPLE__) <(unifdef -U__APPLE__ native/src/Main.cpp)`

`unifdef -U__APPLE__` drops `#ifdef __APPLE__` bodies, keeps `#else` bodies, and removes the directives.

## Result

The Windows view of `native/src/Main.cpp` matches v2.1.2. No hunk.

`ifp_registerNatives`, the Apple include of `MacSdkCompat.hpp`, and the skip of the two RTTI callback lines are inside `#ifdef __APPLE__`. `PostRegisterTypes`, `Query`, and `Supports` are the tag text. `Query` still sets runtime 2.31, version 0.2.0, name `Immersive First Person`, author `Immersive First Person contributors`.

`native/CMakeLists.txt` is unchanged from the tag. The Mac target is `macos/CMakeLists.txt`.
