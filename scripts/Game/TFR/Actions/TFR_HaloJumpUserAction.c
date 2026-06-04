class TFR_HaloJumpUserAction : ScriptedUserAction
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("Solicitar inserción HALO", UIWidgets.EditBox, "Action Name")]
	protected string m_sActionName;

	protected const int TFR_LOCAL_SELECTION_RETRY_COUNT = 12;
	protected const int TFR_LOCAL_SELECTION_RETRY_DELAY_MS = 150;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_bEnabled)
			return;

		if (!pUserEntity)
		{
			TFR_ShowHaloHint("HALO ERROR: usuario no válido");
			LogTFR("HALO map action blocked: user null.");
			return;
		}

		LogTFR("HALO map action pressed. Starting local selection retry flow.");
		TFR_TryBeginHaloMapSelection(pUserEntity, 0);
	}

	protected void TFR_TryBeginHaloMapSelection(IEntity userEntity, int attempt)
	{
		if (!m_bEnabled)
			return;

		if (!userEntity)
		{
			TFR_ShowHaloHint("HALO ERROR: usuario no válido");
			LogTFR("HALO map retry blocked: user null. Attempt=" + attempt.ToString());
			return;
		}

		TFR_HaloMapSelectionComponent selector = TFR_HaloMapSelectionComponent.GetLocalSelectionComponent(userEntity);

		if (!selector)
		{
			if (attempt == 0)
				TFR_ShowHaloHint("Buscando sistema HALO por mapa...");

			LogTFR("HALO map retry: no local TFR_HaloMapSelectionComponent. Attempt=" + attempt.ToString());

			if (attempt >= TFR_LOCAL_SELECTION_RETRY_COUNT)
			{
				TFR_ShowHaloHint("HALO ERROR: no se encontró el sistema de selección por mapa");
				return;
			}

			GetGame().GetCallqueue().CallLater(TFR_TryBeginHaloMapSelection, TFR_LOCAL_SELECTION_RETRY_DELAY_MS, false, userEntity, attempt + 1);
			return;
		}

		if (!selector.BeginHaloMapSelection())
		{
			if (attempt == 0)
				TFR_ShowHaloHint("Preparando selección HALO por mapa...");

			LogTFR("HALO map retry: BeginHaloMapSelection failed. Attempt=" + attempt.ToString());

			if (attempt >= TFR_LOCAL_SELECTION_RETRY_COUNT)
			{
				TFR_ShowHaloHint("HALO ERROR: no se pudo abrir la selección por mapa");
				return;
			}

			GetGame().GetCallqueue().CallLater(TFR_TryBeginHaloMapSelection, TFR_LOCAL_SELECTION_RETRY_DELAY_MS, false, userEntity, attempt + 1);
			return;
		}

		LogTFR("HALO map selection requested successfully. Attempt=" + attempt.ToString());
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (!m_bEnabled)
			return false;

		if (!user)
			return false;

		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		return CanBeShownScript(user);
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = "Solicitar inserción HALO";
		return true;
	}

	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	override bool CanBroadcastScript()
	{
		return false;
	}

	protected void TFR_ShowHaloHint(string message)
	{
		if (message == string.Empty)
			return;

		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();

		if (hintMgr)
			hintMgr.ShowCustomHint(message, "TFR HALO", 5.0);
	}

	protected void LogTFR(string msg)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR HALO ACTION] " + msg, LogLevel.NORMAL);
	}
}
