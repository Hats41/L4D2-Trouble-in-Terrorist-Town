// l4d2_ttt_karma.inc

#if defined _l4d2_ttt_karma_included
 #endinput
#endif
#define _l4d2_ttt_karma_included

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
int g_iKarma[MAXPLAYERS + 1];           // Real Karma (0-1000)
int g_iActiveKarma[MAXPLAYERS + 1];
int g_iCleanRounds[MAXPLAYERS + 1];
int g_iKarmaZeroRounds[MAXPLAYERS + 1];
bool g_bKarmaChangedThisRound[MAXPLAYERS + 1];
bool g_bHadPenaltyThisRound[MAXPLAYERS + 1];
bool g_bHasValidKillThisRound[MAXPLAYERS + 1];

float g_fLastDamageTime[MAXPLAYERS + 1][MAXPLAYERS + 1];

char g_sKarmaLog[MAXPLAYERS + 1][15][64];
int g_iKarmaLogPoints[MAXPLAYERS + 1][15];
int g_iKarmaLogCount[MAXPLAYERS + 1];

char g_sKarmaLog_Prev[MAXPLAYERS + 1][15][64];
int g_iKarmaLogPoints_Prev[MAXPLAYERS + 1][15];
int g_iKarmaLogCount_Prev[MAXPLAYERS + 1];

// Cookies
Handle g_hCookieKarma = null;
Handle g_hCookieCleanRounds = null;

// ConVars
ConVar cv_ttt_karma_enabled;
ConVar cv_ttt_karma_penalty_teamkill;
ConVar cv_ttt_karma_defend_window;
ConVar cv_ttt_karma_zero_autospectator;
ConVar cv_ttt_karma_zero_autosuicide;

ConVar cv_ttt_karma_mult_formal;
ConVar cv_ttt_karma_mult_neutral;
ConVar cv_ttt_karma_mult_suspect;
ConVar cv_ttt_karma_mult_violent;
ConVar cv_ttt_karma_mult_hostile;

ConVar cv_ttt_karma_dmg_formal;
ConVar cv_ttt_karma_dmg_neutral;
ConVar cv_ttt_karma_dmg_suspect;
ConVar cv_ttt_karma_dmg_violent;
ConVar cv_ttt_karma_dmg_hostile;

// -----------------------------------------------------------------------------
// Base Functions
// -----------------------------------------------------------------------------
void Karma_OnPluginStart()
{
	cv_ttt_karma_enabled = CreateConVar("ttt_karma_enabled", "1", "Enable the Karma system");
	cv_ttt_karma_penalty_teamkill = CreateConVar("ttt_karma_penalty_teamkill", "25", "Base penalty for killing a teammate");
	cv_ttt_karma_defend_window = CreateConVar("ttt_karma_defend_window", "5.0", "Time window (seconds) for legitimate defense");
	cv_ttt_karma_zero_autospectator = CreateConVar("ttt_karma_zero_autospectator", "1", "Move to spectator next round if Karma is 0");
	cv_ttt_karma_zero_autosuicide = CreateConVar("ttt_karma_zero_autosuicide", "0", "Force suicide next round if Karma is 0");

	cv_ttt_karma_mult_formal = CreateConVar("ttt_karma_mult_formal", "1.00", "Outgoing damage multiplier Formal");
	cv_ttt_karma_mult_neutral = CreateConVar("ttt_karma_mult_neutral", "0.95", "Outgoing damage multiplier Neutral");
	cv_ttt_karma_mult_suspect = CreateConVar("ttt_karma_mult_suspect", "0.90", "Outgoing damage multiplier Suspect");
	cv_ttt_karma_mult_violent = CreateConVar("ttt_karma_mult_violent", "0.85", "Outgoing damage multiplier Violent");
	cv_ttt_karma_mult_hostile = CreateConVar("ttt_karma_mult_hostile", "0.80", "Outgoing damage multiplier Hostile");

	cv_ttt_karma_dmg_formal = CreateConVar("ttt_karma_dmg_formal", "1.00", "Incoming damage multiplier Formal");
	cv_ttt_karma_dmg_neutral = CreateConVar("ttt_karma_dmg_neutral", "1.00", "Incoming damage multiplier Neutral");
	cv_ttt_karma_dmg_suspect = CreateConVar("ttt_karma_dmg_suspect", "1.10", "Incoming damage multiplier Suspect");
	cv_ttt_karma_dmg_violent = CreateConVar("ttt_karma_dmg_violent", "1.15", "Incoming damage multiplier Violent");
	cv_ttt_karma_dmg_hostile = CreateConVar("ttt_karma_dmg_hostile", "1.20", "Incoming damage multiplier Hostile");

	g_hCookieKarma = RegClientCookie("ttt_karma_value", "Current Karma value", CookieAccess_Private);
	g_hCookieCleanRounds = RegClientCookie("ttt_karma_clean_rounds", "Streak of clean rounds", CookieAccess_Private);

	RegConsoleCmd("sm_karma", Cmd_Karma, "Shows your current Karma reputation");
	RegAdminCmd("sm_viewkarma", Cmd_ViewKarma, ADMFLAG_ROOT, "Shows extended Karma info of a player");
	RegAdminCmd("sm_setkarma", Cmd_SetKarma, ADMFLAG_ROOT, "Sets the Karma of a player");
	RegAdminCmd("sm_addkarma", Cmd_AddKarma, ADMFLAG_ROOT, "Adds or subtracts Karma from a player");
}

void Karma_OnClientPutInServer(int client)
{
	g_iKarma[client] = 1000;
	g_iActiveKarma[client] = 1000;
	g_iCleanRounds[client] = 0;
	g_iKarmaZeroRounds[client] = 0;
	g_bKarmaChangedThisRound[client] = false;
	g_bHadPenaltyThisRound[client] = false;
	g_bHasValidKillThisRound[client] = false;
	g_iKarmaLogCount[client] = 0;
	g_iKarmaLogCount_Prev[client] = 0;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		g_fLastDamageTime[client][i] = 0.0;
		g_fLastDamageTime[i][client] = 0.0;
	}

	if (AreClientCookiesCached(client))
	{
		Karma_LoadCookies(client);
	}
}

void Karma_OnClientCookiesCached(int client)
{
	if (IsClientInGame(client))
	{
		Karma_LoadCookies(client);
	}
}

void Karma_LoadCookies(int client)
{
	if (IsFakeClient(client)) return;
	
	char buffer[32];
	GetClientCookie(client, g_hCookieKarma, buffer, sizeof(buffer));
	if (buffer[0] != '\0')
	{
		g_iKarma[client] = StringToInt(buffer);
	}
	else
	{
		g_iKarma[client] = 1000;
	}
	
	GetClientCookie(client, g_hCookieCleanRounds, buffer, sizeof(buffer));
	if (buffer[0] != '\0')
	{
		g_iCleanRounds[client] = StringToInt(buffer);
	}
	else
	{
		g_iCleanRounds[client] = 0;
	}
	
	// Clamp loaded values
	if (g_iKarma[client] > 1000) g_iKarma[client] = 1000;
	if (g_iKarma[client] < 0) g_iKarma[client] = 0;
	
	g_iActiveKarma[client] = g_iKarma[client];
}

void Karma_SaveCookies(int client)
{
	if (IsFakeClient(client)) return;
	if (!AreClientCookiesCached(client)) return;
	
	char buffer[32];
	IntToString(g_iKarma[client], buffer, sizeof(buffer));
	SetClientCookie(client, g_hCookieKarma, buffer);
	
	IntToString(g_iCleanRounds[client], buffer, sizeof(buffer));
	SetClientCookie(client, g_hCookieCleanRounds, buffer);
}

void Karma_Modify(int client, int amount, bool isPenalty)
{
	if (client <= 0 || client > MaxClients || !IsClientInGame(client)) return;
	
	g_iKarma[client] += amount;
	if (g_iKarma[client] > 1000) g_iKarma[client] = 1000;
	if (g_iKarma[client] < 0) g_iKarma[client] = 0;
	
	g_bKarmaChangedThisRound[client] = true;
	
	if (isPenalty)
	{
		g_bHadPenaltyThisRound[client] = true;
		g_iCleanRounds[client] = 0;
	}
	
	Karma_SaveCookies(client);
}

void Karma_LogEvent(int client, const char[] reason, int amount)
{
	if (client <= 0 || client > MaxClients) return;
	if (g_iKarmaLogCount[client] < 15)
	{
		strcopy(g_sKarmaLog[client][g_iKarmaLogCount[client]], 64, reason);
		g_iKarmaLogPoints[client][g_iKarmaLogCount[client]] = amount;
		g_iKarmaLogCount[client]++;
	}
}

// -----------------------------------------------------------------------------
// Range Utilities
// -----------------------------------------------------------------------------
void Karma_GetRankName(int client, int karma, char[] buffer, int maxlen)
{
	if (karma >= 900) Format(buffer, maxlen, "%T", "Karma_Rank_Formal", client);
	else if (karma >= 750) Format(buffer, maxlen, "%T", "Karma_Rank_Neutral", client);
	else if (karma >= 550) Format(buffer, maxlen, "%T", "Karma_Rank_Suspect", client);
	else if (karma >= 400) Format(buffer, maxlen, "%T", "Karma_Rank_Violent", client);
	else Format(buffer, maxlen, "%T", "Karma_Rank_Hostile", client);
}

float Karma_GetOutgoingDamageMult(int karma)
{
	if (karma >= 900) return cv_ttt_karma_mult_formal.FloatValue;
	if (karma >= 750) return cv_ttt_karma_mult_neutral.FloatValue;
	if (karma >= 550) return cv_ttt_karma_mult_suspect.FloatValue;
	if (karma >= 400) return cv_ttt_karma_mult_violent.FloatValue;
	return cv_ttt_karma_mult_hostile.FloatValue;
}

float Karma_GetIncomingDamageMult(int karma)
{
	if (karma >= 900) return cv_ttt_karma_dmg_formal.FloatValue;
	if (karma >= 750) return cv_ttt_karma_dmg_neutral.FloatValue;
	if (karma >= 550) return cv_ttt_karma_dmg_suspect.FloatValue;
	if (karma >= 400) return cv_ttt_karma_dmg_violent.FloatValue;
	return cv_ttt_karma_dmg_hostile.FloatValue;
}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------
float g_fLastKarmaCmdTime[MAXPLAYERS + 1];

public Action Cmd_Karma(int client, int args)
{
	if (client == 0) return Plugin_Handled;
	
	if (GetEngineTime() - g_fLastKarmaCmdTime[client] < 0.5) return Plugin_Handled;
	g_fLastKarmaCmdTime[client] = GetEngineTime();
	
	char rank[32];
	Karma_GetRankName(client, g_iActiveKarma[client], rank, sizeof(rank));
	
	TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Karma_Current", rank, g_iActiveKarma[client]);
	
	if (g_iKarmaLogCount_Prev[client] > 0)
	{
		TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Karma_Log_Title");
		for (int j = 0; j < g_iKarmaLogCount_Prev[client]; j++)
		{
			char translatedReason[64];
			Format(translatedReason, sizeof(translatedReason), "%T", g_sKarmaLog_Prev[client][j], client);
			
			if (g_iKarmaLogPoints_Prev[client][j] > 0)
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Karma_Log_Positive", translatedReason, g_iKarmaLogPoints_Prev[client][j]);
			else
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Karma_Log_Negative", translatedReason, g_iKarmaLogPoints_Prev[client][j]);
		}
		TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Karma_Log_Footer");
	}
	
	return Plugin_Handled;
}

public Action Cmd_ViewKarma(int client, int args)
{
	if (args < 1)
	{
		ReplyToCommand(client, "%t", "Usage_ViewKarma");
		return Plugin_Handled;
	}
	
	char arg[65];
	GetCmdArg(1, arg, sizeof(arg));
	int target = FindTarget(client, arg, true, false);
	if (target <= 0) return Plugin_Handled;
	
	char rank[32];
	Karma_GetRankName(client, g_iKarma[target], rank, sizeof(rank));
	ReplyToCommand(client, "[TTT] %t", "Karma_View", target, g_iKarma[target], rank, g_iCleanRounds[target]);
	return Plugin_Handled;
}

public Action Cmd_SetKarma(int client, int args)
{
	if (args < 2)
	{
		ReplyToCommand(client, "%t", "Usage_SetKarma");
		return Plugin_Handled;
	}
	
	char arg1[65], arg2[32];
	GetCmdArg(1, arg1, sizeof(arg1));
	GetCmdArg(2, arg2, sizeof(arg2));
	
	int target = FindTarget(client, arg1, true, false);
	if (target <= 0) return Plugin_Handled;
	
	int val = StringToInt(arg2);
	g_iKarma[target] = val;
	if (g_iKarma[target] > 1000) g_iKarma[target] = 1000;
	if (g_iKarma[target] < 0) g_iKarma[target] = 0;
	
	Karma_SaveCookies(target);
	LogAction(client, target, "\"%L\" set karma of \"%L\" to %d", client, target, val);
	
	g_iActiveKarma[target] = g_iKarma[target];
	
	char rank[32];
	Karma_GetRankName(client, g_iKarma[target], rank, sizeof(rank));
	ReplyToCommand(client, "[TTT] %t", "Karma_Set", target, g_iKarma[target], rank);
	
	if (GetClientTeam(target) == 1 && g_iKarma[target] > 0)
	{
		ChangeClientTeam(target, 2);
	}
	
	return Plugin_Handled;
}

public Action Cmd_AddKarma(int client, int args)
{
	if (args < 2)
	{
		ReplyToCommand(client, "%t", "Usage_AddKarma");
		return Plugin_Handled;
	}
	
	char arg1[65], arg2[32];
	GetCmdArg(1, arg1, sizeof(arg1));
	GetCmdArg(2, arg2, sizeof(arg2));
	
	int target = FindTarget(client, arg1, true, false);
	if (target <= 0) return Plugin_Handled;
	
	int val = StringToInt(arg2);
	Karma_Modify(target, val, val < 0);
	
	LogAction(client, target, "\"%L\" added %d karma to \"%L\" (Total: %d)", client, val, target, g_iKarma[target]);
	
	g_iActiveKarma[target] = g_iKarma[target];
	
	char rank[32];
	Karma_GetRankName(client, g_iKarma[target], rank, sizeof(rank));
	ReplyToCommand(client, "[TTT] %t", "Karma_Add", target, val, g_iKarma[target], rank);
	
	if (GetClientTeam(target) == 1 && g_iKarma[target] > 0)
	{
		ChangeClientTeam(target, 2);
	}
	
	return Plugin_Handled;
}