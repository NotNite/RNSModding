# RNSModding

[![.github/workflows/build.yml](https://github.com/NotNite/RNSModding/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/NotNite/RNSModding/actions/workflows/build.yml)

Collection of mods for Rabbit and Steel.

You may be interested in the reverse engineering project alongside it, [Bouny](https://github.com/NotNite/Bouny).

## Installation

Download [Aurie Manager](https://github.com/AurieFramework/Aurie/releases/latest) and then select the Rabbit and Steel game executable. Click "Add Mods" and select the mod .dll(s) you want.

Mod downloads are available on GitHub Actions (for now):

- Click [here](https://github.com/NotNite/RNSModding/actions/workflows/build.yml?query=branch%3Amain) if you have a GitHub account
- Click [here](https://nightly.link/NotNite/RNSModding/workflows/build/main) if you do not have a GitHub account (uses third party nightly.link service)

## Notice about online play

When playing with a mod that changes gameplay significantly, multiplayer will only work with other players who are using modded clients. You must still be on the same game version. This is to prevent cheating, issues with synchronization, and having vanilla players' experiences be disrupted by mods.

The following mods restrict online play:

- SteelYourself

## Mods

### SteelYourself

Force encounters to be the same one over and over again. Create an `encounter.txt` next to the game .exe with the name of the encounter in it (e.g. `enc_rabbit_queen0`). Reference `SteelYourself/encounters.txt` for a list of encounters.

### TextureSwapper

Swap textures in data.win. Create `mods/TextureSwapper/id.png` where `id` is the index in the data.win file (e.g. 24). Not heavily tested!
