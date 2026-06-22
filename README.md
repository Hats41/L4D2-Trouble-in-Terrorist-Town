# Trouble in Terrorist Town (TTT) - Left 4 Dead 2

Welcome to the TTT game mode for L4D2!

## About the Game Mode
Trouble in Terrorist Town (TTT) is a mode of trust and deception. At the start of the round, players are secretly assigned one of three roles:
- **Innocents:** The majority. Their goal is to survive and figure out who the Traitors are.
- **Traitors:** A small group of hidden killers. Their goal is to eliminate all Innocents without getting caught.
- **Detectives:** Detectives with access to unique equipment. They lead the investigation to find the Traitors.

### Features
- **Karma System & Shop:** Play well to earn Karma and Credits, which you can spend in the built-in Shop to buy special items!
- **Large Server Support:** The mode is built to support up to **32 players** (currently experimental and not fully tested yet).
- **Built-in Lobby:** The mod comes with its own native lobby system to wait for players before a match begins. 
- **Multi-Language Support:** The plugin is fully translatable. It currently comes with **English and Spanish** support out of the box, which can be edited in `addons/sourcemod/translations/l4d2_ttt_phrases.txt`. 

## Requirements
Most server owners likely already have these installed, but ensure you have the following before starting:
- **SourceMod (v1.11+)** & **MetaMod:Source**
- **Left4DHooks**
- **SendProxy Extension** *(Note: This package includes `sendproxy.ext.dll` for Windows servers. If you are hosting on a Linux server, you must download and install `sendproxy.ext.so` yourself).*

## Installation
1. Drag and drop the `addons`, `cfg`, and `materials` folders into your server's main `left4dead2` directory.
2. **FastDL (Custom Content):** To ensure players automatically download the necessary custom materials (like sprites and UI elements) when joining your server, you must add the following lines to your `left4dead2/cfg/server.cfg` file:
   ```cfg
   sv_allowdownload 1
   sv_downloadurl "https://hats41.github.io/L4D2-Trouble-in-Terrorist-Town/"
   ```
3. Restart your server.

## Optional Plugins
In the `addons/sourcemod/plugins/optional/` folder, you will find some extra features:
- **Weapon & Ammo Spawners (`l4d_weapon_spawn.smx` & `l4d_ammo_spawn.smx`):** Recommended to properly populate the map with weapons.
- **Ready-Up (`readyup.smx`):** An advanced ready-up system. Since TTT already features its own built-in lobby, **Ready-Up is completely optional**. 
  - *If you choose NOT to use Ready-Up:* You can safely ignore or delete its related files (`readyup.smx`, `readyup.sp`, `readyup.inc`, the `readyup/` include folder, and `translations/readyup.phrases.txt`).
  - *If you DO want to use these optional plugins:* Simply move their `.smx` files from the `optional/` folder into your main `plugins/` folder.

## Game Mechanics & Configuration

### Map Cleanup & Custom Maps
When a round starts, the TTT plugin automatically cleans up the map by removing common infected, default map weapons, and potentially map events to ensure a fair TTT experience. 
*(Note: The game mode has not yet been fully tested on custom maps).*

### Weapon & Ammo Spawning
Since default weapons are removed, TTT relies on external spawn files to populate the map. If you use the optional `l4d_weapon_spawn` and `l4d_ammo_spawn` plugins, they will generate configuration files containing weapon positions. The TTT plugin reads these generated configurations to place weapons for the players.

### Player Spawns
The mod uses its own player spawn system managed via `addons/sourcemod/configs/l4d_ttt_spawns.cfg`. Server admins can manually configure custom spawn points using the following commands in-game:
- `sm_add_ttt_spawn`: Adds a new player spawn at your current position.
- `sm_del_ttt_spawn`: Deletes the last created spawn point.
- `sm_clear_ttt_spawn`: Removes all custom TTT spawns for the current map.

### Built-in RTV (Rock The Vote)
The mod features its own built-in RTV system tailored for TTT.
- This feature can be completely disabled if you prefer using a standard map chooser.
- You can specify which maps are available for voting by editing the `addons/sourcemod/configs/l4d_ttt_rtv.cfg` file.

### Karma & Credits System
The game tracks your behavior through a Karma system. Hitting or killing teammates reduces your Karma, while playing properly and eliminating targets grants you Credits. Your Karma score (out of 1000) dictates your reputation level:
- **Formal (900+):** 100% Outgoing Damage | 100% Incoming Damage.
- **Neutral (750 - 899):** 95% Outgoing Damage | 100% Incoming Damage.
- **Suspect (550 - 749):** 90% Outgoing Damage | 110% Incoming Damage.
- **Violent (400 - 549):** 85% Outgoing Damage | 115% Incoming Damage. **[Penalty:]** Blocked from buying Special Roles in the Shop and locked out of the `!detective` command.
- **Hostile (<400):** 80% Outgoing Damage | 120% Incoming Damage. **[Penalty:]** Completely blocked from opening or using the Shop.

- If your Karma drops to 0, you can be automatically forced into Spectator mode or forced to suicide.
- **Self-Defense Grace Period:** If an Innocent player shoots you first, you have a brief window (default 5 seconds, configurable via `ttt_karma_defend_window`) to return fire and kill them without suffering a Karma penalty. *(Note: This feature is experimental).*
- You can fully configure or completely disable the Karma system using the CVARs in `l4d2_ttt.cfg`.

### The Shop (`!shop` or `!tienda`)
Players can use their earned Credits to buy items and abilities during the match.
- The shop can be disabled entirely by setting `ttt_shop_enabled "0"`.
- **Item Prices:** You can change the price of any item. To **disable** a specific item, simply set its price to `-1` in the configuration file.
- **Available Items include:** Ammo Refill, Baseball Bat, Desert Eagle, Grenade Launcher, Pain Pills, Kevlar & Helmet.
- **Special Abilities & Tools:**
  - **C4 & Defuser:** Plant explosives or defuse planted C4s.
  - **Role Purchases:** "Traitor Now", "Traitor Next Round", or "Detective Now".
  - **Infected Commands:** Spawn a Horde, Special Infected, or a Tank to cause chaos!
  - **Utility:** Corpse Identifier, Immortality, Guided Missile, Taser, and Wallhack.
- **Lootbox:** A mystery box that grants a random item. It can also trigger one of 3 hidden secret drops:
  1. **Randomizer:** Swaps your loadout with random weapons.
  2. **Piano:** Drops a deadly piano from the sky!
  3. **Witch:** Spawns a Witch near your target.

### Main Configuration (`l4d2_ttt.cfg`)
When the server starts with the plugin for the first time, it automatically generates a master configuration file at `cfg/sourcemod/l4d2_ttt.cfg`. Here you can tweak practically everything to your liking:
- Ratio of Detectives and Traitors per round.
- Round duration and Truce duration (safe time at the start).
- RTV duration and extend rounds limit.
- Credit rewards (for killing, surviving, defusing C4, etc.).
- Complete Karma adjustments (penalties, damage multipliers, thresholds).
- Shop toggles, individual item prices, and ability cooldowns.

### Investigation & Detective Tools
Finding the Traitors requires careful investigation. TTT provides interactive ways to uncover information:
- **Corpse Scanning (Press 'E'):** When you find a dead body, get close and press your 'Use' key (Default: E). This will publicly identify the body and reveal vital information, such as the victim's name, their role, and the exact weapon used to kill them.
- **Corpse Identifier (Shop Item):** A specialized tool available in the shop. When used on a body, it performs an advanced forensic scan and reveals the ultimate clue: **the exact name of the killer!**
- **The Taser (Shop Item):** A powerful tool for investigation. If you manage to shoot another player with the Taser, their true role will be temporarily revealed to you for 3 seconds. Use it wisely to confirm your suspicions without shedding blood!

## Basic TTT Rules (For Players)
To ensure a fair and fun environment, it's recommended to enforce the following standard TTT rules on your server:

### General Rules
- **No RDM (Random Deathmatch):** Damaging or killing someone without valid proof is strictly prohibited.
- **No Sound-Whoring:** You cannot kill someone based solely on hearing a weapon shot or footstep. You must have visual proof or strong deductive evidence.
- **No Ghosting:** Dead players and spectators cannot reveal information to alive players (the plugin handles this natively, but outside communication like Discord is prohibited).

### Innocents
- **No T-Baiting:** Do not act like a Traitor (shooting randomly, hiding in suspicious spots, etc.) to bait others into attacking you. If you T-Bait and get killed, it is your fault.
- **Self-Defense:** You can defend yourself and kill anyone who damages you first.
- **Proof is Required:** You cannot kill based on mere suspicion, "gut feeling," or because someone is holding a specific weapon. You need concrete proof.
- **Unidentified Bodies:** If you find someone standing over a pile of unidentified bodies without reporting them, that is highly suspicious and can be grounds for a kill.
- **No Giving Orders:** Innocents cannot boss other players around; that is the Detective's job.

### Traitors
- **Teamwork:** Use `/t` to communicate privately with your fellow Traitors. Do not RDM or sabotage your Traitor teammates.
- **Give Warnings:** Before using massive damage items (like planting C4 or spawning a Horde), warn your team via `/t` so they can clear the area.
- **No Camping:** Traitors must actively try to win. Hiding in a corner for the entire round to delay the game is prohibited.
- **No Snitching:** Do not rat out your fellow Traitors to the Innocents.

### Detectives
- **Give Orders:** Detectives can give simple orders (e.g., "Step away from the body", "Hold still for a scan"). If a player repeatedly disobeys simple orders, they can be deemed a rebel and killed.
- **DNA / Evidence:** If you use your Detective tools (like the Corpse Identifier) and it points to a killer, you have the right to execute them (unless it was a proven self-defense scenario).
- **Communication:** Use `/d` to chat privately with other Detectives.

## Commands Reference

### Player Commands
- `!detective` - Opt-in to apply to become a Detective.
- `!t <message>` - Exclusive chat for Traitors.
- `!d <message>` - Exclusive chat for Detectives.
- `!shop` or `!tienda` - Opens the TTT Shop to buy items.
- `!credits` or `!vercreditos` - Check your current balance of Credits.

### Admin Commands (Requires ROOT Flag)
- `!setrole <player>` - Manually set a player's role (Innocent, Traitor, Detective).
- `!givecredits <player> <amount>` (or `!darcreditos`) - Grant credits to a specific player.
- `!addbot` - Spawns a bot bypassing the standard director limit.
- `!giveimmortality <player>` - Grants a player 3 seconds of immortality.
- `!shop_wall`, `!shop_piano`, `!shop_randomizer` - Force triggers these special shop events for testing.
- `!add_ttt_spawn`, `!del_ttt_spawn`, `!clear_ttt_spawns` - Used for managing custom player spawn points on the map.

## 32 Players Setup (Included in Package)
This package includes **L4DToolZ** to allow your server to host up to 32 players (experimental). 
1. Ensure your server's `server.cfg` includes:
   ```cfg
   sv_maxplayers 32
   sv_visiblemaxplayers 32
   ```
2. Once in-game, you can use the built-in admin command `!addbot` (included natively in the TTT plugin) to spawn bots bypassing the standard limit and test the mode at maximum capacity.

---

## Credits & Acknowledgements
This TTT package wouldn't be possible without the incredible work of the SourceMod community. Special thanks to the original authors of the included optional plugins:
- **Ready-Up Plugin:** Created by **CanadaRox** and **Target**.
- **Weapon Spawn & Ammo Spawn Plugins:** Created by **SilverShot** (Silvers).
