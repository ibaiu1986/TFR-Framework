//------------------------------------------------------------------------------------------------
// TFR_InteractionComponent.c
// TFR Operations
//
// Hints permitidos:
// - respuesta corta del NPC.
// - traición.
// Todo lo demás va al JournalBridge.
//------------------------------------------------------------------------------------------------

enum TFR_InterrogationIntelLevel
{
	NONE = 0,
	VAGUE = 1,
	MEDIUM = 2,
	EXACT = 3
};

class TFR_InteractionComponentClass : ScriptComponentClass
{
};

class TFR_InteractionComponent : ScriptComponent
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	bool m_bDebugLogs;

	[Attribute("Consulta el journal. He dejado allí toda la información.", UIWidgets.EditBox, "Interaction Response")]
	string m_sInteractionResponse;

	[Attribute("0", UIWidgets.ComboBox, "Intel Stage", "", ParamEnumArray.FromEnum(TFR_InterrogationIntelLevel))]
	TFR_InterrogationIntelLevel m_eIntelStage;

	[Attribute("true", UIWidgets.CheckBox, "Single Use")]
	bool m_bSingleUse;

	[Attribute("Información recibida.", UIWidgets.EditBox, "Intel Text For Journal")]
	string m_sIntelHint;

	[Attribute("false", UIWidgets.CheckBox, "Can Trigger Betrayal")]
	bool m_bCanTriggerBetrayal;

	[Attribute("15", UIWidgets.EditBox, "Betrayal Chance %")]
	int m_iBetrayalChance;

	[Attribute("10", UIWidgets.EditBox, "Betrayal Delay Seconds")]
	int m_iBetrayalDelaySeconds;

	[Attribute("0", UIWidgets.EditBox, "Hunt Wave Count")]
	int m_iHuntWaveCount;

	[Attribute("30", UIWidgets.EditBox, "Hunt Wave Interval Seconds")]
	int m_iHuntWaveIntervalSeconds;

	[Attribute("2", UIWidgets.EditBox, "Hunt Wave Size")]
	int m_iHuntWaveSize;

	[Attribute("", UIWidgets.ResourceNamePicker, "Hunt Hunter Prefabs", "et")]
	ref array<ResourceName> m_HuntHunterPrefabs;

	[Attribute("Nos ha vendido. Contacto enemigo en camino.", UIWidgets.EditBox, "Betrayal Hint")]
	string m_sBetrayalHint;

	protected bool m_bConsumed = false;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!m_HuntHunterPrefabs)
			m_HuntHunterPrefabs = new array<ResourceName>();

		LogTFR("Init owner=" + owner + " stage=" + m_eIntelStage.ToString());
	}

	void SetIntelStage(TFR_InterrogationIntelLevel stage)
	{
		m_eIntelStage = stage;
		LogTFR("Intel stage forced by manager: " + m_eIntelStage.ToString() + " owner=" + GetOwner());
	}

	TFR_InterrogationIntelLevel GetIntelStage()
	{
		return m_eIntelStage;
	}

	bool CanInteract()
	{
		if (!m_bEnabled)
			return false;

		if (m_bSingleUse && m_bConsumed)
			return false;

		return true;
	}

	protected int ClampPercent(int value)
	{
		if (value < 0)
			return 0;

		if (value > 100)
			return 100;

		return value;
	}

	protected bool RollPercent(int value)
	{
		int roll = Math.RandomInt(0, 100);
		return roll < ClampPercent(value);
	}

	protected bool RollBetrayal()
	{
		if (!m_bCanTriggerBetrayal)
			return false;

		return RollPercent(m_iBetrayalChance);
	}

	protected void ShowHint(string text, string title = "Interrogatorio", float time = 5.0)
	{
		if (text == string.Empty)
			return;

		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();

		if (hintMgr)
			hintMgr.ShowCustomHint(text, title, time);
	}

	void Interrogate()
	{
		if (!CanInteract())
		{
			ShowHint("Ya no tiene nada más que decir.", "Interrogatorio", 4.0);
			return;
		}

		ShowHint(m_sInteractionResponse, "Interrogatorio", 5.0);

		if (Replication.IsServer())
		{
			LogTFR("SERVER direct process owner=" + GetOwner());
			Server_ProcessInterrogation();
			return;
		}

		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));

		if (!rpl)
		{
			LogTFRError("ERROR CLIENT: NPC has no RplComponent. owner=" + GetOwner());
			return;
		}

		LogTFR("CLIENT -> RPC server owner=" + GetOwner());
		Rpc(Rpc_ServerInterrogate);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_ServerInterrogate()
	{
		LogTFR("SERVER -> RPC received owner=" + GetOwner());
		Server_ProcessInterrogation();
	}

	protected void Server_ProcessInterrogation()
	{
		if (!CanInteract())
		{
			LogTFRWarning("SERVER blocked: consumed/disabled owner=" + GetOwner());
			return;
		}

		TFR_MissionManagerComponent manager = TFR_MissionManagerComponent.GetActiveManager();

		if (!manager)
		{
			LogTFRError("ERROR SERVER: No active TFR_MissionManagerComponent.");
		}
		else
		{
			bool isMissionIntelCivil = manager.IsCurrentIntelCivil(GetOwner());

			if (m_eIntelStage != TFR_InterrogationIntelLevel.NONE || isMissionIntelCivil)
			{
				LogTFR("SERVER -> Intel sent to manager. stage=" + m_eIntelStage.ToString() + " missionIntel=" + isMissionIntelCivil.ToString());
				manager.OnIntelReceived(GetOwner(), m_sIntelHint);
			}
			else
			{
				LogTFR("SERVER -> NPC without intel stage and not active intel. Response only.");
			}

			if (RollBetrayal())
			{
				LogTFRWarning("SERVER -> Betrayal triggered.");

				manager.OnBetrayalTriggered(
					GetOwner(),
					m_iBetrayalDelaySeconds,
					m_iHuntWaveCount,
					m_iHuntWaveIntervalSeconds,
					m_iHuntWaveSize,
					m_HuntHunterPrefabs,
					m_sBetrayalHint
				);

				if (m_sBetrayalHint != string.Empty)
					ShowHint(m_sBetrayalHint, "Delación", 5.0);
			}
		}

		if (m_bSingleUse)
			m_bConsumed = true;
	}

	protected void LogTFR(string message)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR INTERACTION] " + message, LogLevel.NORMAL);
	}

	protected void LogTFRWarning(string message)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR INTERACTION] " + message, LogLevel.WARNING);
	}

	protected void LogTFRError(string message)
	{
		Print("[TFR INTERACTION] " + message, LogLevel.ERROR);
	}
}
