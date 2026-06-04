//------------------------------------------------------------------------------------------------
// TFR_JournalTaskBridgeComponent.c
// TFR Operations
//
// VA EN GAMEMODE.
//
// FUNCIONAMIENTO:
// - Busca entidad LayerTask1.
// - Lee SCR_ScenarioFrameworkLayerTask.
// - Usa GetTask().
// - Actualiza con SetTaskName / SetTaskDescription.
// - Para intel/misión activa mantiene ASSIGNED.
// - Al completar misión SOLO marca COMPLETED.
// - No escribe mensaje de "MISIÓN COMPLETADA".
// - Al empezar nueva cadena limpia texto viejo y vuelve a ASSIGNED.
//------------------------------------------------------------------------------------------------

class TFR_JournalTaskBridgeComponentClass : ScriptComponentClass
{
};

enum TFR_EJournalPhase
{
	IDLE = 0,
	INTEL = 1,
	MISSION = 2,
	COMPLETED = 3,
	FAILED = 4
};

class TFR_JournalTaskBridgeComponent : ScriptComponent
{
	protected static TFR_JournalTaskBridgeComponent s_ActiveBridge;

	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("true", UIWidgets.CheckBox, "Run Server Only")]
	protected bool m_bRunServerOnly;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("LayerTask1", UIWidgets.EditBox, "Main LayerTask Entity Name")]
	protected string m_sMainLayerTaskEntityName;

	[Attribute("TFR OPERATIONS", UIWidgets.EditBox, "Default Journal Title")]
	protected string m_sDefaultTitle;

	protected SCR_ScenarioFrameworkLayerTask m_MainLayerTask;
	protected SCR_ScenarioFrameworkTask m_MainTask;

	protected string m_sIntelSection;
	protected string m_sMissionSection;
	protected string m_sLastTitle;
	protected string m_sLastDescription;
	protected string m_sLastGrid;

	protected TFR_EJournalPhase m_eJournalPhase;
	protected int m_iActiveMissionId;

	void TFR_JournalTaskBridgeComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_bEnabled = true;
		m_bRunServerOnly = true;
		m_bDebugLogs = false;
		m_sMainLayerTaskEntityName = "LayerTask1";
		m_sDefaultTitle = "TFR OPERATIONS";
		m_MainLayerTask = null;
		m_MainTask = null;
		m_sIntelSection = string.Empty;
		m_sMissionSection = string.Empty;
		m_sLastTitle = string.Empty;
		m_sLastDescription = string.Empty;
		m_sLastGrid = string.Empty;
		m_eJournalPhase = TFR_EJournalPhase.IDLE;
		m_iActiveMissionId = 0;
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!TFR_CanRunOnThisMachine())
		{
			LogBridge("JournalBridge skipped on client. Server/host only.");
			return;
		}

		s_ActiveBridge = this;

		LogBridge("JournalBridge initialized in GameMode.");
		LogBridge("Main LayerTask name=" + m_sMainLayerTaskEntityName);

		if (!m_bEnabled)
		{
			LogBridge("JournalBridge disabled.");
			return;
		}

		GetGame().GetCallqueue().CallLater(ResolveMainTaskDelayed, 2000, false);
		GetGame().GetCallqueue().CallLater(ResolveMainTaskDelayed, 5000, false);
		GetGame().GetCallqueue().CallLater(ResolveMainTaskDelayed, 10000, false);
	}

	override void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(ResolveMainTaskDelayed);

		if (s_ActiveBridge == this)
			s_ActiveBridge = null;

		super.OnDelete(owner);
	}

	protected bool TFR_CanRunOnThisMachine()
	{
		if (!m_bRunServerOnly)
			return true;

		if (!Replication.IsRunning())
			return true;

		return Replication.IsServer();
	}

	static TFR_JournalTaskBridgeComponent GetActiveBridge()
	{
		return s_ActiveBridge;
	}

	protected void ResolveMainTaskDelayed()
	{
		if (!m_bEnabled)
			return;

		if (!TFR_CanRunOnThisMachine())
			return;

		ResolveMainTask();
	}

	protected bool ResolveMainTask()
	{
		if (!m_bEnabled)
			return false;

		if (!TFR_CanRunOnThisMachine())
			return false;

		if (m_MainLayerTask && m_MainTask)
			return true;

		if (!GetGame())
		{
			LogBridgeError("GetGame null.");
			return false;
		}

		BaseWorld world = GetGame().GetWorld();

		if (!world)
		{
			LogBridgeError("World null.");
			return false;
		}

		if (m_sMainLayerTaskEntityName == string.Empty)
		{
			LogBridgeError("Main LayerTask name empty.");
			return false;
		}

		IEntity layerEntity = world.FindEntityByName(m_sMainLayerTaskEntityName);

		if (!layerEntity)
		{
			LogBridgeError("No se encontró LayerTask principal '" + m_sMainLayerTaskEntityName + "'");
			return false;
		}

		m_MainLayerTask = SCR_ScenarioFrameworkLayerTask.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerTask));

		if (!m_MainLayerTask)
		{
			LogBridgeError("'" + m_sMainLayerTaskEntityName + "' no tiene SCR_ScenarioFrameworkLayerTask");
			return false;
		}

		m_MainTask = m_MainLayerTask.GetTask();

		if (!m_MainTask)
		{
			LogBridgeError("LayerTask principal no devolvió GetTask().");
			return false;
		}

		LogBridge("Main task resolved correctly.");
		return true;
	}

	void TFR_UpdateIntelJournal(string title, string description, string grid = "")
	{
		if (!m_bEnabled)
			return;

		if (!TFR_CanRunOnThisMachine())
			return;

		if (m_eJournalPhase == TFR_EJournalPhase.MISSION)
		{
			LogBridge("Intel update ignored because mission phase is active.");
			return;
		}

		m_eJournalPhase = TFR_EJournalPhase.INTEL;
		m_sLastTitle = title;
		m_sLastDescription = description;
		m_sLastGrid = grid;
		m_sIntelSection = BuildIntelSection(title, description, grid);
		m_sMissionSection = string.Empty;

		ApplyJournal(title, m_sIntelSection, SCR_TaskState.ASSIGNED);
	}

	void TFR_UpdateMissionJournal(int missionId, string title, string description, string grid = "")
	{
		if (!m_bEnabled)
			return;

		if (!TFR_CanRunOnThisMachine())
			return;

		m_eJournalPhase = TFR_EJournalPhase.MISSION;
		m_iActiveMissionId = missionId;
		m_sLastTitle = title;
		m_sLastDescription = description;
		m_sLastGrid = grid;
		m_sIntelSection = string.Empty;
		m_sMissionSection = BuildMissionSection(title, description, grid);

		ApplyJournal(title, m_sMissionSection, SCR_TaskState.ASSIGNED);
	}

	void TFR_SetMissionCompleted(int missionId)
	{
		if (!m_bEnabled)
			return;

		if (!TFR_CanRunOnThisMachine())
			return;

		if (missionId != 0 && m_iActiveMissionId != 0 && missionId != m_iActiveMissionId)
			return;

		m_eJournalPhase = TFR_EJournalPhase.COMPLETED;

		if (!ResolveMainTask())
			return;

		m_MainTask.SetTaskState(SCR_TaskState.COMPLETED);
	}

	void TFR_SetMissionFailed(int missionId)
	{
		if (!m_bEnabled)
			return;

		if (!TFR_CanRunOnThisMachine())
			return;

		if (missionId != 0 && m_iActiveMissionId != 0 && missionId != m_iActiveMissionId)
			return;

		m_eJournalPhase = TFR_EJournalPhase.FAILED;

		if (!ResolveMainTask())
			return;

		m_MainTask.SetTaskState(SCR_TaskState.FAILED);
	}

	void TFR_AdminResetForNewIntelChain()
	{
		m_sIntelSection = string.Empty;
		m_sMissionSection = string.Empty;
		m_sLastTitle = string.Empty;
		m_sLastDescription = string.Empty;
		m_sLastGrid = string.Empty;
		m_iActiveMissionId = 0;
		m_eJournalPhase = TFR_EJournalPhase.IDLE;

		ApplyJournal(m_sDefaultTitle, string.Empty, SCR_TaskState.ASSIGNED);
	}

	protected string BuildIntelSection(string title, string description, string grid)
	{
		string text = description;

		if (grid != string.Empty)
		{
			text += "\n\nGrid: ";
			text += grid;
		}

		return text;
	}

	protected string BuildMissionSection(string title, string description, string grid)
	{
		string text = description;

		if (grid != string.Empty)
		{
			text += "\n\nGrid: ";
			text += grid;
		}

		return text;
	}

	protected void ApplyJournal(string title, string description, SCR_TaskState state)
	{
		if (!ResolveMainTask())
			return;

		if (title == string.Empty)
			title = m_sDefaultTitle;

		m_MainTask.SetTaskName(title);
		m_MainTask.SetTaskDescription(description);
		m_MainTask.SetTaskState(state);
	}

	protected void LogBridge(string msg)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR JOURNAL BRIDGE] " + msg, LogLevel.NORMAL);
	}

	protected void LogBridgeError(string msg)
	{
		Print("[TFR JOURNAL BRIDGE] ERROR: " + msg, LogLevel.ERROR);
	}
}
