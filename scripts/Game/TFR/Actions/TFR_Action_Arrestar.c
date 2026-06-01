class TFR_Action_Arrestar : SCR_ScriptedUserAction
{
	[Attribute("El sospechoso puede rendirse ahora.", UIWidgets.EditBox, "Texto mostrado al arrestar")]
	protected string m_sTextoArresto;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("true", UIWidgets.CheckBox, "Try External Surrender Provider")]
	protected bool m_bTryExternalSurrenderProvider;

	//------------------------------------------------------------------------------------------------
	void TFR_Action_Arrestar()
	{
		m_bDebugLogs = false;
		m_bTryExternalSurrenderProvider = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();

		if (!owner)
			return false;

		TFR_CivilDataComponent data = TFR_CivilDataComponent.Cast(owner.FindComponent(TFR_CivilDataComponent));

		if (!data)
			return false;

		if (!data.IsEnabled())
			return false;

		if (!data.HasArrestOrder())
			return false;

		if (!data.IsIdentified())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity)
			return;

		ShowHintSafe(m_sTextoArresto, "Arrestar", 4.0);

		TFR_CivilDataComponent data = TFR_CivilDataComponent.Cast(
			pOwnerEntity.FindComponent(TFR_CivilDataComponent)
		);

		if (data)
		{
			data.m_bPuedeRendirse = true;
			LogTFR("CivilData marcado como puede rendirse.");
		}
		else
		{
			LogTFRWarning("Falta TFR_CivilDataComponent.");
		}

		TFR_SurrenderControlComponent ctrl = TFR_SurrenderControlComponent.Cast(
			pOwnerEntity.FindComponent(TFR_SurrenderControlComponent)
		);

		if (ctrl)
		{
			ctrl.EnableSurrender();
			LogTFR("Surrender habilitado en TFR_SurrenderControlComponent.");
		}
		else
		{
			LogTFRWarning("Falta TFR_SurrenderControlComponent.");
		}

		TFR_TryExternalSurrenderProvider(pOwnerEntity, pUserEntity);

		TFR_MissionManagerComponent manager = TFR_MissionManagerComponent.GetActiveManager();

		if (manager)
			manager.NotifyTargetArrested(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_TryExternalSurrenderProvider(IEntity targetEntity, IEntity userEntity)
	{
		if (!m_bTryExternalSurrenderProvider)
			return;

		if (!targetEntity)
			return;

		bool providerResult = TFR_SurrenderBridge.RequestSurrender(targetEntity, userEntity);

		if (providerResult)
		{
			LogTFR("External surrender provider ejecutado: " + TFR_SurrenderBridge.GetProviderName());
			return;
		}

		LogTFR("Sin provider externo de rendición. Core TFR sigue funcionando.");
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
	protected void LogTFR(string message)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR_ARRESTAR] " + message, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFRWarning(string message)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR_ARRESTAR] WARNING: " + message, LogLevel.WARNING);
	}
}
