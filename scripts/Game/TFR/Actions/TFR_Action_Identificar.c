//------------------------------------------------------------------------------------------------
// TFR_Action_Identificar.c
// TFR OPERATIONS
//
// Acción para identificar civiles/objetivos TFR.
//------------------------------------------------------------------------------------------------

class TFR_Action_Identificar : SCR_ScriptedUserAction
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("Identificar", UIWidgets.EditBox, "Action Name")]
	protected string m_sActionName;

	[Attribute("Identidad verificada.", UIWidgets.EditBox, "Identification Hint")]
	protected string m_sHintText;

	//------------------------------------------------------------------------------------------------
	void TFR_Action_Identificar()
	{
		m_bEnabled = true;
		m_bDebugLogs = false;
		m_sActionName = "Identificar";
		m_sHintText = "Identidad verificada.";
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_bEnabled)
			return false;

		IEntity owner = GetOwner();

		if (!owner)
			return false;

		SCR_CharacterControllerComponent ctrl = SCR_CharacterControllerComponent.Cast(
			owner.FindComponent(SCR_CharacterControllerComponent)
		);

		if (ctrl)
		{
			if (ctrl.IsDead())
				return false;

			if (ctrl.IsUnconscious())
				return false;
		}

		SCR_CharacterFactionAffiliationComponent fac = SCR_CharacterFactionAffiliationComponent.Cast(
			owner.FindComponent(SCR_CharacterFactionAffiliationComponent)
		);

		if (fac && fac.GetAffiliatedFaction())
		{
			string factionKey = fac.GetAffiliatedFaction().GetFactionKey();

			if (factionKey == "USSR")
				return false;
		}

		TFR_CivilDataComponent data = TFR_CivilDataComponent.Cast(
			owner.FindComponent(TFR_CivilDataComponent)
		);

		if (!data)
			return false;

		if (!data.IsEnabled())
			return false;

		if (data.IsIdentified())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_sActionName == string.Empty)
			outName = "Identificar";
		else
			outName = m_sActionName;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_bEnabled)
			return;

		if (!pOwnerEntity)
			return;

		TFR_CivilDataComponent data = TFR_CivilDataComponent.Cast(
			pOwnerEntity.FindComponent(TFR_CivilDataComponent)
		);

		if (data)
		{
			data.SetIdentified(true);
			LogTFR("Entity marked as identified.");
		}
		else
		{
			LogTFR("WARNING: Missing TFR_CivilDataComponent.");
			return;
		}

		ShowHintSafe(m_sHintText, "Identificación", 4.0);
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowHintSafe(string text, string title, float time)
	{
		if (text == string.Empty)
			return;

		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();

		if (hintMgr)
			hintMgr.ShowCustomHint(text, title, time);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFR(string msg)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR IDENTIFY] " + msg, LogLevel.NORMAL);
	}
}
