# Immersive First Person on macOS: not done yet

The dylib builds. It has not been booted in the game. No in-game check of the height graph has been run. That check (the native returns Compatible) is later work.

## Class size

`anim::AnimVariableContainer` is 0x218 bytes in the Mac game's own reflection. The SDK header asserts 0x248. The difference is the tail after `transformVariables` (that array ends at 0x98). This plugin walks `floatVariables` only. It does not use `sizeof` on the container and it does not read the tail, so the header was not changed. The shared SDK checkout was not edited.

## Not proven by a dedicated field reader

`AnimatedComponent.graph` is a resource reference at 0x150 in the Mac reflection dump and in both arms of the SDK header. The resource-token helpers (loaded flag at 0x58, error byte at 0x5C, resource handle at 0x28) were read in the Mac binary and match the SDK. No function was found that loads this component's graph token and then tests the token flags on the same base. A function that uses 0x150 as a dynamic-array cursor is a different class and is not evidence for this field.

`AnimVariableFloat.name` is a `CName` at 0x30 in the Mac reflection dump and in the SDK. The plugin compares the name inline. No dedicated name-compare function was used as evidence.

## Addresses that trap

`Memory_Vault_Unk1` (434901863) and `Memory_PoolStorage_OOM` (3578730042) are the SDK allocator cold paths. They are not in the address table and they are not redirected to the local vault stand-ins, so they hit the fail-safe trap (log once, stub returns 0). They run only on an out-of-memory path.

`TTypedClass_IsEqual` is in the injected Codeware rows (hash 1482886888, Mac 0x2197FE4). This plugin does not instantiate that call.

## Registration

The plugin does not install hooks and does not add address rows of its own. `Query` still reports runtime 2.31. On this SDK that macro is file version 3.0.80.51928. Whether the Mac loader accepts that file version is an in-game check. `Supports()` returns the loader's latest API version (this SDK has one, version 0).
