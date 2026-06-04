//------------------------------------------------------------------------------------------------
// TFR_Action_Interrogar.c
// TFR OPERATIONS
//
// Acción para interrogar informantes TFR.
//------------------------------------------------------------------------------------------------

class TFR_Action_Interrogar : ScriptedUserAction
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("Interrogar", UIWidgets.EditBox, "Action Name")]
	protected string m_sActionName;

	[Attribute("No obtiene respuesta.", UIWidgets.EditBox, "Failure Hint")]
	protected string m_sFailureHint;

	//------------------------------------------------------------------------------------------------
	void TFR_Action_Interrogar()
	{
		m_bEnabled = true;
		m_bDebugLogs = false;
		m_sActionName = "Interrogar";
		m_sFailureHint = "No obtiene respuesta.";
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_bEnabled)
			return false;

		IEntity owner = GetOwner();

		if (!owner)
			return false;

		TFR_InteractionComponent interaction = TFR_InteractionComponent.Cast(owner.FindComponent(TFR_InteractionComponent));

		if (!interaction)
			return false;

		return interaction.CanInteract();
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
			outName = "Interrogar";
		else
			outName = m_sActionName;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_bEnabled)
			return;

		IEntity owner = pOwnerEntity;

		if (!owner)
			owner = GetOwner();

		if (!owner)
		{
			LogTFR("ERROR: owner null in PerformAction.");
			return;
		}

		TFR_InteractionComponent interaction = TFR_InteractionComponent.Cast(owner.FindComponent(TFR_InteractionComponent));

		if (!interaction)
		{
			LogTFR("ERROR: owner has no TFR_InteractionComponent.");
			ShowHintSafe(m_sFailureHint, "Interrogatorio", 4.0);
			return;
		}

		interaction.Interrogate();
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

		Print("[TFR INTERROGATE] " + msg, LogLevel.NORMAL);
	}
}
