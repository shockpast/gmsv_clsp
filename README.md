# gmsv_clsp

gmsv_clsp is server plugin for Garry's Mod servers that rewrites `GModDataPack` to add multiple useful features for developers and casual server owners.

## features

- (planned) Lua API
  - `clsp.add` that allows to rule over file contents and modify them however you want

- multi-threaded files cache generation
  - vanilla server: compresses and saves cache for all files upon first player join (it would hang your server for a while, since it's done on main thread);
  - clsp server: compresses and saves cache for all files at the start of server splitting work evenly for all threads;

## support

windows or linux64 support is not planned, since most of owners use `main` or `dev` branches

## installation

1. put `addon_clsp.vdf` in `addons` folder
2. put `plugin_clsp_x86.so` in `lua/bin` folder
3. that's it!