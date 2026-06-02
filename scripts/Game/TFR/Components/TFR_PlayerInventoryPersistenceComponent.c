//------------------------------------------------------------------------------------------------
// TFR_PlayerInventoryPersistenceComponent.c
// TFR OPERATIONS / TFR FRAMEWORK
//
// V0.4.1 PLAYER INVENTORY PERSISTENCE
//
// VA EN GAMEMODE.
//
// OBJETIVO:
// - Escanear inventario de jugadores conectados.
// - Guardar player_inventory.json en $profile:TFR_Operations.
// - Deduplicar entidades repetidas.
// - Extraer prefab real aunque SCR_EditableEntityComponent.GetPrefab(false) devuelva vacío.
// - Restaurar de forma controlada SOLO items de INVENTORY_STORAGE_SLOT.
// - NO reemplazar ropa/equipo/loadout.
// - NO tocar EQUIPMENT_STORAGE_SLOT.
// - Evitar duplicados comparando cantidad existente vs cantidad guardada.
// - NO sobrescribir un save bueno con Players=0 cuando el jugador aún no tiene controlled entity.
// - Reintentar restore si arranca antes de que el jugador tenga entidad controlada.
// - Bloquear autosave mientras restore inicial está pendiente.
// - V0.4.0: opción segura para limpiar INVENTORY_STORAGE_SLOT antes de restaurar.
// - V0.4.1: si no existe player_inventory.json, desbloquea autosave y crea save nuevo.
//
// Requiere:
// - SCR_InventoryStorageManagerComponent en el personaje.
// - PlayerManager disponible.
//
// SERVER POLISH:
// - Debug Logs apagado por defecto.
// - No cambia lógica de guardado.
// - No cambia lógica de restauración.
// - No cambia JSON.
// - No cambia autoscan.
// - Los logs ADMIN se mantienen visibles para diagnóstico operativo.
//------------------------------------------------------------------------------------------------

class TFR_PlayerInventoryPersistenceComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class TFR_PlayerInventoryPersistenceComponent : ScriptComponent
{
	protected static TFR_PlayerInventoryPersistenceComponent s_ActivePlayerInventoryPersistence;

	// -------------------------------------------------------------------------
	// GENERAL
	// -------------------------------------------------------------------------

	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("true", UIWidgets.CheckBox, "Run Server Only")]
	protected bool m_bRunServerOnly;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("true", UIWidgets.CheckBox, "Auto Scan Enabled")]
	protected bool m_bAutoScanEnabled;

	[Attribute("10000", UIWidgets.EditBox, "Initial Scan Delay MS")]
	protected int m_iInitialScanDelayMs;

	[Attribute("30", UIWidgets.EditBox, "Auto Scan Interval Seconds")]
	protected int m_iAutoScanIntervalSeconds;

	[Attribute("true", UIWidgets.CheckBox, "Save Empty Inventories")]
	protected bool m_bSaveEmptyInventories;

	[Attribute("true", UIWidgets.CheckBox, "Skip Save When No Valid Players")]
	protected bool m_bSkipSaveWhenNoValidPlayers;

	[Attribute("true", UIWidgets.CheckBox, "Include All Items")]
	protected bool m_bIncludeAllItems;

	[Attribute("true", UIWidgets.CheckBox, "Include Root Like Items")]
	protected bool m_bIncludeRootItems;

	[Attribute("true", UIWidgets.CheckBox, "Include Item Debug Strings")]
	protected bool m_bIncludeItemDebugStrings;

	[Attribute("true", UIWidgets.CheckBox, "Skip Duplicate Entities")]
	protected bool m_bSkipDuplicateEntities;

	// -------------------------------------------------------------------------
	// RESTORE
	// -------------------------------------------------------------------------

	[Attribute("true", UIWidgets.CheckBox, "Restore On Start Enabled")]
	protected bool m_bRestoreOnStartEnabled;

	[Attribute("18000", UIWidgets.EditBox, "Restore On Start Delay MS")]
	protected int m_iRestoreOnStartDelayMs;

	[Attribute("true", UIWidgets.CheckBox, "Restore Retry Enabled")]
	protected bool m_bRestoreRetryEnabled;

	[Attribute("20", UIWidgets.EditBox, "Restore Retry Count")]
	protected int m_iRestoreRetryCount;

	[Attribute("10000", UIWidgets.EditBox, "Restore Retry Delay MS")]
	protected int m_iRestoreRetryDelayMs;

	[Attribute("true", UIWidgets.CheckBox, "Restore Blocks Save Until Completed")]
	protected bool m_bRestoreBlocksSaveUntilCompleted;

	[Attribute("15000", UIWidgets.EditBox, "Post Restore Save Unlock Delay MS")]
	protected int m_iPostRestoreSaveUnlockDelayMs;

	[Attribute("true", UIWidgets.CheckBox, "Restore Inventory Storage Slot Items")]
	protected bool m_bRestoreInventoryStorageSlotItems;

	[Attribute("false", UIWidgets.CheckBox, "Restore Loadout Slot Items Future Unsafe")]
	protected bool m_bRestoreLoadoutSlotItems;

	[Attribute("false", UIWidgets.CheckBox, "Restore Equipment Storage Slot Items Future Unsafe")]
	protected bool m_bRestoreEquipmentStorageSlotItems;

	[Attribute("true", UIWidgets.CheckBox, "Avoid Duplicate Prefab Counts")]
	protected bool m_bAvoidDuplicatePrefabCounts;

	[Attribute("80", UIWidgets.EditBox, "Max Restore Items Per Player")]
	protected int m_iMaxRestoreItemsPerPlayer;

	[Attribute("true", UIWidgets.CheckBox, "Restore Match Player Name")]
	protected bool m_bRestoreMatchPlayerName;

	[Attribute("true", UIWidgets.CheckBox, "Restore Match Player Id Fallback")]
	protected bool m_bRestoreMatchPlayerIdFallback;

	[Attribute("true", UIWidgets.CheckBox, "Clean Inventory Storage Before Restore")]
	protected bool m_bCleanInventoryStorageBeforeRestore;

	// -------------------------------------------------------------------------
	// FILES
	// -------------------------------------------------------------------------

	[Attribute("$profile:TFR_Operations", UIWidgets.EditBox, "Save Directory")]
	protected string m_sSaveDirectory;

	[Attribute("player_inventory.json", UIWidgets.EditBox, "Player Inventory Save File")]
	protected string m_sPlayerInventorySaveFile;

	// -------------------------------------------------------------------------
	// RUNTIME
	// -------------------------------------------------------------------------

	protected IEntity m_Owner;
	protected int m_iLastSavedPlayerCount;
	protected int m_iLastSavedItemCount;
	protected int m_iLastRawItemCount;
	protected int m_iLastSkippedSaveCount;
	protected int m_iLastRestoreBlockedSaveCount;

	protected int m_iLastRestorePlayerCount;
	protected int m_iLastRestoreAttemptCount;
	protected int m_iLastRestoreSuccessCount;
	protected int m_iLastRestoreSkippedCount;
	protected int m_iLastRestoreFailedCount;

	protected int m_iLastCleanRemovedCount;
	protected int m_iLastCleanFailedCount;

	protected int m_iRestoreRetryRemaining;
	protected bool m_bRestoreCompletedOnce;
	protected bool m_bSaveBlockedByRestore;

	//------------------------------------------------------------------------------------------------
	void TFR_PlayerInventoryPersistenceComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iLastSavedPlayerCount = 0;
		m_iLastSavedItemCount = 0;
		m_iLastRawItemCount = 0;
		m_iLastSkippedSaveCount = 0;
		m_iLastRestoreBlockedSaveCount = 0;

		m_iLastRestorePlayerCount = 0;
		m_iLastRestoreAttemptCount = 0;
		m_iLastRestoreSuccessCount = 0;
		m_iLastRestoreSkippedCount = 0;
		m_iLastRestoreFailedCount = 0;

		m_iLastCleanRemovedCount = 0;
		m_iLastCleanFailedCount = 0;

		m_iRestoreRetryRemaining = 0;
		m_bRestoreCompletedOnce = false;
		m_bSaveBlockedByRestore = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_Owner = owner;

		if (!CanRunOnThisMachine())
			return;

		s_ActivePlayerInventoryPersistence = this;

		ValidateSettings();
		EnsureSaveDirectory();

		LogTFRAdmin("Player inventory persistence component initialized.");
		LogTFR("Owner=" + owner.ToString());
		LogTFR("SavePath=" + GetPlayerInventorySavePath());
		LogTFR("AutoScanEnabled=" + m_bAutoScanEnabled.ToString());
		LogTFR("InitialScanDelayMs=" + m_iInitialScanDelayMs.ToString());
		LogTFR("AutoScanIntervalSeconds=" + m_iAutoScanIntervalSeconds.ToString());
		LogTFR("SkipSaveWhenNoValidPlayers=" + m_bSkipSaveWhenNoValidPlayers.ToString());
		LogTFR("SkipDuplicateEntities=" + m_bSkipDuplicateEntities.ToString());
		LogTFR("RestoreOnStartEnabled=" + m_bRestoreOnStartEnabled.ToString());
		LogTFR("RestoreOnStartDelayMs=" + m_iRestoreOnStartDelayMs.ToString());
		LogTFR("RestoreRetryEnabled=" + m_bRestoreRetryEnabled.ToString());
		LogTFR("RestoreRetryCount=" + m_iRestoreRetryCount.ToString());
		LogTFR("RestoreRetryDelayMs=" + m_iRestoreRetryDelayMs.ToString());
		LogTFR("RestoreBlocksSaveUntilCompleted=" + m_bRestoreBlocksSaveUntilCompleted.ToString());
		LogTFR("PostRestoreSaveUnlockDelayMs=" + m_iPostRestoreSaveUnlockDelayMs.ToString());
		LogTFR("CleanInventoryStorageBeforeRestore=" + m_bCleanInventoryStorageBeforeRestore.ToString());

		if (m_bRestoreOnStartEnabled)
		{
			m_iRestoreRetryRemaining = m_iRestoreRetryCount;

			if (m_bRestoreBlocksSaveUntilCompleted)
			{
				m_bSaveBlockedByRestore = true;
				LogTFRAdmin("Player inventory autosave blocked until initial restore finishes.");
			}

			GetGame().GetCallqueue().Remove(RestoreAllPlayerInventoriesNow);
			GetGame().GetCallqueue().CallLater(RestoreAllPlayerInventoriesNow, m_iRestoreOnStartDelayMs, false);

			LogTFRAdmin("Player inventory restore scheduled. DelayMs=" + m_iRestoreOnStartDelayMs.ToString());
		}

		if (m_bAutoScanEnabled)
		{
			if (m_iInitialScanDelayMs > 0)
				GetGame().GetCallqueue().CallLater(SaveAllPlayerInventoriesNow, m_iInitialScanDelayMs, false);
			else
				SaveAllPlayerInventoriesNow();

			StartAutoScanLoop();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(SaveAllPlayerInventoriesNow);
			GetGame().GetCallqueue().Remove(RestoreAllPlayerInventoriesNow);
			GetGame().GetCallqueue().Remove(AutoScanTick);
			GetGame().GetCallqueue().Remove(UnlockSaveAfterRestore);
		}

		if (s_ActivePlayerInventoryPersistence == this)
			s_ActivePlayerInventoryPersistence = null;

		LogTFR("Player inventory persistence component deleted.");

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static TFR_PlayerInventoryPersistenceComponent GetActivePlayerInventoryPersistence()
	{
		return s_ActivePlayerInventoryPersistence;
	}

	// -------------------------------------------------------------------------
	// VALIDATION / SERVER
	// -------------------------------------------------------------------------

	protected void ValidateSettings()
	{
		if (m_iInitialScanDelayMs < 0)
			m_iInitialScanDelayMs = 0;

		if (m_iAutoScanIntervalSeconds <= 0)
			m_iAutoScanIntervalSeconds = 60;

		if (m_iAutoScanIntervalSeconds < 15)
			m_iAutoScanIntervalSeconds = 15;

		if (m_iRestoreOnStartDelayMs < 0)
			m_iRestoreOnStartDelayMs = 0;

		if (m_iRestoreRetryCount < 0)
			m_iRestoreRetryCount = 0;

		if (m_iRestoreRetryCount > 60)
			m_iRestoreRetryCount = 60;

		if (m_iRestoreRetryDelayMs <= 0)
			m_iRestoreRetryDelayMs = 10000;

		if (m_iRestoreRetryDelayMs < 2000)
			m_iRestoreRetryDelayMs = 2000;

		if (m_iPostRestoreSaveUnlockDelayMs < 0)
			m_iPostRestoreSaveUnlockDelayMs = 0;

		if (m_iMaxRestoreItemsPerPlayer <= 0)
			m_iMaxRestoreItemsPerPlayer = 80;

		if (m_iMaxRestoreItemsPerPlayer > 300)
			m_iMaxRestoreItemsPerPlayer = 300;

		if (m_sSaveDirectory == string.Empty)
			m_sSaveDirectory = "$profile:TFR_Operations";

		if (m_sPlayerInventorySaveFile == string.Empty)
			m_sPlayerInventorySaveFile = "player_inventory.json";
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanRunOnThisMachine()
	{
		if (!m_bEnabled)
			return false;

		if (!GetGame())
			return false;

		if (!m_bRunServerOnly)
			return true;

		if (!Replication.IsRunning())
			return true;

		return Replication.IsServer();
	}

	// -------------------------------------------------------------------------
	// AUTO SCAN
	// -------------------------------------------------------------------------

	protected void StartAutoScanLoop()
	{
		if (!CanRunOnThisMachine())
			return;

		if (!GetGame())
			return;

		int intervalMs = Math.Max(m_iAutoScanIntervalSeconds, 15) * 1000;

		GetGame().GetCallqueue().Remove(AutoScanTick);
		GetGame().GetCallqueue().CallLater(AutoScanTick, intervalMs, true);

		LogTFRAdmin("Player inventory autoscan started. IntervalSeconds=" + m_iAutoScanIntervalSeconds.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void AutoScanTick()
	{
		if (!CanRunOnThisMachine())
			return;

		SaveAllPlayerInventoriesNow();
	}

	// -------------------------------------------------------------------------
	// PUBLIC API SAVE
	// -------------------------------------------------------------------------

	void SaveAllPlayerInventoriesNow()
	{
		if (!CanRunOnThisMachine())
			return;

		string savePath = GetPlayerInventorySavePath();

		if (m_bSaveBlockedByRestore)
		{
			m_iLastRestoreBlockedSaveCount++;

			bool existingSaveBlocked = FileIO.FileExists(savePath);
			LogTFRAdmin("Player inventory save skipped: initial restore pending. Existing save preserved=" + existingSaveBlocked.ToString() + " Path=" + savePath);
			return;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
		{
			LogTFRError("SaveAllPlayerInventoriesNow failed: PlayerManager not found.");
			return;
		}

		ref array<int> players = new array<int>();
		playerManager.GetPlayers(players);

		EnsureSaveDirectory();

		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext(false);
		ctx.SetMaxDecimalPlaces(3);

		ctx.WriteValue("version", 41);
		ctx.WriteValue("scannerOnly", false);
		ctx.WriteValue("restoreMode", "CLEAN_AND_RESTORE_INVENTORY_STORAGE_SLOT_ONLY");
		ctx.WriteValue("note", "TFR V0.4.1 inventory persistence with bootstrap save creation when no JSON exists.");
		ctx.WriteValue("playerCount", players.Count());

		int savedPlayers = 0;
		int totalItems = 0;
		int totalRawItems = 0;

		for (int i = 0; i < players.Count(); i++)
		{
			int playerId = players[i];

			int rawCount = 0;
			int savedItems = SaveSinglePlayerInventoryToContext(ctx, playerManager, playerId, savedPlayers, rawCount);

			if (savedItems < 0)
				continue;

			savedPlayers++;
			totalItems += savedItems;
			totalRawItems += rawCount;
		}

		if (m_bSkipSaveWhenNoValidPlayers && savedPlayers <= 0)
		{
			m_iLastSkippedSaveCount++;

			bool existingSave = FileIO.FileExists(savePath);

			LogTFRAdmin("Player inventory save skipped: no valid controlled players. Existing save preserved=" + existingSave.ToString() + " Path=" + savePath);
			return;
		}

		ctx.WriteValue("savedPlayerCount", savedPlayers);
		ctx.WriteValue("totalRawItemCount", totalRawItems);
		ctx.WriteValue("totalUniqueItemCount", totalItems);
		ctx.WriteValue("totalItemCount", totalItems);

		bool saved = ctx.SaveToFile(savePath);

		if (!saved)
		{
			LogTFRError("SaveAllPlayerInventoriesNow failed: SaveToFile returned false. Path=" + savePath);
			return;
		}

		m_iLastSavedPlayerCount = savedPlayers;
		m_iLastSavedItemCount = totalItems;
		m_iLastRawItemCount = totalRawItems;

		LogTFRAdmin("Player inventories saved. Players=" + savedPlayers.ToString() + " UniqueItems=" + totalItems.ToString() + " RawItems=" + totalRawItems.ToString());
		LogTFR("Saved path=" + savePath);
	}

	// -------------------------------------------------------------------------
	// PUBLIC API RESTORE
	// -------------------------------------------------------------------------

	void RestoreAllPlayerInventoriesNow()
	{
		if (!CanRunOnThisMachine())
			return;

		if (m_bRestoreCompletedOnce)
		{
			LogTFR("Restore ignored: restore already completed once this session.");
			return;
		}

		string savePath = GetPlayerInventorySavePath();

		if (!FileIO.FileExists(savePath))
		{
			LogTFRAdmin("RestoreAllPlayerInventoriesNow skipped: no player inventory save found. First save/bootstrap mode enabled. Path=" + savePath);

			m_bRestoreCompletedOnce = true;

			if (m_bSaveBlockedByRestore)
			{
				m_bSaveBlockedByRestore = false;
				LogTFRAdmin("Player inventory autosave unlocked because no save file exists. A new save will be created on next autoscan.");
			}

			return;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
		{
			LogTFRError("RestoreAllPlayerInventoriesNow failed: PlayerManager not found.");
			ScheduleRestoreRetryIfNeeded("PlayerManager missing");
			return;
		}

		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();

		if (!ctx.LoadFromFile(savePath))
		{
			LogTFRError("RestoreAllPlayerInventoriesNow failed: LoadFromFile returned false. Path=" + savePath);
			ScheduleRestoreRetryIfNeeded("LoadFromFile failed");
			return;
		}

		int version = 0;
		ctx.ReadValue("version", version);

		int savedPlayerCount = 0;
		ctx.ReadValue("savedPlayerCount", savedPlayerCount);

		if (savedPlayerCount <= 0)
		{
			int fallbackPlayerCount = 0;
			ctx.ReadValue("playerCount", fallbackPlayerCount);
			savedPlayerCount = fallbackPlayerCount;
		}

		if (savedPlayerCount <= 0)
		{
			LogTFRAdmin("RestoreAllPlayerInventoriesNow skipped: save has no saved players. Version=" + version.ToString());
			ScheduleRestoreRetryIfNeeded("save has no players");
			return;
		}

		ref array<int> currentPlayers = new array<int>();
		playerManager.GetPlayers(currentPlayers);

		if (currentPlayers.Count() <= 0)
		{
			LogTFRAdmin("RestoreAllPlayerInventoriesNow waiting: no current players yet.");
			ScheduleRestoreRetryIfNeeded("no current players");
			return;
		}

		m_iLastRestorePlayerCount = 0;
		m_iLastRestoreAttemptCount = 0;
		m_iLastRestoreSuccessCount = 0;
		m_iLastRestoreSkippedCount = 0;
		m_iLastRestoreFailedCount = 0;
		m_iLastCleanRemovedCount = 0;
		m_iLastCleanFailedCount = 0;

		LogTFRAdmin("RestoreAllPlayerInventoriesNow started. Version=" + version.ToString() + " SavedPlayers=" + savedPlayerCount.ToString() + " CurrentPlayers=" + currentPlayers.Count().ToString());

		int matchedPlayers = 0;
		int playersWithControlledEntity = 0;

		for (int i = 0; i < savedPlayerCount; i++)
		{
			string baseKey = "player_" + i.ToString();

			int savedPlayerId = -1;
			string savedPlayerName = string.Empty;

			ctx.ReadValue(baseKey + "_playerId", savedPlayerId);
			ctx.ReadValue(baseKey + "_playerName", savedPlayerName);

			int currentPlayerId = FindCurrentPlayerForSave(playerManager, currentPlayers, savedPlayerId, savedPlayerName);

			if (currentPlayerId < 0)
			{
				LogTFR("Restore skipped: no current player match for saved player. SavedId=" + savedPlayerId.ToString() + " Name=" + savedPlayerName);
				continue;
			}

			matchedPlayers++;

			IEntity controlledEntity = playerManager.GetPlayerControlledEntity(currentPlayerId);

			if (!controlledEntity)
			{
				LogTFR("Restore waiting: matched player has no controlled entity. PlayerId=" + currentPlayerId.ToString() + " Name=" + savedPlayerName);
				continue;
			}

			playersWithControlledEntity++;

			SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(
				controlledEntity.FindComponent(SCR_InventoryStorageManagerComponent)
			);

			if (!inventoryManager)
			{
				LogTFR("Restore skipped: controlled entity has no SCR_InventoryStorageManagerComponent. PlayerId=" + currentPlayerId.ToString());
				continue;
			}

			if (m_bCleanInventoryStorageBeforeRestore)
				CleanInventoryStorageItems(savedPlayerName, inventoryManager);

			RestoreSinglePlayerInventoryFromContext(ctx, baseKey, savedPlayerName, currentPlayerId, inventoryManager);
		}

		if (matchedPlayers > 0 && playersWithControlledEntity <= 0)
		{
			LogTFRAdmin("RestoreAllPlayerInventoriesNow waiting: matched players exist but no controlled entities yet.");
			ScheduleRestoreRetryIfNeeded("matched players without controlled entity");
			return;
		}

		if (m_iLastRestorePlayerCount <= 0 && m_iLastRestoreAttemptCount <= 0)
		{
			LogTFRAdmin("RestoreAllPlayerInventoriesNow finished with no restored players. MatchedPlayers=" + matchedPlayers.ToString() + " ControlledPlayers=" + playersWithControlledEntity.ToString());
			ScheduleRestoreRetryIfNeeded("no restored players");
			return;
		}

		m_bRestoreCompletedOnce = true;

		LogTFRAdmin("RestoreAllPlayerInventoriesNow finished. Players=" + m_iLastRestorePlayerCount.ToString() + " Attempts=" + m_iLastRestoreAttemptCount.ToString() + " Success=" + m_iLastRestoreSuccessCount.ToString() + " Skipped=" + m_iLastRestoreSkippedCount.ToString() + " Failed=" + m_iLastRestoreFailedCount.ToString() + " CleanRemoved=" + m_iLastCleanRemovedCount.ToString() + " CleanFailed=" + m_iLastCleanFailedCount.ToString());

		ScheduleUnlockSaveAfterRestore("restore completed");
	}

	//------------------------------------------------------------------------------------------------
	protected void ScheduleRestoreRetryIfNeeded(string reason)
	{
		if (!m_bRestoreRetryEnabled)
		{
			LogTFRAdmin("Restore retry disabled. Reason=" + reason);
			ScheduleUnlockSaveAfterRestore("restore retry disabled: " + reason);
			return;
		}

		if (m_bRestoreCompletedOnce)
			return;

		if (m_iRestoreRetryRemaining <= 0)
		{
			LogTFRAdmin("Restore retry exhausted. Reason=" + reason);
			ScheduleUnlockSaveAfterRestore("restore retry exhausted: " + reason);
			return;
		}

		m_iRestoreRetryRemaining--;

		if (!GetGame())
			return;

		GetGame().GetCallqueue().Remove(RestoreAllPlayerInventoriesNow);
		GetGame().GetCallqueue().CallLater(RestoreAllPlayerInventoriesNow, m_iRestoreRetryDelayMs, false);

		LogTFRAdmin("Restore retry scheduled. Reason=" + reason + " Remaining=" + m_iRestoreRetryRemaining.ToString() + " DelayMs=" + m_iRestoreRetryDelayMs.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void ScheduleUnlockSaveAfterRestore(string reason)
	{
		if (!m_bSaveBlockedByRestore)
			return;

		if (!GetGame())
			return;

		GetGame().GetCallqueue().Remove(UnlockSaveAfterRestore);
		GetGame().GetCallqueue().CallLater(UnlockSaveAfterRestore, m_iPostRestoreSaveUnlockDelayMs, false);

		LogTFRAdmin("Player inventory autosave unlock scheduled. Reason=" + reason + " DelayMs=" + m_iPostRestoreSaveUnlockDelayMs.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void UnlockSaveAfterRestore()
	{
		if (!m_bSaveBlockedByRestore)
			return;

		m_bSaveBlockedByRestore = false;

		LogTFRAdmin("Player inventory autosave unlocked after restore window.");
	}

	//------------------------------------------------------------------------------------------------
	void TFR_AdminPrintPlayerInventoryPersistenceState()
	{
		LogTFRAdmin("----- PLAYER INVENTORY PERSISTENCE STATE -----");
		LogTFRAdmin("Enabled=" + m_bEnabled.ToString());
		LogTFRAdmin("RunServerOnly=" + m_bRunServerOnly.ToString());
		LogTFRAdmin("DebugLogs=" + m_bDebugLogs.ToString());
		LogTFRAdmin("AutoScanEnabled=" + m_bAutoScanEnabled.ToString());
		LogTFRAdmin("InitialScanDelayMs=" + m_iInitialScanDelayMs.ToString());
		LogTFRAdmin("AutoScanIntervalSeconds=" + m_iAutoScanIntervalSeconds.ToString());
		LogTFRAdmin("SkipSaveWhenNoValidPlayers=" + m_bSkipSaveWhenNoValidPlayers.ToString());
		LogTFRAdmin("RestoreOnStartEnabled=" + m_bRestoreOnStartEnabled.ToString());
		LogTFRAdmin("RestoreOnStartDelayMs=" + m_iRestoreOnStartDelayMs.ToString());
		LogTFRAdmin("RestoreRetryEnabled=" + m_bRestoreRetryEnabled.ToString());
		LogTFRAdmin("RestoreRetryCount=" + m_iRestoreRetryCount.ToString());
		LogTFRAdmin("RestoreRetryRemaining=" + m_iRestoreRetryRemaining.ToString());
		LogTFRAdmin("RestoreRetryDelayMs=" + m_iRestoreRetryDelayMs.ToString());
		LogTFRAdmin("RestoreBlocksSaveUntilCompleted=" + m_bRestoreBlocksSaveUntilCompleted.ToString());
		LogTFRAdmin("PostRestoreSaveUnlockDelayMs=" + m_iPostRestoreSaveUnlockDelayMs.ToString());
		LogTFRAdmin("SaveBlockedByRestore=" + m_bSaveBlockedByRestore.ToString());
		LogTFRAdmin("RestoreCompletedOnce=" + m_bRestoreCompletedOnce.ToString());
		LogTFRAdmin("RestoreInventoryStorageSlotItems=" + m_bRestoreInventoryStorageSlotItems.ToString());
		LogTFRAdmin("RestoreLoadoutSlotItemsFutureUnsafe=" + m_bRestoreLoadoutSlotItems.ToString());
		LogTFRAdmin("RestoreEquipmentStorageSlotItemsFutureUnsafe=" + m_bRestoreEquipmentStorageSlotItems.ToString());
		LogTFRAdmin("AvoidDuplicatePrefabCounts=" + m_bAvoidDuplicatePrefabCounts.ToString());
		LogTFRAdmin("CleanInventoryStorageBeforeRestore=" + m_bCleanInventoryStorageBeforeRestore.ToString());
		LogTFRAdmin("MaxRestoreItemsPerPlayer=" + m_iMaxRestoreItemsPerPlayer.ToString());
		LogTFRAdmin("SavePath=" + GetPlayerInventorySavePath());
		LogTFRAdmin("SaveExists=" + FileIO.FileExists(GetPlayerInventorySavePath()).ToString());
		LogTFRAdmin("LastSavedPlayerCount=" + m_iLastSavedPlayerCount.ToString());
		LogTFRAdmin("LastSavedUniqueItemCount=" + m_iLastSavedItemCount.ToString());
		LogTFRAdmin("LastRawItemCount=" + m_iLastRawItemCount.ToString());
		LogTFRAdmin("LastSkippedSaveCount=" + m_iLastSkippedSaveCount.ToString());
		LogTFRAdmin("LastRestoreBlockedSaveCount=" + m_iLastRestoreBlockedSaveCount.ToString());
		LogTFRAdmin("LastRestorePlayerCount=" + m_iLastRestorePlayerCount.ToString());
		LogTFRAdmin("LastRestoreAttemptCount=" + m_iLastRestoreAttemptCount.ToString());
		LogTFRAdmin("LastRestoreSuccessCount=" + m_iLastRestoreSuccessCount.ToString());
		LogTFRAdmin("LastRestoreSkippedCount=" + m_iLastRestoreSkippedCount.ToString());
		LogTFRAdmin("LastRestoreFailedCount=" + m_iLastRestoreFailedCount.ToString());
		LogTFRAdmin("LastCleanRemovedCount=" + m_iLastCleanRemovedCount.ToString());
		LogTFRAdmin("LastCleanFailedCount=" + m_iLastCleanFailedCount.ToString());
		LogTFRAdmin("----------------------------------------------");
	}

	// -------------------------------------------------------------------------
	// SAVE ONE PLAYER
	// -------------------------------------------------------------------------

	protected int SaveSinglePlayerInventoryToContext(SCR_JsonSaveContext ctx, PlayerManager playerManager, int playerId, int playerIndex, out int rawItemCount)
	{
		rawItemCount = 0;

		if (!playerManager)
			return -1;

		IEntity controlledEntity = playerManager.GetPlayerControlledEntity(playerId);

		if (!controlledEntity)
		{
			LogTFR("Player has no controlled entity. PlayerId=" + playerId.ToString());
			return -1;
		}

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(
			controlledEntity.FindComponent(SCR_InventoryStorageManagerComponent)
		);

		if (!inventoryManager)
		{
			LogTFR("Player controlled entity has no SCR_InventoryStorageManagerComponent. PlayerId=" + playerId.ToString());
			return -1;
		}

		string playerName = playerManager.GetPlayerName(playerId);
		string playerKey = BuildPlayerKey(playerId, playerName);

		ref array<IEntity> rawItems = new array<IEntity>();
		ref array<IEntity> allItems = new array<IEntity>();
		ref array<IEntity> rootItems = new array<IEntity>();

		if (m_bIncludeAllItems)
			inventoryManager.GetItems(rawItems, EStoragePurpose.PURPOSE_ANY);

		rawItemCount = rawItems.Count();

		if (m_bSkipDuplicateEntities)
			BuildUniqueItemArray(rawItems, allItems);
		else
			CopyItemArray(rawItems, allItems);

		if (m_bIncludeRootItems)
			BuildRootLikeItemArray(allItems, rootItems);

		if (!m_bSaveEmptyInventories && allItems.Count() <= 0 && rootItems.Count() <= 0)
			return -1;

		ref array<BaseInventoryStorageComponent> storages = new array<BaseInventoryStorageComponent>();
		inventoryManager.GetStorages(storages, EStoragePurpose.PURPOSE_ANY);

		string baseKey = "player_" + playerIndex.ToString();

		ctx.WriteValue(baseKey + "_playerId", playerId);
		ctx.WriteValue(baseKey + "_playerName", playerName);
		ctx.WriteValue(baseKey + "_playerKey", playerKey);
		ctx.WriteValue(baseKey + "_controlledEntity", controlledEntity.ToString());
		ctx.WriteValue(baseKey + "_storageCount", storages.Count());
		ctx.WriteValue(baseKey + "_rootLikeItemCount", rootItems.Count());
		ctx.WriteValue(baseKey + "_rootItemCount", rootItems.Count());
		ctx.WriteValue(baseKey + "_rawItemCount", rawItems.Count());
		ctx.WriteValue(baseKey + "_uniqueItemCount", allItems.Count());
		ctx.WriteValue(baseKey + "_allItemCount", allItems.Count());
		ctx.WriteValue(baseKey + "_totalWeight", inventoryManager.GetTotalWeightOfAllStorages());

		SaveRootItems(ctx, baseKey, rootItems);
		SaveAllItems(ctx, baseKey, allItems, rootItems);

		LogTFR("Saved player inventory scan. Player=" + playerName + " PlayerId=" + playerId.ToString() + " UniqueItems=" + allItems.Count().ToString() + " RawItems=" + rawItems.Count().ToString());

		return allItems.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveRootItems(SCR_JsonSaveContext ctx, string baseKey, array<IEntity> rootItems)
	{
		if (!rootItems)
		{
			ctx.WriteValue(baseKey + "_root_items_count", 0);
			return;
		}

		ctx.WriteValue(baseKey + "_root_items_count", rootItems.Count());

		for (int i = 0; i < rootItems.Count(); i++)
		{
			IEntity item = rootItems[i];

			string itemKey = baseKey + "_root_item_" + i.ToString();

			SaveItemToContext(ctx, itemKey, item, true, i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveAllItems(SCR_JsonSaveContext ctx, string baseKey, array<IEntity> allItems, array<IEntity> rootItems)
	{
		if (!allItems)
		{
			ctx.WriteValue(baseKey + "_items_count", 0);
			return;
		}

		ctx.WriteValue(baseKey + "_items_count", allItems.Count());

		for (int i = 0; i < allItems.Count(); i++)
		{
			IEntity item = allItems[i];

			bool isRoot = false;

			if (rootItems && rootItems.Find(item) >= 0)
				isRoot = true;

			string itemKey = baseKey + "_item_" + i.ToString();

			SaveItemToContext(ctx, itemKey, item, isRoot, i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveItemToContext(SCR_JsonSaveContext ctx, string itemKey, IEntity item, bool isRoot, int index)
	{
		ctx.WriteValue(itemKey + "_index", index);
		ctx.WriteValue(itemKey + "_valid", item != null);
		ctx.WriteValue(itemKey + "_isRoot", isRoot);

		if (!item)
			return;

		string debugText = item.ToString();

		ResourceName prefab = GetItemPrefab(item);
		string prefabText = prefab;

		ctx.WriteValue(itemKey + "_prefab", prefabText);

		if (m_bIncludeItemDebugStrings)
			ctx.WriteValue(itemKey + "_debug", debugText);

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(
			item.FindComponent(InventoryItemComponent)
		);

		ctx.WriteValue(itemKey + "_hasInventoryItemComponent", itemComponent != null);

		if (!itemComponent)
			return;

		ctx.WriteValue(itemKey + "_locked", itemComponent.IsLocked());
		ctx.WriteValue(itemKey + "_userLocked", itemComponent.IsUserLocked());
		ctx.WriteValue(itemKey + "_systemLocked", itemComponent.IsSystemLocked());
		ctx.WriteValue(itemKey + "_hiddenInVicinity", itemComponent.IsHiddenInVicnity());
		ctx.WriteValue(itemKey + "_additionalWeight", itemComponent.GetAdditionalWeight());
		ctx.WriteValue(itemKey + "_totalWeight", itemComponent.GetTotalWeight());
		ctx.WriteValue(itemKey + "_additionalVolume", itemComponent.GetAdditionalVolume());
		ctx.WriteValue(itemKey + "_totalVolume", itemComponent.GetTotalVolume());

		InventoryStorageSlot parentSlot = itemComponent.GetParentSlot();

		string parentSlotText = string.Empty;
		string parentSlotType = string.Empty;

		if (parentSlot)
		{
			parentSlotText = parentSlot.ToString();
			parentSlotType = ClassifyParentSlot(parentSlotText);
		}

		ctx.WriteValue(itemKey + "_parentSlot", parentSlotText);
		ctx.WriteValue(itemKey + "_parentSlotType", parentSlotType);

		UIInfo uiInfo = itemComponent.GetUIInfo();

		if (uiInfo)
			ctx.WriteValue(itemKey + "_uiName", uiInfo.GetName());
		else
			ctx.WriteValue(itemKey + "_uiName", string.Empty);
	}

	// -------------------------------------------------------------------------
	// CLEAN BEFORE RESTORE
	// -------------------------------------------------------------------------

	protected void CleanInventoryStorageItems(string playerName, SCR_InventoryStorageManagerComponent inventoryManager)
	{
		if (!inventoryManager)
			return;

		ref array<IEntity> currentItems = new array<IEntity>();
		inventoryManager.GetItems(currentItems, EStoragePurpose.PURPOSE_ANY);

		int removed = 0;
		int failed = 0;

		for (int i = currentItems.Count() - 1; i >= 0; i--)
		{
			IEntity item = currentItems[i];

			if (!item)
				continue;

			if (item.IsDeleted())
				continue;

			if (!IsInventoryStorageSlotItem(item))
				continue;

			ResourceName prefab = GetItemPrefab(item);
			string prefabText = prefab;

			SCR_EntityHelper.DeleteEntityAndChildren(item);

			removed++;
			LogTFR("Clean removed inventory storage item. Player=" + playerName + " Prefab=" + prefabText);
		}

		m_iLastCleanRemovedCount += removed;
		m_iLastCleanFailedCount += failed;

		LogTFRAdmin("Clean inventory storage finished. Player=" + playerName + " Removed=" + removed.ToString() + " Failed=" + failed.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsInventoryStorageSlotItem(IEntity item)
	{
		if (!item)
			return false;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(
			item.FindComponent(InventoryItemComponent)
		);

		if (!itemComponent)
			return false;

		InventoryStorageSlot parentSlot = itemComponent.GetParentSlot();

		if (!parentSlot)
			return false;

		string parentSlotText = parentSlot.ToString();
		string parentSlotType = ClassifyParentSlot(parentSlotText);

		return parentSlotType == "INVENTORY_STORAGE_SLOT";
	}

	// -------------------------------------------------------------------------
	// RESTORE ONE PLAYER
	// -------------------------------------------------------------------------

	protected void RestoreSinglePlayerInventoryFromContext(SCR_JsonLoadContext ctx, string baseKey, string savedPlayerName, int currentPlayerId, SCR_InventoryStorageManagerComponent inventoryManager)
	{
		if (!ctx)
			return;

		if (!inventoryManager)
			return;

		int itemCount = 0;
		ctx.ReadValue(baseKey + "_items_count", itemCount);

		if (itemCount <= 0)
			ctx.ReadValue(baseKey + "_allItemCount", itemCount);

		if (itemCount <= 0)
		{
			LogTFR("Restore player skipped: saved item count is zero. Player=" + savedPlayerName);
			return;
		}

		m_iLastRestorePlayerCount++;

		ref array<IEntity> currentItems = new array<IEntity>();
		inventoryManager.GetItems(currentItems, EStoragePurpose.PURPOSE_ANY);

		ref map<string, int> existingPrefabCounts = new map<string, int>();
		BuildExistingPrefabCountMap(currentItems, existingPrefabCounts);

		ref map<string, int> desiredSeenCounts = new map<string, int>();

		int attempted = 0;
		int restored = 0;
		int skipped = 0;
		int failed = 0;

		LogTFRAdmin("Restoring player inventory. Player=" + savedPlayerName + " CurrentPlayerId=" + currentPlayerId.ToString() + " SavedItems=" + itemCount.ToString());

		for (int i = 0; i < itemCount; i++)
		{
			if (attempted >= m_iMaxRestoreItemsPerPlayer)
			{
				LogTFR("Restore max item limit reached for player " + savedPlayerName + ". Limit=" + m_iMaxRestoreItemsPerPlayer.ToString());
				break;
			}

			string itemKey = baseKey + "_item_" + i.ToString();

			string prefabText = string.Empty;
			string parentSlotType = string.Empty;
			string uiName = string.Empty;

			ctx.ReadValue(itemKey + "_prefab", prefabText);
			ctx.ReadValue(itemKey + "_parentSlotType", parentSlotType);
			ctx.ReadValue(itemKey + "_uiName", uiName);

			if (prefabText == string.Empty)
			{
				skipped++;
				continue;
			}

			if (!ShouldRestoreSavedItem(parentSlotType))
			{
				skipped++;
				continue;
			}

			attempted++;

			int desiredCount = IncrementCount(desiredSeenCounts, prefabText);
			int existingCount = GetCountFromMap(existingPrefabCounts, prefabText);

			if (m_bAvoidDuplicatePrefabCounts && existingCount >= desiredCount)
			{
				LogTFR("Restore skip existing prefab count. Prefab=" + prefabText + " Existing=" + existingCount.ToString() + " DesiredSeen=" + desiredCount.ToString());
				skipped++;
				continue;
			}

			ResourceName prefab = prefabText;

			bool ok = inventoryManager.TrySpawnPrefabToStorage(
				prefab,
				null,
				-1,
				EStoragePurpose.PURPOSE_ANY,
				null,
				1
			);

			if (ok)
			{
				restored++;
				IncrementCount(existingPrefabCounts, prefabText);
				LogTFR("Restored item. Player=" + savedPlayerName + " Prefab=" + prefabText + " Name=" + uiName);
			}
			else
			{
				failed++;
				LogTFRError("Restore failed TrySpawnPrefabToStorage. Player=" + savedPlayerName + " Prefab=" + prefabText + " Name=" + uiName);
			}
		}

		m_iLastRestoreAttemptCount += attempted;
		m_iLastRestoreSuccessCount += restored;
		m_iLastRestoreSkippedCount += skipped;
		m_iLastRestoreFailedCount += failed;

		LogTFRAdmin("Restore player finished. Player=" + savedPlayerName + " Attempts=" + attempted.ToString() + " Restored=" + restored.ToString() + " Skipped=" + skipped.ToString() + " Failed=" + failed.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected bool ShouldRestoreSavedItem(string parentSlotType)
	{
		if (parentSlotType == "INVENTORY_STORAGE_SLOT")
			return m_bRestoreInventoryStorageSlotItems;

		if (parentSlotType == "LOADOUT_SLOT")
			return m_bRestoreLoadoutSlotItems;

		if (parentSlotType == "EQUIPMENT_STORAGE_SLOT")
			return m_bRestoreEquipmentStorageSlotItems;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected int FindCurrentPlayerForSave(PlayerManager playerManager, array<int> currentPlayers, int savedPlayerId, string savedPlayerName)
	{
		if (!playerManager)
			return -1;

		if (!currentPlayers)
			return -1;

		if (m_bRestoreMatchPlayerName && savedPlayerName != string.Empty)
		{
			for (int i = 0; i < currentPlayers.Count(); i++)
			{
				int playerId = currentPlayers[i];
				string currentName = playerManager.GetPlayerName(playerId);

				if (currentName == savedPlayerName)
					return playerId;
			}
		}

		if (m_bRestoreMatchPlayerIdFallback && savedPlayerId >= 0)
		{
			for (int j = 0; j < currentPlayers.Count(); j++)
			{
				int currentPlayerId = currentPlayers[j];

				if (currentPlayerId == savedPlayerId)
					return currentPlayerId;
			}
		}

		return -1;
	}

	// -------------------------------------------------------------------------
	// COUNT HELPERS
	// -------------------------------------------------------------------------

	protected void BuildExistingPrefabCountMap(array<IEntity> currentItems, map<string, int> outCounts)
	{
		if (!currentItems)
			return;

		if (!outCounts)
			return;

		outCounts.Clear();

		for (int i = 0; i < currentItems.Count(); i++)
		{
			IEntity item = currentItems[i];

			if (!item)
				continue;

			if (item.IsDeleted())
				continue;

			ResourceName prefab = GetItemPrefab(item);
			string prefabText = prefab;

			if (prefabText == string.Empty)
				continue;

			IncrementCount(outCounts, prefabText);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int IncrementCount(map<string, int> counts, string key)
	{
		if (!counts)
			return 0;

		if (key == string.Empty)
			return 0;

		int value = 0;

		if (counts.Contains(key))
			value = counts.Get(key);

		value++;

		counts.Set(key, value);

		return value;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetCountFromMap(map<string, int> counts, string key)
	{
		if (!counts)
			return 0;

		if (key == string.Empty)
			return 0;

		if (!counts.Contains(key))
			return 0;

		return counts.Get(key);
	}

	// -------------------------------------------------------------------------
	// ITEM ARRAYS
	// -------------------------------------------------------------------------

	protected void CopyItemArray(array<IEntity> sourceItems, array<IEntity> outItems)
	{
		if (!sourceItems)
			return;

		if (!outItems)
			return;

		outItems.Clear();

		for (int i = 0; i < sourceItems.Count(); i++)
		{
			IEntity item = sourceItems[i];

			if (!item)
				continue;

			if (item.IsDeleted())
				continue;

			outItems.Insert(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildUniqueItemArray(array<IEntity> sourceItems, array<IEntity> outUniqueItems)
	{
		if (!sourceItems)
			return;

		if (!outUniqueItems)
			return;

		outUniqueItems.Clear();

		for (int i = 0; i < sourceItems.Count(); i++)
		{
			IEntity item = sourceItems[i];

			if (!item)
				continue;

			if (item.IsDeleted())
				continue;

			if (outUniqueItems.Find(item) >= 0)
				continue;

			outUniqueItems.Insert(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildRootLikeItemArray(array<IEntity> uniqueItems, array<IEntity> outRootItems)
	{
		if (!uniqueItems)
			return;

		if (!outRootItems)
			return;

		outRootItems.Clear();

		for (int i = 0; i < uniqueItems.Count(); i++)
		{
			IEntity item = uniqueItems[i];

			if (!item)
				continue;

			InventoryItemComponent itemComponent = InventoryItemComponent.Cast(
				item.FindComponent(InventoryItemComponent)
			);

			if (!itemComponent)
				continue;

			InventoryStorageSlot parentSlot = itemComponent.GetParentSlot();

			if (!parentSlot)
				continue;

			string parentSlotText = parentSlot.ToString();

			if (parentSlotText.Contains("LoadoutSlotInfo"))
				continue;

			if (parentSlotText.Contains("SCR_EquipmentStorageSlot"))
				continue;

			outRootItems.Insert(item);
		}
	}

	// -------------------------------------------------------------------------
	// ITEM HELPERS
	// -------------------------------------------------------------------------

	protected ResourceName GetItemPrefab(IEntity item)
	{
		if (!item)
			return string.Empty;

		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(
			item.FindComponent(SCR_EditableEntityComponent)
		);

		if (editable)
		{
			ResourceName editablePrefab = editable.GetPrefab(false);

			if (editablePrefab != string.Empty)
				return editablePrefab;
		}

		string debugText = item.ToString();
		ResourceName parsedPrefab = ExtractPrefabFromEntityDebugString(debugText);

		if (parsedPrefab != string.Empty)
			return parsedPrefab;

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName ExtractPrefabFromEntityDebugString(string debugText)
	{
		if (debugText == string.Empty)
			return string.Empty;

		string marker = "@\"{";

		int markerIndex = debugText.IndexOf(marker);

		if (markerIndex < 0)
			return string.Empty;

		int prefabStart = markerIndex + 2;

		int quoteEnd = debugText.IndexOfFrom(prefabStart, "\"");

		if (quoteEnd <= prefabStart)
			return string.Empty;

		int prefabLength = quoteEnd - prefabStart;

		if (prefabLength <= 0)
			return string.Empty;

		string prefabText = debugText.Substring(prefabStart, prefabLength);

		if (prefabText == string.Empty)
			return string.Empty;

		if (!prefabText.Contains(".et"))
			return string.Empty;

		return prefabText;
	}

	//------------------------------------------------------------------------------------------------
	protected string ClassifyParentSlot(string parentSlotText)
	{
		if (parentSlotText == string.Empty)
			return string.Empty;

		if (parentSlotText.Contains("LoadoutSlotInfo"))
			return "LOADOUT_SLOT";

		if (parentSlotText.Contains("SCR_EquipmentStorageSlot"))
			return "EQUIPMENT_STORAGE_SLOT";

		if (parentSlotText.Contains("InventoryStorageSlot"))
			return "INVENTORY_STORAGE_SLOT";

		return "UNKNOWN_SLOT";
	}

	//------------------------------------------------------------------------------------------------
	protected string BuildPlayerKey(int playerId, string playerName)
	{
		string key = "player_";
		key += playerId.ToString();

		if (playerName != string.Empty)
		{
			key += "_";
			key += playerName;
		}

		return key;
	}

	// -------------------------------------------------------------------------
	// PATHS
	// -------------------------------------------------------------------------

	protected void EnsureSaveDirectory()
	{
		if (m_sSaveDirectory == string.Empty)
			m_sSaveDirectory = "$profile:TFR_Operations";

		FileIO.MakeDirectory(m_sSaveDirectory);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetPlayerInventorySavePath()
	{
		string dir = m_sSaveDirectory;

		if (dir == string.Empty)
			dir = "$profile:TFR_Operations";

		string fileName = m_sPlayerInventorySaveFile;

		if (fileName == string.Empty)
			fileName = "player_inventory.json";

		return dir + "/" + fileName;
	}

	// -------------------------------------------------------------------------
	// LOG
	// -------------------------------------------------------------------------

	protected void LogTFR(string msg)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR Player Inventory Persistence] " + msg, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFRAdmin(string msg)
	{
		Print("[TFR Player Inventory Persistence ADMIN] " + msg, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFRError(string msg)
	{
		Print("[TFR Player Inventory Persistence] ERROR: " + msg, LogLevel.ERROR);
	}
}
