// TTT Shop System

public Action Cmd_Shop(int client, int args)
{
	if (client == 0 || !IsClientInGame(client)) return Plugin_Handled;

	if (!cv_ttt_shop_enabled.BoolValue)
	{
		TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Disabled");
		return Plugin_Handled;
	}

	if (g_bRoundEnded)
	{
		TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Round_Ended");
		return Plugin_Handled;
	}
	
	if (!IsPlayerAlive(client) && g_bRoundLive)
	{
		TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Dead_Cannot_Shop");
		return Plugin_Handled;
	}

	if (g_iPlayerRole[client] == ROLE_NONE)
	{
		TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Wait_Roles");
		return Plugin_Handled;
	}

	ShowShopMenu(client);
	return Plugin_Handled;
}

void AddShopItem(Menu menu, int client, const char[] info, const char[] nameKey, int price)
{
	if (price < 0) return;
	char display[128];
	char translatedName[64];
	Format(translatedName, sizeof(translatedName), "%T", nameKey, client);
	Format(display, sizeof(display), "%T", "Shop_Item_Format", client, translatedName, price, price == 1 ? "" : "s");
	menu.AddItem(info, display);
}

void ShowShopMenu(int client)
{
	Menu menu = new Menu(MenuHandler_Shop);
	char title[128];
	Format(title, sizeof(title), "%T", "Shop_Menu_Title", client, g_iCredits[client]);
	menu.SetTitle(title);

	int role = g_iPlayerRole[client];

	if (role == ROLE_INNOCENT)
	{
		AddShopItem(menu, client, "kevlar", "Shop_Item_Kevlar", cv_ttt_shop_price_kevlar.IntValue);
		AddShopItem(menu, client, "ammo", "Shop_Item_Ammo", cv_ttt_shop_price_ammo.IntValue);
		AddShopItem(menu, client, "bat", "Shop_Item_Bat", cv_ttt_shop_price_bat.IntValue);
		AddShopItem(menu, client, "deagle", "Shop_Item_Deagle", cv_ttt_shop_price_deagle.IntValue);
		AddShopItem(menu, client, "pills", "Shop_Item_Pills", cv_ttt_shop_price_pills.IntValue);
		AddShopItem(menu, client, "taser", "Shop_Item_Taser", cv_ttt_shop_price_taser.IntValue);
		AddShopItem(menu, client, "traitor_next", "Shop_Item_TraitorNext", cv_ttt_shop_price_traitor_next.IntValue);
		AddShopItem(menu, client, "detective_now", "Shop_Item_DetectiveNow", cv_ttt_shop_price_detective_now.IntValue);
		AddShopItem(menu, client, "traitor_now", "Shop_Item_TraitorNow", cv_ttt_shop_price_traitor_now.IntValue);
	}
	else if (role == ROLE_TRAITOR)
	{
		AddShopItem(menu, client, "ammo", "Shop_Item_Ammo", cv_ttt_shop_price_ammo.IntValue);
		AddShopItem(menu, client, "kevlar_helmet", "Shop_Item_KevlarHelmet", cv_ttt_shop_price_kevlar_helmet.IntValue);
		AddShopItem(menu, client, "horde", "Shop_Item_Horde", cv_ttt_shop_price_horde.IntValue);
		AddShopItem(menu, client, "deagle", "Shop_Item_Deagle", cv_ttt_shop_price_deagle.IntValue);
		AddShopItem(menu, client, "pills", "Shop_Item_Pills", cv_ttt_shop_price_pills.IntValue);
		AddShopItem(menu, client, "wallhack", "Shop_Item_Wallhack", cv_ttt_shop_price_wallhack.IntValue);
		AddShopItem(menu, client, "gl", "Shop_Item_GL", cv_ttt_shop_price_gl.IntValue);
		AddShopItem(menu, client, "c4", "Shop_Item_C4", cv_ttt_shop_price_c4.IntValue);
		AddShopItem(menu, client, "missile", "Shop_Item_Missile", cv_ttt_shop_price_missile.IntValue);
		AddShopItem(menu, client, "special", "Shop_Item_SpecialInfected", cv_ttt_shop_price_special.IntValue);
		AddShopItem(menu, client, "tank", "Shop_Item_Tank", cv_ttt_shop_price_tank.IntValue);
	}
	else if (role == ROLE_DETECTIVE)
	{
		AddShopItem(menu, client, "ammo", "Shop_Item_Ammo", cv_ttt_shop_price_ammo.IntValue);
		AddShopItem(menu, client, "kevlar_helmet", "Shop_Item_KevlarHelmet", cv_ttt_shop_price_kevlar_helmet.IntValue);
		AddShopItem(menu, client, "defuser", "Shop_Item_Defuser", cv_ttt_shop_price_defuser.IntValue);
		AddShopItem(menu, client, "deagle", "Shop_Item_Deagle", cv_ttt_shop_price_deagle.IntValue);
		AddShopItem(menu, client, "identifier", "Shop_Item_Identifier", cv_ttt_shop_price_identifier.IntValue);
		AddShopItem(menu, client, "pills", "Shop_Item_Pills", cv_ttt_shop_price_pills.IntValue);
		AddShopItem(menu, client, "taser", "Shop_Item_Taser", cv_ttt_shop_price_taser.IntValue);
		AddShopItem(menu, client, "lootbox", "Shop_Item_Lootbox", cv_ttt_shop_price_lootbox.IntValue);
	}

	if (role == ROLE_INNOCENT || role == ROLE_TRAITOR || role == ROLE_DETECTIVE)
	{
		AddShopItem(menu, client, "immortality", "Shop_Item_Immortality", cv_ttt_shop_price_immortality.IntValue);
	}

	menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_Shop(Menu menu, MenuAction action, int param1, int param2)
{
	if (action == MenuAction_Select)
	{
		char info[32];
		menu.GetItem(param2, info, sizeof(info));
		ConfirmPurchaseMenu(param1, info);
	}
	else if (action == MenuAction_End)
	{
		delete menu;
	}
	return 0;
}

void ConfirmPurchaseMenu(int client, const char[] item)
{
	Menu menu = new Menu(MenuHandler_Confirm);
	int cost = 0;
	char title[128];
	
	if (StrEqual(item, "kevlar"))
	{
		cost = cv_ttt_shop_price_kevlar.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Kevlar", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "pills"))
	{
		cost = cv_ttt_shop_price_pills.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Pills", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "gl"))
	{
		cost = cv_ttt_shop_price_gl.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_GL", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "wallhack"))
	{
		cost = cv_ttt_shop_price_wallhack.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Wallhack", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "c4"))
	{
		cost = cv_ttt_shop_price_c4.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_C4", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "missile"))
	{
		cost = cv_ttt_shop_price_missile.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Missile", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "taser"))
	{
		cost = cv_ttt_shop_price_taser.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Taser", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "defuser"))
	{
		cost = cv_ttt_shop_price_defuser.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Defuser", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "traitor_next"))
	{
		cost = cv_ttt_shop_price_traitor_next.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_TraitorNext", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "traitor_now"))
	{
		cost = cv_ttt_shop_price_traitor_now.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_TraitorNow", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "detective_now"))
	{
		cost = cv_ttt_shop_price_detective_now.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_DetectiveNow", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "immortality"))
	{
		cost = cv_ttt_shop_price_immortality.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Immortality", client, cost, cost == 1 ? "" : "s", cv_ttt_shop_immortality_duration.FloatValue);
	}
	else if (StrEqual(item, "deagle"))
	{
		cost = cv_ttt_shop_price_deagle.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Deagle", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "bat"))
	{
		cost = cv_ttt_shop_price_bat.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Bat", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "horde"))
	{
		cost = cv_ttt_shop_price_horde.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Horde", client, cost, cost == 1 ? "" : "s", cv_ttt_shop_horde_cooldown.FloatValue);
	}
	else if (StrEqual(item, "special"))
	{
		cost = cv_ttt_shop_price_special.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_SpecialInfected", client, cost, cost == 1 ? "" : "s", cv_ttt_shop_infected_cooldown.FloatValue);
	}
	else if (StrEqual(item, "kevlar_helmet"))
	{
		cost = cv_ttt_shop_price_kevlar_helmet.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_KevlarHelmet", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "identifier"))
	{
		cost = cv_ttt_shop_price_identifier.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Identifier", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "ammo"))
	{
		cost = cv_ttt_shop_price_ammo.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Ammo", client, cost, cost == 1 ? "" : "s");
	}
	else if (StrEqual(item, "tank"))
	{
		cost = cv_ttt_shop_price_tank.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Tank", client, cost, cost == 1 ? "" : "s", cv_ttt_shop_tank_cooldown.FloatValue);
	}
	else if (StrEqual(item, "lootbox"))
	{
		cost = cv_ttt_shop_price_lootbox.IntValue;
		Format(title, sizeof(title), "%T", "Shop_Desc_Lootbox", client, cost, cost == 1 ? "" : "s");
	}

	menu.SetTitle(title);
	
	char btnBuy[32], btnBack[32];
	Format(btnBuy, sizeof(btnBuy), "%T", "Shop_Buy", client);
	Format(btnBack, sizeof(btnBack), "%T", "Shop_Back", client);
	
	menu.AddItem(item, btnBuy);
	menu.AddItem("back", btnBack);
	menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_Confirm(Menu menu, MenuAction action, int param1, int param2)
{
	if (action == MenuAction_Select)
	{
		int client = param1;
		char info[32];
		menu.GetItem(param2, info, sizeof(info));
		
		if (StrEqual(info, "back"))
		{
			ShowShopMenu(client);
			return 0;
		}

		if (!g_bRoundLive && !g_bRoundEnded)
		{
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Round_Not_Active");
			return 0;
		}

		if (g_bRoundEnded)
		{
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Round_Ended");
			return 0;
		}
		
		if (!IsPlayerAlive(client) && g_bRoundLive)
		{
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Dead_Cannot_Shop");
			return 0;
		}
		
		if (cv_ttt_karma_enabled.BoolValue)
		{
			if (g_iActiveKarma[client] < 400) // Hostile
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Hostile");
				return 0;
			}
			
			if (g_iActiveKarma[client] < 550) // Violent
			{
				if (StrEqual(info, "traitor_next") || StrEqual(info, "traitor_now") || StrEqual(info, "detective_now"))
				{
					TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_ViolentRole");
					return 0;
				}
			}
		}
		
		int cost = 0;
		if (StrEqual(info, "kevlar")) cost = cv_ttt_shop_price_kevlar.IntValue;
		else if (StrEqual(info, "kevlar_helmet")) cost = cv_ttt_shop_price_kevlar_helmet.IntValue;
		else if (StrEqual(info, "defuser")) cost = cv_ttt_shop_price_defuser.IntValue;
		else if (StrEqual(info, "identifier")) cost = cv_ttt_shop_price_identifier.IntValue;
		else if (StrEqual(info, "pills")) cost = cv_ttt_shop_price_pills.IntValue;
		else if (StrEqual(info, "wallhack")) cost = cv_ttt_shop_price_wallhack.IntValue;
		else if (StrEqual(info, "taser")) cost = cv_ttt_shop_price_taser.IntValue;
		else if (StrEqual(info, "traitor_next")) cost = cv_ttt_shop_price_traitor_next.IntValue;
		else if (StrEqual(info, "gl")) cost = cv_ttt_shop_price_gl.IntValue;
		else if (StrEqual(info, "c4")) cost = cv_ttt_shop_price_c4.IntValue;
		else if (StrEqual(info, "detective_now")) cost = cv_ttt_shop_price_detective_now.IntValue;
		else if (StrEqual(info, "missile")) cost = cv_ttt_shop_price_missile.IntValue;
		else if (StrEqual(info, "traitor_now")) cost = cv_ttt_shop_price_traitor_now.IntValue;
		else if (StrEqual(info, "immortality")) cost = cv_ttt_shop_price_immortality.IntValue;
		else if (StrEqual(info, "bat")) cost = cv_ttt_shop_price_bat.IntValue;
		else if (StrEqual(info, "deagle")) cost = cv_ttt_shop_price_deagle.IntValue;
		else if (StrEqual(info, "horde")) cost = cv_ttt_shop_price_horde.IntValue;
		else if (StrEqual(info, "special")) cost = cv_ttt_shop_price_special.IntValue;
		else if (StrEqual(info, "ammo")) cost = cv_ttt_shop_price_ammo.IntValue;
		else if (StrEqual(info, "tank")) cost = cv_ttt_shop_price_tank.IntValue;
		else if (StrEqual(info, "lootbox")) cost = cv_ttt_shop_price_lootbox.IntValue;

		if (g_iCredits[client] < cost)
		{
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_NoCredits");
			return 0;
		}

		bool success = false;

		if (StrEqual(info, "kevlar"))
		{
			if (g_bHasKevlar[client]) TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Kevlar");
			else { 
				g_bHasKevlar[client] = true; 
				success = true; 
				char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_Kevlar", client);
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName); 
			}
		}
		else if (StrEqual(info, "kevlar_helmet"))
		{
			if (g_bHasKevlar[client] && g_bHasHelmet[client]) TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_KevlarHelmet");
			else { 
				g_bHasKevlar[client] = true; 
				g_bHasHelmet[client] = true; 
				success = true; 
				char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_KevlarHelmet", client);
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName); 
			}
		}
		else if (StrEqual(info, "pills"))
		{
			int slot4 = GetPlayerWeaponSlot(client, 4);
			if (slot4 != -1) TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Pills_SlotFull");
			else
			{
				int ent = CreateEntityByName("weapon_pain_pills");
				if (ent != -1) { DispatchSpawn(ent); EquipPlayerWeapon(client, ent); }
				success = true; 
				char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_Pills", client);
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName);
			}
		}
		else if (StrEqual(info, "gl"))
		{
			int slot0 = GetPlayerWeaponSlot(client, 0);
			if (slot0 != -1)
			{
				char cls[64];
				GetEntityClassname(slot0, cls, sizeof(cls));
				if (StrEqual(cls, "weapon_grenade_launcher"))
				{
					TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_GL");
					return 0;
				}
				RemovePlayerItem(client, slot0);
				AcceptEntityInput(slot0, "Kill");
			}
			
			int wep = CreateEntityByName("weapon_grenade_launcher");
			if (wep != -1) 
			{ 
				DispatchSpawn(wep); 
				EquipPlayerWeapon(client, wep); 
				SetEntProp(wep, Prop_Send, "m_iClip1", 1);
				
				int ammoOffset = FindDataMapInfo(client, "m_iAmmo");
				SetEntData(client, ammoOffset + (17 * 4), 0);
			}
			success = true; 
			char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_GL", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName);
		}
		else if (StrEqual(info, "wallhack"))
		{
			if (g_fWallhackEndTime[client] > GetEngineTime())
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Wallhack");
			}
			else
			{
				GiveWallhackToClient(client);
				success = true;
				char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_Wallhack", client);
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName);
			}
		}
		else if (StrEqual(info, "defuser"))
		{
			if (g_bHasDefuser[client])
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Defuser");
				return 0;
			}
			g_bHasDefuser[client] = true;
			success = true;
			char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_Defuser", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName);
		}
		else if (StrEqual(info, "c4"))
		{
			if (g_bBoughtC4[client])
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_C4");
				return 0;
			}
			
			int slot3 = GetPlayerWeaponSlot(client, 3);
			if (slot3 != -1)
			{
				RemovePlayerItem(client, slot3);
				AcceptEntityInput(slot3, "Kill");
			}
			
			int wep = CreateEntityByName("weapon_upgradepack_incendiary");
			if (wep != -1) 
			{ 
				DispatchSpawn(wep); 
				EquipPlayerWeapon(client, wep); 
				g_iC4PackEnt[client] = EntIndexToEntRef(wep);
			}
			g_bBoughtC4[client] = true;
			success = true; 
			char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_C4", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName);
		}
		else if (StrEqual(info, "missile"))
		{
			if (g_bBoughtMissile[client])
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Missile");
				return 0;
			}
			
			int slot2 = GetPlayerWeaponSlot(client, 2);
			if (slot2 != -1)
			{
				char cls[64];
				GetEntityClassname(slot2, cls, sizeof(cls));
				if (StrEqual(cls, "weapon_pipe_bomb"))
				{
					g_bBoughtMissile[client] = true;
					success = true;
					char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_Missile", client);
					TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName);
					PrintToServer("%t", "Shop_Bought_Missile_Server", client);
					return 0;
				}
				RemovePlayerItem(client, slot2);
				AcceptEntityInput(slot2, "Kill");
			}
			
			int wep = CreateEntityByName("weapon_pipe_bomb");
			if (wep != -1) 
			{ 
				DispatchSpawn(wep); 
				EquipPlayerWeapon(client, wep); 
			}
			g_bBoughtMissile[client] = true;
			success = true; 
			char itemName[64]; Format(itemName, sizeof(itemName), "%T", "Shop_Item_Missile", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemName);
			PrintToServer("%t", "Shop_Bought_Missile_Server", client);
		}
		else if (StrEqual(info, "taser"))
		{
			int slot1 = GetPlayerWeaponSlot(client, 1);
			if (slot1 != -1)
			{
				char cls[64];
				GetEntityClassname(slot1, cls, sizeof(cls));
				if (StrEqual(cls, "weapon_melee"))
				{
					char itemName[64];
					GetEntPropString(slot1, Prop_Data, "m_strMapSetScriptName", itemName, sizeof(itemName));
					if (StrEqual(itemName, "tonfa"))
					{
						TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Taser");
						return 0;
					}
				}
				RemovePlayerItem(client, slot1);
				AcceptEntityInput(slot1, "Kill");
			}
			
			int wep = CreateEntityByName("weapon_melee");
			if (wep != -1) { DispatchKeyValue(wep, "melee_script_name", "tonfa"); DispatchSpawn(wep); EquipPlayerWeapon(client, wep); }
			success = true; 
			char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_Taser", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN);
		}
		else if (StrEqual(info, "identifier"))
		{
			if (g_iCorpseIdentifiers[client] > 0) TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Identifier");
			else { 
				g_iCorpseIdentifiers[client] = 1; 
				success = true; 
				char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_Identifier", client);
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN); 
			}
		}
		else if (StrEqual(info, "traitor_next"))
		{
			if (g_bWantsDetective[client])
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_NextRoundTraitor_Detective");
			}
			else if (g_bNextRoundTraitor[client]) 
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_NextRoundTraitor_Already");
			}
			else 
			{ 
				g_bNextRoundTraitor[client] = true; 
				success = true; 
				char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_TraitorNext", client);
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN); 
			}
		}
		else if (StrEqual(info, "traitor_now"))
		{
			if (g_iPlayerRole[client] == ROLE_TRAITOR) TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Traitor");
			else
			{
				if (g_iSpriteEntity[client] > 0 && IsValidEntity(g_iSpriteEntity[client])) { AcceptEntityInput(g_iSpriteEntity[client], "Kill"); g_iSpriteEntity[client] = 0; }
				g_iPlayerRole[client] = ROLE_TRAITOR;
				CreateRoleSprite(client, ROLE_TRAITOR);
				ScheduleModelChange(client, g_sModelPaths[6], 6);
				SetScreenOverlay(client, "sprites/Hud/Thud");
				success = true; 
				char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_TraitorNow", client);
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN);
			}
		}
		else if (StrEqual(info, "detective_now"))
		{
			if (g_iPlayerRole[client] == ROLE_DETECTIVE) TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Detective");
			else
			{
				if (g_iSpriteEntity[client] > 0 && IsValidEntity(g_iSpriteEntity[client])) { AcceptEntityInput(g_iSpriteEntity[client], "Kill"); g_iSpriteEntity[client] = 0; }
				g_iPlayerRole[client] = ROLE_DETECTIVE;
				CreateRoleSprite(client, ROLE_DETECTIVE);
				ScheduleModelChange(client, g_sModelPaths[4], 4);
				SetScreenOverlay(client, "sprites/Hud/Dhud");
				
				g_bHasKevlar[client] = true;
				g_bHasHelmet[client] = true;
				g_iCorpseIdentifiers[client] = 1;
				
				int slot1 = GetPlayerWeaponSlot(client, 1);
				if (slot1 != -1) { RemovePlayerItem(client, slot1); AcceptEntityInput(slot1, "Kill"); }
				int wep = CreateEntityByName("weapon_melee");
				if (wep != -1) { DispatchKeyValue(wep, "melee_script_name", "tonfa"); DispatchSpawn(wep); EquipPlayerWeapon(client, wep); }
				
				int slot4 = GetPlayerWeaponSlot(client, 4);
				if (slot4 != -1) { RemovePlayerItem(client, slot4); AcceptEntityInput(slot4, "Kill"); }
				int pill = CreateEntityByName("weapon_pain_pills");
				if (pill != -1) { DispatchSpawn(pill); EquipPlayerWeapon(client, pill); }
				
				success = true; 
				char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_DetectiveNow", client);
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN);
			}
		}
		else if (StrEqual(info, "deagle"))
		{
			int slot1 = GetPlayerWeaponSlot(client, 1);
			if (slot1 != -1)
			{
				char cls[64];
				GetEntityClassname(slot1, cls, sizeof(cls));
				if (StrEqual(cls, "weapon_pistol_magnum"))
				{
					TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Deagle");
					return 0;
				}
				RemovePlayerItem(client, slot1);
				AcceptEntityInput(slot1, "Kill");
			}
			
			int wep = CreateEntityByName("weapon_pistol_magnum");
			if (wep != -1) 
			{ 
				DispatchSpawn(wep); 
				EquipPlayerWeapon(client, wep); 
			}
			success = true; 
			char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_Deagle", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN);
		}
		else if (StrEqual(info, "immortality"))
		{
			if (g_fImmortalityEndTime[client] > GetEngineTime())
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Immortality");
				return 0;
			}
			GiveImmortality(client);
			success = true;
			char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_Immortality", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN);
		}
		else if (StrEqual(info, "bat"))
		{
			int slot1 = GetPlayerWeaponSlot(client, 1);
			if (slot1 != -1)
			{
				char cls[64];
				GetEntityClassname(slot1, cls, sizeof(cls));
				if (StrEqual(cls, "weapon_melee"))
				{
					char itemName[64];
					GetEntPropString(slot1, Prop_Data, "m_strMapSetScriptName", itemName, sizeof(itemName));
					if (StrEqual(itemName, "baseball_bat"))
					{
						TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Already_Bat");
						return 0;
					}
				}
				RemovePlayerItem(client, slot1);
				AcceptEntityInput(slot1, "Kill");
			}
			
			int wep = CreateEntityByName("weapon_melee");
			if (wep != -1) { DispatchKeyValue(wep, "melee_script_name", "baseball_bat"); DispatchSpawn(wep); EquipPlayerWeapon(client, wep); }
			success = true; 
			char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_Bat", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN);
		}
		else if (StrEqual(info, "horde"))
		{
			if (g_fHordeCooldownEndTime > GetEngineTime())
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Cooldown_Horde", g_fHordeCooldownEndTime - GetEngineTime());
				return 0;
			}
			
			g_fHordeCooldownEndTime = GetEngineTime() + cv_ttt_shop_horde_cooldown.FloatValue;
			InvokeHorde(); 
			
			success = true; 
			TTT_PrintToChatAll("\x05[TTT]\x01 %t", "Shop_Broadcast_Horde");
		}
		else if (StrEqual(info, "special"))
		{
			if (g_fSpecialCooldownEndTime > GetEngineTime())
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Cooldown_Infected", g_fSpecialCooldownEndTime - GetEngineTime());
				return 0;
			}
			
			g_fSpecialCooldownEndTime = GetEngineTime() + cv_ttt_shop_infected_cooldown.FloatValue;
			SpawnSpecialAtCrosshair(client); 
			
			success = true; 
		}
		else if (StrEqual(info, "ammo"))
		{
			if (GiveFullAmmo(client))
			{
				success = true;
			}
			else
			{
				return 0;
			}
		}
		else if (StrEqual(info, "tank"))
		{
			if (g_fTankCooldownEndTime[client] > GetEngineTime())
			{
				TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Cooldown_Tank", g_fTankCooldownEndTime[client] - GetEngineTime());
				return 0;
			}
			
			if (SpawnTankAtCrosshair(client))
			{
				g_fTankCooldownEndTime[client] = GetEngineTime() + cv_ttt_shop_tank_cooldown.FloatValue;
				success = true;
			}
			else
			{
				return 0;
			}
		}
		else if (StrEqual(info, "lootbox"))
		{
			float eyePos[3], eyeAng[3], fwd[3], spawnPos[3], endPos[3];
			GetClientEyePosition(client, eyePos);
			GetClientEyeAngles(client, eyeAng);
			
			eyeAng[0] = 0.0; 
			GetAngleVectors(eyeAng, fwd, NULL_VECTOR, NULL_VECTOR);
			
			endPos[0] = eyePos[0] + fwd[0] * 100.0;
			endPos[1] = eyePos[1] + fwd[1] * 100.0;
			endPos[2] = eyePos[2]; 
			
			Handle trace = TR_TraceRayFilterEx(eyePos, endPos, MASK_SOLID, RayType_EndPoint, TraceFilter_NoPlayers, client);
			if (TR_DidHit(trace))
			{
				TR_GetEndPosition(spawnPos, trace);
				spawnPos[0] -= fwd[0] * 25.0;
				spawnPos[1] -= fwd[1] * 25.0;
				spawnPos[2] = eyePos[2];
			}
			else
			{
				spawnPos = endPos;
			}
			delete trace;
			
			SpawnLootboxAtPosition(spawnPos);
			success = true;
			char itemN[64]; Format(itemN, sizeof(itemN), "%T", "Shop_Item_Lootbox", client);
			TTT_PrintToChat(client, "\x05[TTT]\x01 %t", "Shop_Bought", itemN);
		}

		if (success)
		{
			g_iCredits[client] -= cost;
			SaveCredits(client);
		}
	}
	else if (action == MenuAction_End)
	{
		delete menu;
	}
	return 0;
}