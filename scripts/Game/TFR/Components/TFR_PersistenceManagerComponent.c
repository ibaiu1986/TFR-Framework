//------------------------------------------------------------------------------------------------
// TFR_PersistenceManagerComponent.c
// TFR OPERATIONS
//
// V0.4 CLEAN OPERATION PERSISTENCE
//
// VA EN GAMEMODE.
//
// OBJETIVO:
// - Punto único de persistencia TFR.
// - Guarda estado lógico de operación en $profile.
// - Restaura operación/intel/misión al reiniciar.
// - Prepara archivos separados para inventario jugador y maleteros/cargo vehículo.
// - Permite resets admin separados.
//
// V0.1:
// - Save Operation Now.
// - Reset Operation.
// - Reset Player Inventory.
// - Reset Vehicle Cargo.
// - Reset All.
// - Load Operation preparado.
//
// V0.2:
// - Load Operation Now restaura realmente llamando a:
//   TFR_MissionManagerComponent.TFR_RestoreOperationFromPersistence(...)
//
// V0.3:
// - Logs ADMIN forzados para diagnosticar guardado.
//
// V0.4:
// - Limpieza.
// - El constructor ya NO pisa valores configurados en Workbench.
// - AutoSaveIntervalSeconds, DebugLogs, paths y toggles respetan Attributes/Workbench.
// - Logs reducidos a información útil.
//
// V0.4.1 SERVER POLISH:
// - Admin Logs configurable.
// - Por defecto sigue activo para no cambiar comportamiento actual.
// - Errores críticos siguen siempre visibles.
// - No cambia JSON.
// - No cambia Save/Load/Reset.
// - No cambia autosave.
//------------------------------------------------------------------------------------------------

class TFR_PersistenceManagerComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class TFR_PersistenceManagerComponent : ScriptComponent
{
	protected static TFR_PersistenceManagerComponent s_ActivePersistence;

	// -------------------------------------------------------------------------
	// GENERAL
	// -------------------------------------------------------------------------

	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("true", UIWidgets.CheckBox, "Run Server Only")]
	protected bool m_bRunServerOnly;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("true", UIWidgets.CheckBox, "Admin Logs")]
	protected bool m_bAdminLogs;

	[Attribute("true", UIWidgets.CheckBox, "Load Operation On Start")]
	protected bool m_bLoadOperationOnStart;

	[Attribute("8000", UIWidgets.EditBox, "Load Delay MS")]
	protected int m_iLoadDelayMs;

	[Attribute("true", UIWidgets.CheckBox, "Auto Save Enabled")]
	protected bool m_bAutoSaveEnabled;

	[Attribute("300", UIWidgets.EditBox, "Auto Save Interval Seconds")]
	protected int m_iAutoSaveIntervalSeconds;

	// -------------------------------------------------------------------------
	// FILES
	// -------------------------------------------------------------------------

	[Attribute("$profile:TFR_Operations", UIWidgets.EditBox, "Save Directory")]
	protected string m_sSaveDirectory;

	[Attribute("operation_state.json", UIWidgets.EditBox, "Operation Save File")]
	protected string m_sOperationSaveFile;

	[Attribute("player_inventory.json", UIWidgets.EditBox, "Player Inventory Save File")]
	protected string m_sPlayerInventorySaveFile;

	[Attribute("vehicle_cargo.json", UIWidgets.EditBox, "Vehicle Cargo Save File")]
	protected string m_sVehicleCargoSaveFile;

	// -------------------------------------------------------------------------
	// RUNTIME CACHE
	// -------------------------------------------------------------------------

	protected bool m_bLoadedOperationFile;
	protected bool m_bLastLoadedOperationActive;
	protected bool m_bLastLoadedIntelChainActive;
	protected bool m_bLastLoadedMissionActive;

	protected int m_iLastLoadedIntelStep;
	protected int m_iLastLoadedRequiredIntelSteps;
	protected int m_iLastLoadedIntelJobType;
	protected int m_iLastLoadedMissionType;
	protected int m_iLastLoadedMissionArea;
	protected int m_iLastLoadedActiveMissionId;

	protected vector m_vLastLoadedIntelJobPosition;
	protected vector m_vLastLoadedMissionPosition;
	protected vector m_vLastLoadedConvoyStartPosition;
	protected vector m_vLastLoadedConvoyDestinationPosition;

	//------------------------------------------------------------------------------------------------
	void TFR_PersistenceManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		// Importante:
		// No asignar aquí valores que ya vienen de [Attribute].
		// Si lo hacemos, Workbench/Prefab puede quedar pisado en runtime.
		ClearLoadedOperationCache();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!CanRunOnThisMachine())
			return;

		s_ActivePersistence = this;

		ValidateSettings();
		EnsureSaveDirectory();

		LogTFRAdmin("Persistence manager initialized.");
		LogTFR("Owner=" + owner.ToString());
		LogTFR("OperationPath=" + GetOperationSavePath());
		LogTFR("InventoryPath=" + GetPlayerInventorySavePath());
		LogTFR("VehicleCargoPath=" + GetVehicleCargoSavePath());
		LogTFR("AutoSaveEnabled=" + m_bAutoSaveEnabled.ToString());
		LogTFR("AutoSaveIntervalSeconds=" + m_iAutoSaveIntervalSeconds.ToString());
		LogTFR("LoadOperationOnStart=" + m_bLoadOperationOnStart.ToString());
		LogTFR("LoadDelayMs=" + m_iLoadDelayMs.ToString());
		LogTFR("AdminLogs=" + m_bAdminLogs.ToString());

		if (m_bLoadOperationOnStart && GetGame())
		{
			GetGame().GetCallqueue().Remove(LoadOperationNow);
			GetGame().GetCallqueue().CallLater(LoadOperationNow, m_iLoadDelayMs, false);
		}

		if (m_bAutoSaveEnabled)
			StartAutoSaveLoop();
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(LoadOperationNow);
			GetGame().GetCallqueue().Remove(AutoSaveTick);
		}

		if (s_ActivePersistence == this)
			s_ActivePersistence = null;

		LogTFR("Persistence manager deleted.");

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static TFR_PersistenceManagerComponent GetActivePersistence()
	{
		return s_ActivePersistence;
	}

	// -------------------------------------------------------------------------
	// VALIDATION / SERVER
	// -------------------------------------------------------------------------

	protected void ValidateSettings()
	{
		if (m_iLoadDelayMs < 0)
			m_iLoadDelayMs = 0;

		if (m_iAutoSaveIntervalSeconds <= 0)
			m_iAutoSaveIntervalSeconds = 300;

		if (m_iAutoSaveIntervalSeconds < 30)
			m_iAutoSaveIntervalSeconds = 30;

		if (m_sSaveDirectory == string.Empty)
			m_sSaveDirectory = "$profile:TFR_Operations";

		if (m_sOperationSaveFile == string.Empty)
			m_sOperationSaveFile = "operation_state.json";

		if (m_sPlayerInventorySaveFile == string.Empty)
			m_sPlayerInventorySaveFile = "player_inventory.json";

		if (m_sVehicleCargoSaveFile == string.Empty)
			m_sVehicleCargoSaveFile = "vehicle_cargo.json";
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanRunOnThisMachine()
	{
		if (!m_bEnabled)
			return false;

		if (!m_bRunServerOnly)
			return true;

		if (!Replication.IsRunning())
			return true;

		return Replication.IsServer();
	}

	// -------------------------------------------------------------------------
	// AUTOSAVE
	// -------------------------------------------------------------------------

	protected void StartAutoSaveLoop()
	{
		if (!CanRunOnThisMachine())
			return;

		if (!GetGame())
			return;

		int intervalMs = Math.Max(m_iAutoSaveIntervalSeconds, 30) * 1000;

		GetGame().GetCallqueue().Remove(AutoSaveTick);
		GetGame().GetCallqueue().CallLater(AutoSaveTick, intervalMs, true);

		LogTFRAdmin("AutoSave started. IntervalSeconds=" + m_iAutoSaveIntervalSeconds.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void AutoSaveTick()
	{
		if (!CanRunOnThisMachine())
			return;

		LogTFR("AutoSaveTick fired.");

		SaveOperationNow();
	}

	// -------------------------------------------------------------------------
	// SAVE / LOAD OPERATION
	// -------------------------------------------------------------------------

	void SaveOperationNow()
	{
		if (!CanRunOnThisMachine())
			return;

		EnsureSaveDirectory();

		string savePath = GetOperationSavePath();

		TFR_MissionManagerComponent missionManager = TFR_MissionManagerComponent.GetActiveManager();

		if (!missionManager)
		{
			LogTFRError("SaveOperationNow failed: MissionManager not found.");
			return;
		}

		bool intelChainActive = missionManager.IsIntelChainActive();
		bool missionActive = missionManager.IsMissionActive();
		bool operationActive = intelChainActive || missionActive;

		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext(false);
		ctx.SetMaxDecimalPlaces(3);

		ctx.WriteValue("version", 4);
		ctx.WriteValue("operationActive", operationActive);
		ctx.WriteValue("intelChainActive", intelChainActive);
		ctx.WriteValue("missionActive", missionActive);
		ctx.WriteValue("missionResolved", missionManager.IsMissionResolved());

		ctx.WriteValue("intelStep", missionManager.GetCurrentIntelStep());
		ctx.WriteValue("requiredIntelSteps", missionManager.GetRequiredIntelSteps());
		ctx.WriteValue("currentIntelJobType", missionManager.GetCurrentIntelJobType());
		ctx.WriteValue("currentIntelJobPosition", missionManager.GetCurrentIntelJobPosition());

		ctx.WriteValue("missionType", missionManager.GetCurrentMissionType());
		ctx.WriteValue("missionArea", missionManager.GetCurrentMissionArea());
		ctx.WriteValue("missionPosition", missionManager.GetCurrentMissionPosition());
		ctx.WriteValue("activeMissionId", missionManager.GetActiveMissionId());

		ctx.WriteValue("destroyConvoyStartPosition", missionManager.GetCurrentMissionPosition());
		ctx.WriteValue("destroyConvoyDestinationPosition", missionManager.GetDestroyConvoyDestinationPosition());

		bool saved = ctx.SaveToFile(savePath);

		if (!saved)
		{
			LogTFRError("SaveOperationNow failed: SaveToFile returned false. Path=" + savePath);
			return;
		}

		LogTFRAdmin("Operation saved. Active=" + operationActive.ToString() + " Intel=" + intelChainActive.ToString() + " Mission=" + missionActive.ToString());
		LogTFR("Saved path=" + savePath);
	}

	//------------------------------------------------------------------------------------------------
	void LoadOperationNow()
	{
		if (!CanRunOnThisMachine())
			return;

		ClearLoadedOperationCache();

		string path = GetOperationSavePath();

		LogTFRAdmin("LoadOperationNow started.");

		if (!FileIO.FileExists(path))
		{
			LogTFRAdmin("No operation save found.");
			return;
		}

		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext(false);

		if (!ctx.LoadFromFile(path))
		{
			LogTFRError("LoadOperationNow failed: LoadFromFile returned false.");
			return;
		}

		int saveVersion = 0;
		ctx.ReadValue("version", saveVersion);

		ctx.ReadValue("operationActive", m_bLastLoadedOperationActive);
		ctx.ReadValue("intelChainActive", m_bLastLoadedIntelChainActive);
		ctx.ReadValue("missionActive", m_bLastLoadedMissionActive);
		ctx.ReadValue("intelStep", m_iLastLoadedIntelStep);
		ctx.ReadValue("requiredIntelSteps", m_iLastLoadedRequiredIntelSteps);
		ctx.ReadValue("currentIntelJobType", m_iLastLoadedIntelJobType);
		ctx.ReadValue("currentIntelJobPosition", m_vLastLoadedIntelJobPosition);
		ctx.ReadValue("missionType", m_iLastLoadedMissionType);
		ctx.ReadValue("missionArea", m_iLastLoadedMissionArea);
		ctx.ReadValue("missionPosition", m_vLastLoadedMissionPosition);
		ctx.ReadValue("activeMissionId", m_iLastLoadedActiveMissionId);
		ctx.ReadValue("destroyConvoyStartPosition", m_vLastLoadedConvoyStartPosition);
		ctx.ReadValue("destroyConvoyDestinationPosition", m_vLastLoadedConvoyDestinationPosition);

		m_bLoadedOperationFile = true;

		LogTFRAdmin("Operation save loaded. Version=" + saveVersion.ToString());
		LogTFRAdmin("Active=" + m_bLastLoadedOperationActive.ToString() + " Intel=" + m_bLastLoadedIntelChainActive.ToString() + " Mission=" + m_bLastLoadedMissionActive.ToString());
		LogTFR("IntelStep=" + m_iLastLoadedIntelStep.ToString() + "/" + m_iLastLoadedRequiredIntelSteps.ToString());
		LogTFR("IntelJobType=" + m_iLastLoadedIntelJobType.ToString());
		LogTFR("MissionType=" + m_iLastLoadedMissionType.ToString());
		LogTFR("MissionArea=" + m_iLastLoadedMissionArea.ToString());
		LogTFR("MissionId=" + m_iLastLoadedActiveMissionId.ToString());

		if (!m_bLastLoadedOperationActive)
		{
			LogTFRAdmin("Loaded operation save is not active. Runtime untouched.");
			return;
		}

		TFR_MissionManagerComponent missionManager = TFR_MissionManagerComponent.GetActiveManager();

		if (!missionManager)
		{
			LogTFRError("LoadOperationNow failed: MissionManager not found for restore.");
			return;
		}

		bool restored = missionManager.TFR_RestoreOperationFromPersistence(
			m_bLastLoadedOperationActive,
			m_bLastLoadedIntelChainActive,
			m_bLastLoadedMissionActive,
			m_iLastLoadedIntelStep,
			m_iLastLoadedRequiredIntelSteps,
			m_iLastLoadedIntelJobType,
			m_vLastLoadedIntelJobPosition,
			m_iLastLoadedMissionType,
			m_iLastLoadedMissionArea,
			m_vLastLoadedMissionPosition,
			m_iLastLoadedActiveMissionId,
			m_vLastLoadedConvoyStartPosition,
			m_vLastLoadedConvoyDestinationPosition
		);

		if (!restored)
		{
			LogTFRError("LoadOperationNow restore returned false.");
			return;
		}

		LogTFRAdmin("Operation restored from persistence.");
	}

	// -------------------------------------------------------------------------
	// RESET API
	// -------------------------------------------------------------------------

	void ResetOperationPersistence(bool restartOperation = true)
	{
		if (!CanRunOnThisMachine())
			return;

		DeleteFileIfExists(GetOperationSavePath());
		ClearLoadedOperationCache();

		TFR_MissionManagerComponent missionManager = TFR_MissionManagerComponent.GetActiveManager();

		if (missionManager && restartOperation)
			missionManager.TFR_AdminCancelAndRestartIntel();

		LogTFRAdmin("Operation persistence reset. RestartOperation=" + restartOperation.ToString());
	}

	//------------------------------------------------------------------------------------------------
	void ResetPlayerInventoryPersistence()
	{
		if (!CanRunOnThisMachine())
			return;

		DeleteFileIfExists(GetPlayerInventorySavePath());
		LogTFRAdmin("Player inventory persistence reset.");
	}

	//------------------------------------------------------------------------------------------------
	void ResetVehicleCargoPersistence()
	{
		if (!CanRunOnThisMachine())
			return;

		DeleteFileIfExists(GetVehicleCargoSavePath());
		LogTFRAdmin("Vehicle cargo persistence reset.");
	}

	//------------------------------------------------------------------------------------------------
	void ResetAllPersistence(bool restartOperation = true)
	{
		if (!CanRunOnThisMachine())
			return;

		DeleteFileIfExists(GetOperationSavePath());
		DeleteFileIfExists(GetPlayerInventorySavePath());
		DeleteFileIfExists(GetVehicleCargoSavePath());
		ClearLoadedOperationCache();

		TFR_MissionManagerComponent missionManager = TFR_MissionManagerComponent.GetActiveManager();

		if (missionManager && restartOperation)
			missionManager.TFR_AdminCancelAndRestartIntel();

		LogTFRAdmin("ALL persistence reset. RestartOperation=" + restartOperation.ToString());
	}

	// -------------------------------------------------------------------------
	// PATHS / FILE HELPERS
	// -------------------------------------------------------------------------

	protected void EnsureSaveDirectory()
	{
		if (m_sSaveDirectory == string.Empty)
			m_sSaveDirectory = "$profile:TFR_Operations";

		FileIO.MakeDirectory(m_sSaveDirectory);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetOperationSavePath()
	{
		return BuildSavePath(m_sOperationSaveFile);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetPlayerInventorySavePath()
	{
		return BuildSavePath(m_sPlayerInventorySaveFile);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetVehicleCargoSavePath()
	{
		return BuildSavePath(m_sVehicleCargoSaveFile);
	}

	//------------------------------------------------------------------------------------------------
	protected string BuildSavePath(string fileName)
	{
		string dir = m_sSaveDirectory;

		if (dir == string.Empty)
			dir = "$profile:TFR_Operations";

		if (fileName == string.Empty)
			fileName = "tfr_save.json";

		return dir + "/" + fileName;
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteFileIfExists(string path)
	{
		if (path == string.Empty)
			return;

		if (!FileIO.FileExists(path))
			return;

		if (!FileIO.DeleteFile(path))
		{
			LogTFRError("Failed to delete file: " + path);
			return;
		}

		LogTFR("Deleted file: " + path);
	}

	// -------------------------------------------------------------------------
	// CACHE
	// -------------------------------------------------------------------------

	protected void ClearLoadedOperationCache()
	{
		m_bLoadedOperationFile = false;
		m_bLastLoadedOperationActive = false;
		m_bLastLoadedIntelChainActive = false;
		m_bLastLoadedMissionActive = false;

		m_iLastLoadedIntelStep = 0;
		m_iLastLoadedRequiredIntelSteps = 0;
		m_iLastLoadedIntelJobType = 0;
		m_iLastLoadedMissionType = 0;
		m_iLastLoadedMissionArea = 0;
		m_iLastLoadedActiveMissionId = 0;

		m_vLastLoadedIntelJobPosition = vector.Zero;
		m_vLastLoadedMissionPosition = vector.Zero;
		m_vLastLoadedConvoyStartPosition = vector.Zero;
		m_vLastLoadedConvoyDestinationPosition = vector.Zero;
	}

	// -------------------------------------------------------------------------
	// PUBLIC DEBUG STATE
	// -------------------------------------------------------------------------

	void TFR_AdminPrintPersistenceState()
	{
		LogTFRAdmin("----- PERSISTENCE STATE -----");
		LogTFRAdmin("Enabled=" + m_bEnabled.ToString());
		LogTFRAdmin("RunServerOnly=" + m_bRunServerOnly.ToString());
		LogTFRAdmin("DebugLogs=" + m_bDebugLogs.ToString());
		LogTFRAdmin("AdminLogs=" + m_bAdminLogs.ToString());
		LogTFRAdmin("LoadOperationOnStart=" + m_bLoadOperationOnStart.ToString());
		LogTFRAdmin("LoadDelayMs=" + m_iLoadDelayMs.ToString());
		LogTFRAdmin("AutoSaveEnabled=" + m_bAutoSaveEnabled.ToString());
		LogTFRAdmin("AutoSaveIntervalSeconds=" + m_iAutoSaveIntervalSeconds.ToString());
		LogTFRAdmin("SaveDirectory=" + m_sSaveDirectory);
		LogTFRAdmin("OperationPath=" + GetOperationSavePath());
		LogTFRAdmin("InventoryPath=" + GetPlayerInventorySavePath());
		LogTFRAdmin("VehicleCargoPath=" + GetVehicleCargoSavePath());
		LogTFRAdmin("OperationSaveExists=" + FileIO.FileExists(GetOperationSavePath()).ToString());
		LogTFRAdmin("InventorySaveExists=" + FileIO.FileExists(GetPlayerInventorySavePath()).ToString());
		LogTFRAdmin("VehicleCargoSaveExists=" + FileIO.FileExists(GetVehicleCargoSavePath()).ToString());
		LogTFRAdmin("LoadedOperationFile=" + m_bLoadedOperationFile.ToString());
		LogTFRAdmin("LoadedOperationActive=" + m_bLastLoadedOperationActive.ToString());
		LogTFRAdmin("LoadedIntelActive=" + m_bLastLoadedIntelChainActive.ToString());
		LogTFRAdmin("LoadedMissionActive=" + m_bLastLoadedMissionActive.ToString());
		LogTFRAdmin("LoadedIntelStep=" + m_iLastLoadedIntelStep.ToString() + "/" + m_iLastLoadedRequiredIntelSteps.ToString());
		LogTFRAdmin("LoadedIntelJobType=" + m_iLastLoadedIntelJobType.ToString());
		LogTFRAdmin("LoadedMissionType=" + m_iLastLoadedMissionType.ToString());
		LogTFRAdmin("LoadedMissionArea=" + m_iLastLoadedMissionArea.ToString());
		LogTFRAdmin("LoadedMissionId=" + m_iLastLoadedActiveMissionId.ToString());
		LogTFRAdmin("-----------------------------");
	}

	// -------------------------------------------------------------------------
	// LOG
	// -------------------------------------------------------------------------

	protected void LogTFR(string msg)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR Persistence] " + msg, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFRAdmin(string msg)
	{
		if (!m_bAdminLogs)
			return;

		Print("[TFR Persistence ADMIN] " + msg, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFRError(string msg)
	{
		Print("[TFR Persistence] ERROR: " + msg, LogLevel.ERROR);
	}
}
