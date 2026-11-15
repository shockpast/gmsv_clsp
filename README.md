# gmsv_clsp

gmsv_clsp is server plugin for Garry's Mod servers that rewrites client lua files sending pipeline to add multiple useful features for developers and casual server owners.

## features

- lua modification on the fly
  - `clsp.add(identifier, rule, callback)` transforms client Lua files before they're sent to clients, while keeping original on disk;

- multi-threaded files cache generation
  - vanilla server: compresses and saves cache for all files upon first player join (it would hang your server for a while, since it's done on main thread);
  - clsp server: compresses and saves cache for all files at the start of server splitting work evenly for all threads;
  - map shutdown cancels remaining jobs, waits for in-flight file compression, and clears all queues and cached files;
  - plugin unload removes the hooks and joins all worker threads before releasing their state;

## api

```lua
-- function will throw, if identifier is not unique and was registered before;
-- route accepts glob patterns: 'addons/*', 'addons/**/*.lua', 'addons/**/cl_*.lua'
-- callback can return string to modify content with returned value, or nil to keep the original
function clsp.add(identifier: string, route: string, callback: fun(path: string, content: string): string)
```

## support

windows or linux64 support is not planned, since most of owners use `main` or `dev` branches

## installation

1. put `addon_clsp.vdf` in `addons` folder
2. put `plugin_clsp_x86.so` in `lua/bin` folder
3. that's it!