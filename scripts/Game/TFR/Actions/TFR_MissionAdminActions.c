//------------------------------------------------------------------------------------------------
// TFR_MissionAdminActions.c
// TFR OPERATIONS
//
// Acciones de interacción para TFR_MissionAdminConsoleComponent.
//------------------------------------------------------------------------------------------------

class TFR_MissionAdminActionBase : ScriptedUserAction
{
	protected IEntity m_OwnerEntity;
	protected TFR_MissionAdminConsoleComponent m_Console;

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_OwnerEntity = pOwnerEntity;

		if (m_OwnerEntity)
			m_Console = TFR_MissionAdminConsoleComponent.Cast(m_OwnerEntity.FindComponent(TFR_MissionAdminConsoleComponent));
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (!m_Console)
			return false;

		return m_Console.TFR_CanShowAction(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_Console)
		{
			SetCannotPerformReason("Consola TFR no encontrada.");
			return false;
		}

		if (!m_Console.TFR_IsEnabled())
		{
			SetCannotPerformReason("Consola TFR desactivada.");
			return false;
		}

		if (!m_Console.TFR_CanUserAccess(user))
		{
			SetCannotPerformReason("No tienes autorización para usar esta consola.");
			return false;
		}

		return true;
	}

	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	override bool CanBroadcastScript()
	{
		return true;
	}

	protected TFR_MissionAdminConsoleComponent TFR_GetConsole(IEntity pOwnerEntity)
	{
		if (m_Console)
			return m_Console;

		if (!pOwnerEntity)
			return null;

		m_Console = TFR_MissionAdminConsoleComponent.Cast(pOwnerEntity.FindComponent(TFR_MissionAdminConsoleComponent));
		return m_Console;
	}
}

class TFR_AdminPrintMissionStateAction : TFR_MissionAdminActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "TFR · Diagnóstico de misión";
		return true;
	}

	override bool GetActionDescriptionScript(out string outName)
	{
		outName = "Imprime en servidor el estado actual del sistema de misiones TFR.";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_MissionAdminConsoleComponent console = TFR_GetConsole(pOwnerEntity);

		if (!console)
			return;

		console.TFR_AdminPrintState(pUserEntity);
	}
}

class TFR_AdminForceNextIntelAction : TFR_MissionAdminActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "TFR · Forzar siguiente intel";
		return true;
	}

	override bool GetActionDescriptionScript(out string outName)
	{
		outName = "Avanza la cadena de inteligencia y genera el siguiente contacto si procede.";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_MissionAdminConsoleComponent console = TFR_GetConsole(pOwnerEntity);

		if (!console)
			return;

		console.TFR_AdminForceNextIntel(pUserEntity);
	}
}

class TFR_AdminCompleteMissionAction : TFR_MissionAdminActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "TFR · Completar misión";
		return true;
	}

	override bool GetActionDescriptionScript(out string outName)
	{
		outName = "Marca la misión TFR activa como completada y continúa el flujo normal.";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_MissionAdminConsoleComponent console = TFR_GetConsole(pOwnerEntity);

		if (!console)
			return;

		console.TFR_AdminCompleteMission(pUserEntity);
	}
}

class TFR_AdminFailMissionAction : TFR_MissionAdminActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "TFR · Fallar misión";
		return true;
	}

	override bool GetActionDescriptionScript(out string outName)
	{
		outName = "Marca la misión TFR activa como fallida y continúa el flujo normal.";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_MissionAdminConsoleComponent console = TFR_GetConsole(pOwnerEntity);

		if (!console)
			return;

		console.TFR_AdminFailMission(pUserEntity);
	}
}

class TFR_AdminCancelAndRestartIntelAction : TFR_MissionAdminActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "TFR · Reiniciar cadena intel";
		return true;
	}

	override bool GetActionDescriptionScript(out string outName)
	{
		outName = "Cancela el estado actual, limpia entidades TFR y reinicia la cadena de inteligencia.";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_MissionAdminConsoleComponent console = TFR_GetConsole(pOwnerEntity);

		if (!console)
			return;

		console.TFR_AdminCancelAndRestartIntel(pUserEntity);
	}
}
