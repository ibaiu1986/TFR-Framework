//------------------------------------------------------------------------------------------------
// TFR_MissionAdminConsoleComponent.c
// TFR OPERATIONS
//
// Componente físico para consola/radio administrativa de misión.
//------------------------------------------------------------------------------------------------

class TFR_MissionAdminConsoleComponentClass : ScriptComponentClass
{
}

class TFR_MissionAdminConsoleComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("0", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebug;

	[Attribute("US", UIWidgets.EditBox, "Authorized Faction Key. Empty = use object faction")]
	protected string m_sAuthorizedFactionKey;

	[Attribute("1", UIWidgets.CheckBox, "Hide Actions If Unauthorized")]
	protected bool m_bHideActionsIfUnauthorized;

	protected IEntity m_Owner;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_Owner = owner;

		if (m_sAuthorizedFactionKey == string.Empty)
		{
			string ownerFaction = TFR_GetFactionKey(owner);

			if (ownerFaction != string.Empty)
				m_sAuthorizedFactionKey = ownerFaction;
		}

		TFR_DebugLog("Admin console initialized. Owner=" + owner + " AuthorizedFaction=" + m_sAuthorizedFactionKey);
	}

	bool TFR_IsEnabled()
	{
		return m_bEnabled;
	}

	bool TFR_ShouldHideActionsIfUnauthorized()
	{
		return m_bHideActionsIfUnauthorized;
	}

	string TFR_GetAuthorizedFactionKey()
	{
		return m_sAuthorizedFactionKey;
	}

	bool TFR_CanUserAccess(IEntity user)
	{
		if (!m_bEnabled)
			return false;

		if (!user)
			return false;

		string requiredFaction = m_sAuthorizedFactionKey;

		if (requiredFaction == string.Empty && m_Owner)
			requiredFaction = TFR_GetFactionKey(m_Owner);

		if (requiredFaction == string.Empty)
		{
			TFR_DebugLog("Access blocked: no authorized faction configured and object has no faction.");
			return false;
		}

		string userFaction = TFR_GetFactionKey(user);

		if (userFaction == string.Empty)
		{
			TFR_DebugLog("Access blocked: user faction could not be resolved.");
			return false;
		}

		if (userFaction != requiredFaction)
		{
			TFR_DebugLog("Access blocked by faction. UserFaction=" + userFaction + " RequiredFaction=" + requiredFaction);
			return false;
		}

		return true;
	}

	bool TFR_CanShowAction(IEntity user)
	{
		if (!m_bEnabled)
			return false;

		if (!m_bHideActionsIfUnauthorized)
			return true;

		return TFR_CanUserAccess(user);
	}

	void TFR_AdminPrintState(IEntity user)
	{
		if (!TFR_CanUserAccess(user))
		{
			TFR_LogDenied(user, "Print state");
			return;
		}

		TFR_MissionManagerComponent manager = TFR_GetMissionManager();

		if (!manager)
		{
			TFR_LogNoManager("Print state");
			return;
		}

		TFR_LogAction(user, "Print mission state");
		manager.TFR_AdminPrintState();
	}

	void TFR_AdminForceNextIntel(IEntity user)
	{
		if (!TFR_CanUserAccess(user))
		{
			TFR_LogDenied(user, "Force next intel");
			return;
		}

		TFR_MissionManagerComponent manager = TFR_GetMissionManager();

		if (!manager)
		{
			TFR_LogNoManager("Force next intel");
			return;
		}

		TFR_LogAction(user, "Force next intel");
		manager.TFR_AdminForceNextIntel();
	}

	void TFR_AdminCompleteMission(IEntity user)
	{
		if (!TFR_CanUserAccess(user))
		{
			TFR_LogDenied(user, "Complete mission");
			return;
		}

		TFR_MissionManagerComponent manager = TFR_GetMissionManager();

		if (!manager)
		{
			TFR_LogNoManager("Complete mission");
			return;
		}

		TFR_LogAction(user, "Complete mission");
		manager.TFR_AdminCompleteMission();
	}

	void TFR_AdminFailMission(IEntity user)
	{
		if (!TFR_CanUserAccess(user))
		{
			TFR_LogDenied(user, "Fail mission");
			return;
		}

		TFR_MissionManagerComponent manager = TFR_GetMissionManager();

		if (!manager)
		{
			TFR_LogNoManager("Fail mission");
			return;
		}

		TFR_LogAction(user, "Fail mission");
		manager.TFR_AdminFailMission();
	}

	void TFR_AdminCancelAndRestartIntel(IEntity user)
	{
		if (!TFR_CanUserAccess(user))
		{
			TFR_LogDenied(user, "Cancel and restart intel");
			return;
		}

		TFR_MissionManagerComponent manager = TFR_GetMissionManager();

		if (!manager)
		{
			TFR_LogNoManager("Cancel and restart intel");
			return;
		}

		TFR_LogAction(user, "Cancel mission and restart intel chain");
		manager.TFR_AdminCancelAndRestartIntel();
	}

	protected TFR_MissionManagerComponent TFR_GetMissionManager()
	{
		return TFR_MissionManagerComponent.GetActiveManager();
	}

	protected string TFR_GetFactionKey(IEntity ent)
	{
		if (!ent)
			return string.Empty;

		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));

		if (!factionComp)
			return string.Empty;

		Faction faction = factionComp.GetAffiliatedFaction();

		if (!faction)
			return string.Empty;

		return faction.GetFactionKey();
	}

	protected void TFR_LogAction(IEntity user, string actionName)
	{
		if (!m_bDebug)
			return;

		string userFaction = TFR_GetFactionKey(user);

		string msg = "Action executed: ";
		msg += actionName;
		msg += " User=" + user;
		msg += " UserFaction=" + userFaction;

		Print("[TFR ADMIN CONSOLE] " + msg, LogLevel.NORMAL);
	}

	protected void TFR_LogDenied(IEntity user, string actionName)
	{
		if (!m_bDebug)
			return;

		string userFaction = TFR_GetFactionKey(user);

		string msg = "Action denied: ";
		msg += actionName;
		msg += " User=" + user;
		msg += " UserFaction=" + userFaction;
		msg += " RequiredFaction=" + m_sAuthorizedFactionKey;

		Print("[TFR ADMIN CONSOLE] " + msg, LogLevel.WARNING);
	}

	protected void TFR_LogNoManager(string actionName)
	{
		string msg = "Action cancelled: ";
		msg += actionName;
		msg += ". No active TFR_MissionManagerComponent exists.";

		Print("[TFR ADMIN CONSOLE] " + msg, LogLevel.ERROR);
	}

	protected void TFR_DebugLog(string message)
	{
		if (!m_bDebug)
			return;

		Print("[TFR ADMIN CONSOLE] " + message, LogLevel.NORMAL);
	}
}
