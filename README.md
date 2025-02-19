# MaplePS

Table of Contents
==================
- [Table of Contents](#table-of-contents)
  - [What is MaplePS?](#what-is-mapleps)
  - [Features](#features)
  - [How does it work?](#how-does-it-work)
  - [How do I get back on Bancho?](#how-do-i-get-back-on-bancho)
  - [Get Started](#get-started)

What is MaplePS?
------
MaplePS is a unofficial release of Maple after maple.software had their source code leaked.  
This is not intended for use on Bancho. **It will get you banned.** I promise.

Features
------
- Relax
- Aim Assist
- Timewarp
- Replay Bot
- Visuals (includes unreleased Enlighten & Taiko Mania overlays)
- Spoofer
- Disable Score Submission
- Disable Spectators
- Config System

How does it work?
------
MaplePS is basically just a injector with some extra steps.  
It uses a very basic `LoadLibrary` DLL injection method.

1. Open MaplePS.
2. MaplePS will ask you to locate your osu! folder.
3. MaplePS will then create a read-only `_staging` file in your osu! directory to prevent osu! from updating.
4. MaplePS will downgrade your `osu!auth.dll` to an older version that Maple supports.
5. MaplePS will prompt you to open osu!.
6. After you open osu!, simply press `Enter`.
7. MaplePS will then inject.

How do I get back on Bancho?
------
Simply remove the `_staging` and `osu!auth.dll` files from your osu! folder.

Get Started
------
Head over to the [Releases](https://github.com/yo-ru/MaplePS/releases) tab.
