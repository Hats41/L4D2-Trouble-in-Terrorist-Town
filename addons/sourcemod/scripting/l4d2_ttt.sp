#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#undef REQUIRE_PLUGIN
#include <readyup>
#include <left4dhooks>

#define PLUGIN_VERSION "0.4-alpha"

#define ROLE_NONE 0
#define ROLE_INNOCENT 1
#define ROLE_TRAITOR 2
#define ROLE_DETECTIVE 3

public Plugin myinfo = 
{
	name = "[L4D2] Trouble in Terrorist Town",
	author = "Not HaTs",
	description = "TTT Gamemode for L4D2",
	version = PLUGIN_VERSION,
	url = ""
};

// Game State
bool g_bRoundLive = false;
int g_iPlayerRole[MAXPLAYERS + 1];
bool g_bWantsDetective[MAXPLAYERS + 1];
int g_iSpriteEntity[MAXPLAYERS + 1];

// Ragdoll Tracking
int g_RagdollRole[2048];
bool g_RagdollScanned[2048];
char g_RagdollName[2048][MAX_NAME_LENGTH];
int g_iLastButtons[MAXPLAYERS + 1];
float g_fLastChatHint[MAXPLAYERS + 1];

// Traitor Traps
bool g_bTraitorSpawning = false;
bool g_bTraitorSpawned[MAXPLAYERS + 1];

// Round Manager
int g_iCurrentRound = 1;
bool g_bTruceActive = false;
bool g_bRoundEnded = false;

// Weapon Config Data (read directly from l4d_spawn_weapon.cfg)
ArrayList g_aConfigPos;
ArrayList g_aConfigAng;
ArrayList g_aConfigMod;

ArrayList g_aAmmoPos;
ArrayList g_aAmmoAng;

public void OnPluginStart()
{
	g_aConfigPos = new ArrayList(3);
	g_aConfigAng = new ArrayList(3);
	g_aConfigMod = new ArrayList(1);
	
	g_aAmmoPos = new ArrayList(3);
	g_aAmmoAng = new ArrayList(3);
	
	HookEvent("round_start", Event_RoundStart, EventHookMode_PostNoCopy);
	HookEvent("round_end", Event_RoundEnd, EventHookMode_PostNoCopy);
	HookEvent("player_death", Event_PlayerDeath, EventHookMode_Pre);
	
	RegConsoleCmd("sm_detective", Cmd_Detective, "Opt-in to be a Detective");
	RegAdminCmd("sm_setrole", Cmd_SetRole, ADMFLAG_ROOT, "Set a player's TTT role");
	
	HookEvent("player_spawn", Event_PlayerSpawn, EventHookMode_Post);
	
	CreateConVar("ttt_version", PLUGIN_VERSION, "TTT Gamemode Version", FCVAR_NOTIFY|FCVAR_DONTRECORD);
	
	SetConVarInt(FindConVar("director_no_mobs"), 1);
	SetConVarInt(FindConVar("director_no_specials"), 1);
	SetConVarInt(FindConVar("director_no_bosses"), 1);
	SetConVarInt(FindConVar("z_common_limit"), 0);
	SetConVarInt(FindConVar("z_max_player_zombies"), 0);
	
	SetConVarInt(FindConVar("survivor_max_incapacitated_count"), 0);
	SetConVarInt(FindConVar("survivor_limp_health"), 0);
	SetConVarFloat(FindConVar("survivor_friendly_fire_factor_normal"), 0.7);
	SetConVarFloat(FindConVar("survivor_friendly_fire_factor_hard"), 0.7);
	SetConVarFloat(FindConVar("survivor_friendly_fire_factor_expert"), 0.7);
}

// Remove Timer_CheckAim completely

public Action Cmd_Detective(int client, int args)
{
	if (client > 0 && client <= MaxClients && IsClientInGame(client))
	{
		Menu menu = new Menu(MenuHandler_Detective);
		menu.SetTitle("¿Quieres ser Detective?\nRequisitos:\n- Conocer las reglas\n- Tener micrófono");
		menu.AddItem("yes", "Sí, quiero ser Detective");
		menu.AddItem("no", "No, gracias");
		menu.Display(client, MENU_TIME_FOREVER);
	}
	return Plugin_Handled;
}

public int MenuHandler_Detective(Menu menu, MenuAction action, int param1, int param2)
{
	if (action == MenuAction_Select)
	{
		char info[32];
		menu.GetItem(param2, info, sizeof(info));
		if (StrEqual(info, "yes"))
		{
			g_bWantsDetective[param1] = true;
			PrintToChat(param1, "\x05[TTT]\x01 Te has postulado para ser \x03Detective\x01.");
		}
		else
		{
			g_bWantsDetective[param1] = false;
			PrintToChat(param1, "\x05[TTT]\x01 Ya no eres candidato a Detective.");
		}
	}
	else if (action == MenuAction_End)
	{
		delete menu;
	}
	return 0;
}

public Action Cmd_SetRole(int client, int args)
{
	if (args < 2)
	{
		ReplyToCommand(client, "Usage: sm_setrole <target> <1|2|3> (1=Innocent, 2=Traitor, 3=Detective)");
		return Plugin_Handled;
	}
	
	char arg1[32], arg2[32];
	GetCmdArg(1, arg1, sizeof(arg1));
	GetCmdArg(2, arg2, sizeof(arg2));
	
	int role = StringToInt(arg2);
	if (role < 1 || role > 3) role = 1;
	
	char target_name[MAX_TARGET_LENGTH];
	int target_list[MAXPLAYERS], target_count;
	bool tn_is_ml;
	
	if ((target_count = ProcessTargetString(
			arg1,
			client,
			target_list,
			MAXPLAYERS,
			COMMAND_FILTER_ALIVE,
			target_name,
			sizeof(target_name),
			tn_is_ml)) <= 0)
	{
		ReplyToTargetError(client, target_count);
		return Plugin_Handled;
	}
	
	for (int i = 0; i < target_count; i++)
	{
		int target = target_list[i];
		if (GetClientTeam(target) != 2) continue;
		
		if (g_iSpriteEntity[target] > 0 && IsValidEntity(g_iSpriteEntity[target]))
		{
			AcceptEntityInput(g_iSpriteEntity[target], "Kill");
			g_iSpriteEntity[target] = 0;
		}
		
		g_iPlayerRole[target] = role;
		CreateRoleSprite(target, role);
		
		if (role == ROLE_TRAITOR) PrintToChat(target, "\x05[TTT]\x01 Eres un \x04TRAIDOR\x01 (Admin Override).");
		else if (role == ROLE_DETECTIVE) PrintToChat(target, "\x05[TTT]\x01 Eres un \x03DETECTIVE\x01 (Admin Override).");
		else PrintToChat(target, "\x05[TTT]\x01 Eres un \x05INOCENTE\x01 (Admin Override).");
	}
	
	ReplyToCommand(client, "Roles updated.");
	CheckWinConditions();
	return Plugin_Handled;
}

public void OnClientPutInServer(int client)
{
	SDKHook(client, SDKHook_OnTakeDamage, OnTakeDamage);
	SDKHook(client, SDKHook_TraceAttack, OnTraceAttack);
	
	Handle cv = FindConVar("mp_gamemode");
	if (cv != null) SendConVarValue(client, cv, "realism");
}

// ============================
// MAP & CONFIG
// ============================

public void OnMapStart()
{
	AddFileToDownloadsTable("materials/sprites/Tside/Tside.vmt");
	AddFileToDownloadsTable("materials/sprites/Tside/Tside.vtf");
	AddFileToDownloadsTable("materials/sprites/Dside/Dside.vmt");
	AddFileToDownloadsTable("materials/sprites/Dside/Dside.vtf");
	AddFileToDownloadsTable("materials/sprites/Iside/Iside.vmt");
	AddFileToDownloadsTable("materials/sprites/Iside/Iside.vtf");
	
	PrecacheModel("sprites/Tside/Tside.vmt", true);
	PrecacheModel("sprites/Dside/Dside.vmt", true);
	
	LoadWeaponsFromConfig();
	
	// Disable the external weapon plugin so it doesn't spawn weapons
	ServerCommand("sm plugins unload l4d_weapon_spawn");
	ServerCommand("sm plugins unload optional/l4d_weapon_spawn");
	
	// Disable external ammo plugin
	ServerCommand("sm plugins unload l4d_ammo_spawn");
	ServerCommand("sm plugins unload optional/l4d_ammo_spawn");
}

void LoadWeaponsFromConfig()
{
	g_aConfigPos.Clear();
	g_aConfigAng.Clear();
	g_aConfigMod.Clear();
	
	char path[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, path, sizeof(path), "data/l4d_spawn_weapon.cfg");
	
	KeyValues kv = new KeyValues("spawns");
	if (!kv.ImportFromFile(path))
	{
		delete kv;
		return;
	}
	
	char currentMap[64];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if (!kv.JumpToKey(currentMap))
	{
		delete kv;
		return;
	}
	
	if (kv.GotoFirstSubKey())
	{
		do
		{
			float pos[3], ang[3];
			kv.GetVector("pos", pos);
			kv.GetVector("ang", ang);
			int mod = kv.GetNum("MOD");
			
			g_aConfigPos.PushArray(pos, 3);
			g_aConfigAng.PushArray(ang, 3);
			g_aConfigMod.Push(mod);
		} while (kv.GotoNextKey());
	}
	
	delete kv;
	
	// Load ammo spawns
	g_aAmmoPos.Clear();
	g_aAmmoAng.Clear();
	
	char ammoPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, ammoPath, sizeof(ammoPath), "data/l4d_ammo_spawn.cfg");
	
	KeyValues kvAmmo = new KeyValues("spawns");
	if (kvAmmo.ImportFromFile(ammoPath))
	{
		if (kvAmmo.JumpToKey(currentMap) && kvAmmo.GotoFirstSubKey())
		{
			do
			{
				float pos[3], ang[3];
				kvAmmo.GetVector("pos", pos);
				kvAmmo.GetVector("ang", ang);
				
				g_aAmmoPos.PushArray(pos, 3);
				g_aAmmoAng.PushArray(ang, 3);
			} while (kvAmmo.GotoNextKey());
		}
	}
	delete kvAmmo;
}

// ============================
// ENTITY HOOKS
// ============================

public void OnEntityCreated(int entity, const char[] classname)
{
	if (StrEqual(classname, "weapon_ammo_spawn"))
	{
		SDKHook(entity, SDKHook_Use, OnAmmoUse);
	}
	else if (StrContains(classname, "weapon_") == 0 && StrContains(classname, "_spawn") == -1)
	{
		// Strip reserve ammo from actual weapons (not spawners)
		RequestFrame(Frame_SetWeaponAmmo, EntIndexToEntRef(entity));
	}
	else if (StrEqual(classname, "survivor_death_model"))
	{
		AcceptEntityInput(entity, "KillHierarchy");
	}
	else if (StrEqual(classname, "infected") || StrEqual(classname, "witch"))
	{
		if (!g_bTraitorSpawning)
		{
			AcceptEntityInput(entity, "KillHierarchy");
		}
	}
}

public void Event_PlayerSpawn(Event event, const char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(event.GetInt("userid"));
	if (client > 0 && client <= MaxClients && IsClientInGame(client))
	{
		if (IsFakeClient(client) && GetClientTeam(client) == 3)
		{
			if (!g_bTraitorSpawned[client])
			{
				KickClient(client, "Natural Special Infected blocked");
			}
		}
		
		if (GetClientTeam(client) == 2)
		{
			SetEntProp(client, Prop_Send, "m_bSurvivorGlowEnabled", 0);
			int hidehud = GetEntProp(client, Prop_Send, "m_iHideHUD");
			SetEntProp(client, Prop_Send, "m_iHideHUD", hidehud | 64);
		}
	}
}

public void Frame_SetWeaponAmmo(int ref)
{
	int entity = EntRefToEntIndex(ref);
	if (entity > 0 && IsValidEntity(entity))
	{
		if (HasEntProp(entity, Prop_Send, "m_iExtraPrimaryAmmo"))
		{
			SetEntProp(entity, Prop_Send, "m_iExtraPrimaryAmmo", 0);
		}
	}
}

public Action OnAmmoUse(int entity, int activator, int caller, UseType type, float value)
{
	if (activator > 0 && activator <= MaxClients && IsClientInGame(activator) && GetClientTeam(activator) == 2)
	{
		int weapon = GetPlayerWeaponSlot(activator, 0);
		if (weapon > 0 && IsValidEntity(weapon))
		{
			int ammotype = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType");
			if (ammotype != -1)
			{
				char cls[64];
				GetEntityClassname(weapon, cls, sizeof(cls));
				int addAmmo = 60;
				if (StrContains(cls, "shotgun") != -1) addAmmo = 16;
				else if (StrContains(cls, "smg") != -1) addAmmo = 100;
				else if (StrContains(cls, "rifle") != -1 || StrContains(cls, "sniper") != -1) addAmmo = 30;
				
				int currentAmmo = GetEntProp(activator, Prop_Send, "m_iAmmo", _, ammotype);
				SetEntProp(activator, Prop_Send, "m_iAmmo", currentAmmo + addAmmo, _, ammotype);
				AcceptEntityInput(entity, "Kill");
				PrintToChat(activator, "\x05[TTT]\x01 Has recogido un \x04Alijo de Munición\x01.");
				return Plugin_Handled;
			}
		}
	}
	return Plugin_Continue;
}

// ============================
// DAMAGE SYSTEM
// ============================

public Action OnTakeDamage(int victim, int &attacker, int &inflictor, float &damage, int &damagetype, int &weapon, float damageForce[3], float damagePosition[3], int damagecustom)
{
	if ((!g_bRoundLive && !g_bRoundEnded) || g_bTruceActive)
	{
		damage = 0.0;
		return Plugin_Handled;
	}
	return Plugin_Continue;
}

public Action OnTraceAttack(int victim, int &attacker, int &inflictor, float &damage, int &damagetype, int &ammotype, int hitbox, int hitgroup)
{
	if ((!g_bRoundLive && !g_bRoundEnded) || g_bTruceActive) return Plugin_Handled;
	
	if (victim > 0 && victim <= MaxClients && attacker > 0 && attacker <= MaxClients)
	{
		if (GetClientTeam(victim) == 2 && GetClientTeam(attacker) == 2)
		{
			if (hitgroup == 1) // HEAD
			{
				damage *= 4.0;
				return Plugin_Changed;
			}
			else
			{
				damage *= 1.0;
				return Plugin_Changed;
			}
		}
	}
	return Plugin_Continue;
}

// ============================
// CORPSE SCANNING & GLOWS
// ============================

public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon)
{
	if ((!g_bRoundLive && !g_bRoundEnded) || !IsPlayerAlive(client)) return Plugin_Continue;
	if (GetClientTeam(client) != 2) return Plugin_Continue;
	
	float eyeOrigin[3], eyeAngles[3];
	GetClientEyePosition(client, eyeOrigin);
	GetClientEyeAngles(client, eyeAngles);
	
	Handle trace = TR_TraceRayFilterEx(eyeOrigin, eyeAngles, MASK_SOLID, RayType_Infinite, TraceFilter_IgnorePlayers);
	if (TR_DidHit(trace))
	{
		int ent = TR_GetEntityIndex(trace);
		if (ent > MaxClients && IsValidEntity(ent))
		{
			char cls[64];
			GetEntityClassname(ent, cls, sizeof(cls));
			if (StrEqual(cls, "prop_ragdoll"))
			{
				float ePos[3];
				GetEntPropVector(ent, Prop_Send, "m_vecOrigin", ePos);
				if (GetVectorDistance(eyeOrigin, ePos) < 150.0)
				{
					if (!g_RagdollScanned[ent])
					{
						if (GetEngineTime() - g_fLastChatHint[client] > 3.0)
						{
							PrintToChat(client, "\x05[TTT]\x01 Presiona \x04[E]\x01 para inspeccionar el cadáver.");
							g_fLastChatHint[client] = GetEngineTime();
						}
						
						if ((buttons & IN_USE) && !(g_iLastButtons[client] & IN_USE))
						{
							g_RagdollScanned[ent] = true;
							ScanCorpse(client, ent);
						}
					}
				}
			}
		}
	}
	CloseHandle(trace);
	
	g_iLastButtons[client] = buttons;
	return Plugin_Continue;
}

public bool TraceFilter_IgnorePlayers(int entity, int contentsMask)
{
	if (entity == 0) return true;
	if (entity > 0 && entity <= MaxClients) return false;
	return true;
}

void ScanCorpse(int client, int ent)
{
	int role = g_RagdollRole[ent];
	
	SetEntityRenderMode(ent, RENDER_TRANSCOLOR);
	
	if (role == ROLE_TRAITOR)
	{
		SetEntityRenderColor(ent, 255, 0, 0, 255);
		PrintToChatAll("\x05[TTT]\x01 %N ha encontrado el cuerpo de \x04%s\x01. ¡Era un \x04TRAIDOR\x01!", client, g_RagdollName[ent]);
	}
	else if (role == ROLE_DETECTIVE)
	{
		SetEntityRenderColor(ent, 0, 0, 255, 255);
		PrintToChatAll("\x05[TTT]\x01 %N ha encontrado el cuerpo de \x04%s\x01. ¡Era un \x04DETECTIVE\x01!", client, g_RagdollName[ent]);
	}
	else
	{
		SetEntityRenderColor(ent, 0, 255, 0, 255);
		PrintToChatAll("\x05[TTT]\x01 %N ha encontrado el cuerpo de \x04%s\x01. ¡Era un \x04INOCENTE\x01!", client, g_RagdollName[ent]);
	}
}

// ============================
// ROUND MANAGEMENT
// ============================

int g_iCleanCount = 0;

public void Event_RoundStart(Event event, const char[] name, bool dontBroadcast)
{
	g_bRoundLive = false;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		g_iPlayerRole[i] = ROLE_NONE;
		if (g_iSpriteEntity[i] > 0 && IsValidEntity(g_iSpriteEntity[i]))
		{
			AcceptEntityInput(g_iSpriteEntity[i], "Kill");
			g_iSpriteEntity[i] = 0;
		}
	}
	
	for (int i = 0; i < 2048; i++)
	{
		g_RagdollRole[i] = ROLE_NONE;
		g_RagdollScanned[i] = false;
	}
	
	// Repeatedly clean lobby to catch all stragglers (every 2 seconds, 5 times)
	g_iCleanCount = 0;
	CreateTimer(2.0, Timer_CleanLobby, _, TIMER_REPEAT);
}

public void Event_RoundEnd(Event event, const char[] name, bool dontBroadcast)
{
	g_bRoundLive = false;
	for (int i = 1; i <= MaxClients; i++)
	{
		if (g_iSpriteEntity[i] > 0 && IsValidEntity(g_iSpriteEntity[i]))
		{
			AcceptEntityInput(g_iSpriteEntity[i], "Kill");
			g_iSpriteEntity[i] = 0;
		}
	}
}

public Action Timer_CleanLobby(Handle timer)
{
	ServerCommand("sm_weapon_spawn_clear");
	ServerCommand("sm_ammo_spawn_clear");
	CleanWorldWeapons();
	g_iCleanCount++;
	if (g_iCleanCount >= 5) return Plugin_Stop;
	return Plugin_Continue;
}

// Kill all unowned weapons, all spawner entities, all ragdolls, and all ammo crates
void CleanWorldWeapons()
{
	int toKill[2048];
	int killCount = 0;
	
	int ent = -1;
	while ((ent = FindEntityByClassname(ent, "weapon_*")) != -1)
	{
		if (!IsValidEntity(ent)) continue;
		
		char cls[64];
		GetEntityClassname(ent, cls, sizeof(cls));
		
		// KILL all spawner entities (weapon_spawn from external plugin + vanilla _spawn)
		if (StrContains(cls, "_spawn") != -1)
		{
			if (killCount < 2048) toKill[killCount++] = ent;
			continue;
		}
		
		// KILL unowned weapons on the ground
		if (HasEntProp(ent, Prop_Send, "m_hOwnerEntity"))
		{
			if (GetEntPropEnt(ent, Prop_Send, "m_hOwnerEntity") == -1)
			{
				if (killCount < 2048) toKill[killCount++] = ent;
			}
		}
		else
		{
			if (killCount < 2048) toKill[killCount++] = ent;
		}
	}
	
	// Kill ragdolls
	ent = -1;
	while ((ent = FindEntityByClassname(ent, "prop_ragdoll")) != -1)
	{
		if (IsValidEntity(ent))
		{
			if (killCount < 2048) toKill[killCount++] = ent;
		}
	}
	
	for (int i = 0; i < killCount; i++)
	{
		if (IsValidEntity(toKill[i])) AcceptEntityInput(toKill[i], "Kill");
	}
}

// Called by ReadyUp plugin when all players are ready
public void OnRoundIsLive()
{
	if (g_bRoundLive) return;
	
	// Strip all player weapons first
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && GetClientTeam(i) == 2)
		{
			for (int slot = 0; slot < 5; slot++)
			{
				int weapon = GetPlayerWeaponSlot(i, slot);
				if (weapon > 0 && IsValidEntity(weapon))
				{
					RemovePlayerItem(i, weapon);
					AcceptEntityInput(weapon, "Kill");
				}
			}
			int wep = CreateEntityByName("weapon_pistol");
			DispatchSpawn(wep);
			EquipPlayerWeapon(i, wep);
		}
	}
	
	// Clean any remaining world weapons (this kills ammo crates too)
	CleanWorldWeapons();
	
	// Spawn weapons from config file
	SpawnConfigWeapons();
	SpawnConfigAmmo();
	
	g_iCurrentRound = 1;
	g_bRoundEnded = false;
	StartTruce();
}

// ============================
// WEAPON SPAWNING FROM CONFIG
// ============================

void SpawnConfigWeapons()
{
	// MOD -> weapon classname mapping (matches l4d_weapon_spawn plugin by Silvers)
	char modWeapons[29][32] = {
		"weapon_pistol",                // 0
		"weapon_pistol_magnum",         // 1
		"weapon_rifle",                 // 2
		"weapon_rifle_ak47",            // 3
		"weapon_rifle_sg552",           // 4
		"weapon_rifle_desert",          // 5
		"weapon_autoshotgun",           // 6
		"weapon_shotgun_spas",          // 7
		"weapon_pumpshotgun",           // 8
		"weapon_shotgun_chrome",        // 9
		"weapon_smg",                   // 10
		"weapon_smg_silenced",          // 11
		"weapon_smg_mp5",               // 12
		"weapon_hunting_rifle",         // 13
		"weapon_sniper_awp",            // 14
		"weapon_sniper_military",       // 15
		"weapon_sniper_scout",          // 16
		"weapon_rifle_m60",             // 17
		"weapon_grenade_launcher",      // 18
		"weapon_chainsaw",              // 19
		"weapon_molotov",               // 20
		"weapon_pipe_bomb",             // 21
		"weapon_vomitjar",              // 22
		"weapon_pain_pills",            // 23
		"weapon_adrenaline",            // 24
		"weapon_first_aid_kit",         // 25
		"weapon_defibrillator",         // 26
		"weapon_upgradepack_explosive", // 27
		"weapon_upgradepack_incendiary" // 28
	};
	
	char meleeTypes[4][32] = { "fireaxe", "baseball_bat", "crowbar", "katana" };
	
	int len = g_aConfigPos.Length;
	for (int i = 0; i < len; i++)
	{
		int mod = g_aConfigMod.Get(i);
		if (mod < 0 || mod >= 29) continue;
		if (modWeapons[mod][0] == '\0') continue;
		
		float pos[3], ang[3];
		g_aConfigPos.GetArray(i, pos, 3);
		g_aConfigAng.GetArray(i, ang, 3);
		
		int wep = CreateEntityByName(modWeapons[mod]);
		if (wep > 0 && IsValidEntity(wep))
		{
			// Set count to prevent pickup from destroying the entity immediately
			DispatchKeyValue(wep, "count", "1");
			DispatchSpawn(wep);
			TeleportEntity(wep, pos, ang, NULL_VECTOR);
			
			// Freeze weapon in place so it doesn't fall through the floor
			SetEntityMoveType(wep, MOVETYPE_NONE);
		}
	}
}

void SpawnConfigAmmo()
{
	int len = g_aAmmoPos.Length;
	for (int i = 0; i < len; i++)
	{
		float pos[3], ang[3];
		g_aAmmoPos.GetArray(i, pos, 3);
		g_aAmmoAng.GetArray(i, ang, 3);
		
		int wep = CreateEntityByName("weapon_ammo_spawn");
		if (wep > 0 && IsValidEntity(wep))
		{
			// Give it 1 model count like standard crates
			DispatchKeyValue(wep, "count", "1");
			DispatchSpawn(wep);
			TeleportEntity(wep, pos, ang, NULL_VECTOR);
		}
	}
}

// ============================
// TRUCE & ROLES
// ============================

void StartTruce()
{
	g_bRoundLive = true;
	g_bTruceActive = true;
	g_bRoundEnded = false;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i) && GetClientTeam(i) == 2)
		{
			SetEntityModel(i, "models/survivors/survivor_gambler.mdl");
			SetEntProp(i, Prop_Send, "m_survivorCharacter", 0);
			// Disable default survivor outlines and floating names, and hide bottom HUD
			SetEntProp(i, Prop_Send, "m_bSurvivorGlowEnabled", 0);
			int hidehud = GetEntProp(i, Prop_Send, "m_iHideHUD");
			SetEntProp(i, Prop_Send, "m_iHideHUD", hidehud | 64);
		}
	}
	
	TeleportPlayersToSpawns();
	
	PrintToChatAll("\x05[TTT]\x01 Ronda \x04%d\x01 de \x0412\x01 ha comenzado.", g_iCurrentRound);
	PrintToChatAll("\x05[TTT]\x01 ¡Tienen \x0410 segundos de TREGUA\x01 para recolectar armas!");
	
	CreateTimer(10.0, Timer_EndTruce);
}

void TeleportPlayersToSpawns()
{
	if (g_aConfigPos.Length == 0) return;
	
	ArrayList availableSpawns = g_aConfigPos.Clone();
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i) && GetClientTeam(i) == 2)
		{
			if (availableSpawns.Length == 0)
			{
				delete availableSpawns;
				availableSpawns = g_aConfigPos.Clone();
			}
			
			int rand = GetRandomInt(0, availableSpawns.Length - 1);
			float pos[3];
			availableSpawns.GetArray(rand, pos, 3);
			availableSpawns.Erase(rand);
			
			pos[2] += 20.0;
			TeleportEntity(i, pos, NULL_VECTOR, NULL_VECTOR);
		}
	}
	delete availableSpawns;
}

public Action Timer_EndTruce(Handle timer)
{
	g_bTruceActive = false;
	AssignRoles();
	return Plugin_Stop;
}

void AssignRoles()
{
	int players[MAXPLAYERS + 1];
	int count = 0;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i) && GetClientTeam(i) == 2)
		{
			players[count] = i;
			count++;
			g_iPlayerRole[i] = ROLE_INNOCENT;
		}
	}
	
	if (count == 0) return;
	
	// Determine number of Detectives
	int numDetectives = 0;
	if (count >= 4)
	{
		if (count > 20) numDetectives = 3;
		else if (count > 12) numDetectives = 2;
		else numDetectives = 1;
	}
	
	int optInPool[MAXPLAYERS + 1];
	int optInCount = 0;
	for (int i = 0; i < count; i++)
	{
		if (g_bWantsDetective[players[i]]) optInPool[optInCount++] = players[i];
	}
	
	int assignedDetectives = 0;
	while (assignedDetectives < numDetectives && optInCount > 0)
	{
		int randIdx = GetRandomInt(0, optInCount - 1);
		int client = optInPool[randIdx];
		if (g_iPlayerRole[client] == ROLE_INNOCENT)
		{
			g_iPlayerRole[client] = ROLE_DETECTIVE;
			assignedDetectives++;
			optInPool[randIdx] = optInPool[optInCount - 1];
			optInCount--;
		}
	}
	
	int numTraitors = count / 3;
	if (numTraitors < 1) numTraitors = 1;
	
	int assignedTraitors = 0;
	while (assignedTraitors < numTraitors)
	{
		int randIdx = GetRandomInt(0, count - 1);
		int client = players[randIdx];
		if (g_iPlayerRole[client] == ROLE_INNOCENT)
		{
			g_iPlayerRole[client] = ROLE_TRAITOR;
			assignedTraitors++;
		}
	}
	
	PrintToChatAll("\x05[TTT]\x01 ¡LA RONDA HA COMENZADO!");
	PrintToChatAll("\x05[TTT]\x01 \x04%d TRAIDOR(ES)\x01 Y \x03%d DETECTIVE(S)\x01 HAN SIDO SELECCIONADOS.", numTraitors, assignedDetectives);
	
	for (int i = 0; i < count; i++)
	{
		int client = players[i];
		if (g_iPlayerRole[client] == ROLE_TRAITOR)
		{
			PrintToChat(client, "\x05[TTT]\x01 Eres un \x04TRAIDOR\x01.");
			CreateRoleSprite(client, ROLE_TRAITOR);
		}
		else if (g_iPlayerRole[client] == ROLE_DETECTIVE)
		{
			PrintToChat(client, "\x05[TTT]\x01 Eres un \x03DETECTIVE\x01.");
			CreateRoleSprite(client, ROLE_DETECTIVE);
		}
		else
		{
			PrintToChat(client, "\x05[TTT]\x01 Eres un \x05INOCENTE\x01.");
		}
	}
}

// ============================
// DEATH & WIN CONDITIONS
// ============================


public void Event_PlayerDeath(Event event, const char[] name, bool dontBroadcast)
{
	if (!g_bRoundLive && !g_bRoundEnded) return;
	
	event.BroadcastDisabled = true; // Hide killfeed
	
	int client = GetClientOfUserId(event.GetInt("userid"));
	if (client > 0 && client <= MaxClients && GetClientTeam(client) == 2)
	{
		float pos[3], ang[3];
		GetClientAbsOrigin(client, pos);
		GetClientAbsAngles(client, ang);
		
		int ragdoll = CreateEntityByName("prop_ragdoll");
		if (ragdoll != -1)
		{
			DispatchKeyValue(ragdoll, "model", "models/survivors/survivor_gambler.mdl");
			DispatchKeyValueVector(ragdoll, "origin", pos);
			DispatchKeyValueVector(ragdoll, "angles", ang);
			DispatchSpawn(ragdoll);
			
			// Debris collision so players don't get pushed
			SetEntProp(ragdoll, Prop_Data, "m_CollisionGroup", 2);
			
			// Random velocity for variety
			float vel[3];
			vel[0] = GetRandomFloat(-150.0, 150.0);
			vel[1] = GetRandomFloat(-150.0, 150.0);
			vel[2] = GetRandomFloat(100.0, 250.0);
			TeleportEntity(ragdoll, NULL_VECTOR, NULL_VECTOR, vel);
			
			g_RagdollRole[ragdoll] = g_iPlayerRole[client];
			GetClientName(client, g_RagdollName[ragdoll], MAX_NAME_LENGTH);
			g_RagdollScanned[ragdoll] = false;
		}
		
		g_iPlayerRole[client] = ROLE_NONE;
		CheckWinConditions();
	}
}

void CheckWinConditions()
{
	if (!g_bRoundLive) return;
	
	int innocentsAlive = 0;
	int traitorsAlive = 0;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i) && GetClientTeam(i) == 2)
		{
			if (g_iPlayerRole[i] == ROLE_TRAITOR) traitorsAlive++;
			else if (g_iPlayerRole[i] == ROLE_INNOCENT || g_iPlayerRole[i] == ROLE_DETECTIVE) innocentsAlive++;
		}
	}
	
	if (traitorsAlive == 0)
	{
		g_bRoundLive = false;
		g_bRoundEnded = true;
		PrintToChatAll("\x05[TTT]\x01 ¡Todos los \x04Traidores\x01 han muerto! ¡Los \x05Inocentes\x01 GANAN!");
		CreateTimer(5.0, Timer_RestartRound);
	}
	else if (innocentsAlive == 0)
	{
		g_bRoundLive = false;
		g_bRoundEnded = true;
		PrintToChatAll("\x05[TTT]\x01 ¡Todos los \x05Inocentes\x01 han sido eliminados! ¡Los \x04Traidores\x01 GANAN!");
		CreateTimer(5.0, Timer_RestartRound);
	}
}

public Action Timer_RestartRound(Handle timer)
{
	g_bRoundEnded = false;
	g_iCurrentRound++;
	
	if (g_iCurrentRound > 12)
	{
		PrintToChatAll("\x05[TTT]\x01 ¡Las 12 rondas han terminado! Reiniciando mapa...");
		char currentMap[64];
		GetCurrentMap(currentMap, sizeof(currentMap));
		ServerCommand("changelevel %s", currentMap);
		return Plugin_Stop;
	}
	
	// Strip player weapons and revive
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && GetClientTeam(i) == 2)
		{
			if (!IsPlayerAlive(i)) L4D_RespawnPlayer(i);
			SetEntityHealth(i, 100);
			
			for (int slot = 0; slot < 5; slot++)
			{
				int weapon = GetPlayerWeaponSlot(i, slot);
				if (weapon > 0 && IsValidEntity(weapon))
				{
					RemovePlayerItem(i, weapon);
					AcceptEntityInput(weapon, "Kill");
				}
			}
			
			int wep = CreateEntityByName("weapon_pistol");
			DispatchSpawn(wep);
			EquipPlayerWeapon(i, wep);
			
			if (g_iSpriteEntity[i] > 0 && IsValidEntity(g_iSpriteEntity[i]))
			{
				AcceptEntityInput(g_iSpriteEntity[i], "Kill");
				g_iSpriteEntity[i] = 0;
			}
			g_iPlayerRole[i] = ROLE_NONE;
		}
	}
	
	// Clear ragdoll tracking
	for (int i = 0; i < 2048; i++)
	{
		g_RagdollRole[i] = ROLE_NONE;
		g_RagdollScanned[i] = false;
	}
	
	// Clean world and respawn weapons from config
	CleanWorldWeapons();
	SpawnConfigWeapons();
	SpawnConfigAmmo();
	
	// Start next round
	StartTruce();
	
	return Plugin_Stop;
}

void CreateRoleSprite(int client, int role)
{
	if (g_iSpriteEntity[client] > 0 && IsValidEntity(g_iSpriteEntity[client]))
	{
		AcceptEntityInput(g_iSpriteEntity[client], "Kill");
		g_iSpriteEntity[client] = 0;
	}
	
	if (role == ROLE_INNOCENT || role == ROLE_NONE) return;
	
	int sprite = CreateEntityByName("env_sprite");
	if (sprite == -1) return;
	
	if (role == ROLE_TRAITOR)
	{
		DispatchKeyValue(sprite, "model", "sprites/Tside/Tside.vmt");
		DispatchKeyValue(sprite, "rendercolor", "255 255 255");
	}
	else if (role == ROLE_DETECTIVE)
	{
		DispatchKeyValue(sprite, "model", "sprites/Dside/Dside.vmt");
		DispatchKeyValue(sprite, "rendercolor", "255 255 255");
	}
		
	float pos[3];
	GetClientAbsOrigin(client, pos);
	pos[2] += 80.0;
	DispatchKeyValueVector(sprite, "origin", pos);
	
	DispatchSpawn(sprite);
	
	SetVariantString("!activator");
	AcceptEntityInput(sprite, "SetParent", client, sprite, 0);
	
	SDKHook(sprite, SDKHook_SetTransmit, OnSpriteTransmit);
	g_iSpriteEntity[client] = sprite;
}

public Action OnSpriteTransmit(int entity, int client)
{
	if (client <= 0 || client > MaxClients || !IsClientInGame(client)) return Plugin_Continue;
	
	int owner = -1;
	for (int i = 1; i <= MaxClients; i++)
	{
		if (g_iSpriteEntity[i] == entity)
		{
			owner = i;
			break;
		}
	}
	
	if (owner == -1) return Plugin_Continue;
	if (client == owner) return Plugin_Handled; // Don't see own sprite
	
	if (g_iPlayerRole[owner] == ROLE_TRAITOR && g_iPlayerRole[client] == ROLE_TRAITOR) return Plugin_Continue;
	if (g_iPlayerRole[owner] == ROLE_DETECTIVE && g_iPlayerRole[client] == ROLE_DETECTIVE) return Plugin_Continue;
	
	return Plugin_Handled; // Hide from everyone else
}
