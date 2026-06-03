//------------------------------------------------------------------------------------------------
// TFR_ParachuteBridge.c
// TFR OPERATIONS / FRAMEWORK
//
// Bridge interno SIN dependencia directa de Parachute Framework.
//
// IMPORTANTE:
// - No usar ParachuteComponent aquí.
// - No usar ParachuteItemComponent aquí.
// - Este archivo debe compilar aunque Parachute Framework esté roto o no exista.
//
// OBJETIVO FRENTE 1:
// - El core TFR no depende de Parachute Framework.
// - Un addon opcional TFR_Parachute_Compat podrá registrar un provider externo.
// - Si el provider no existe, el core sigue compilando y funcionando con fallback seguro.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
class TFR_ParachuteProvider : Managed
{
	//------------------------------------------------------------------------------------------------
	string GetProviderName()
	{
		return "Unnamed";
	}

	//------------------------------------------------------------------------------------------------
	bool CanHaloJump(SCR_PlayerController playerController, SCR_ChimeraCharacter character, bool requireComponent, bool requireItem)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool RequestAutoDeploy(SCR_PlayerController playerController, SCR_ChimeraCharacter character)
	{
		return false;
	}
}

//------------------------------------------------------------------------------------------------
class TFR_ParachuteBridge
{
	protected static bool s_bDebugLogs = false;
	protected static ref TFR_ParachuteProvider s_Provider;

	//------------------------------------------------------------------------------------------------
	static void SetDebugLogs(bool state)
	{
		s_bDebugLogs = state;
	}

	//------------------------------------------------------------------------------------------------
	static void RegisterProvider(TFR_ParachuteProvider provider)
	{
		s_Provider = provider;

		if (s_Provider)
			TFR_DebugLog("Parachute provider registered: " + s_Provider.GetProviderName());
		else
			TFR_DebugLog("Parachute provider cleared through RegisterProvider(null).");
	}

	//------------------------------------------------------------------------------------------------
	static void ClearProvider(TFR_ParachuteProvider provider = null)
	{
		if (provider && s_Provider != provider)
			return;

		if (s_Provider)
			TFR_DebugLog("Parachute provider cleared: " + s_Provider.GetProviderName());

		s_Provider = null;
	}

	//------------------------------------------------------------------------------------------------
	static bool HasExternalParachuteProvider()
	{
		return s_Provider != null;
	}

	//------------------------------------------------------------------------------------------------
	static string GetProviderName()
	{
		if (!s_Provider)
			return "None";

		return s_Provider.GetProviderName();
	}

	//------------------------------------------------------------------------------------------------
	static bool CanHaloJump(SCR_PlayerController playerController, SCR_ChimeraCharacter character, bool requireComponent, bool requireItem)
	{
		if (!playerController)
			return false;

		if (!character)
			return false;

		if (character.IsDeleted())
			return false;

		if (!requireComponent && !requireItem)
			return true;

		if (!s_Provider)
		{
			TFR_DebugLog("Parachute requirements requested, but no external parachute provider is available.");
			return false;
		}

		bool ok = s_Provider.CanHaloJump(playerController, character, requireComponent, requireItem);

		if (!ok)
			TFR_DebugLog("Parachute provider rejected HALO request: " + s_Provider.GetProviderName());

		return ok;
	}

	//------------------------------------------------------------------------------------------------
	static bool RequestAutoDeploy(SCR_PlayerController playerController, SCR_ChimeraCharacter character)
	{
		if (!playerController)
			return false;

		if (!character)
			return false;

		if (character.IsDeleted())
			return false;

		if (!s_Provider)
		{
			TFR_DebugLog("Auto deploy requested, but no external parachute provider is available.");
			return false;
		}

		bool ok = s_Provider.RequestAutoDeploy(playerController, character);

		if (!ok)
			TFR_DebugLog("Parachute provider failed auto deploy request: " + s_Provider.GetProviderName());

		return ok;
	}

	//------------------------------------------------------------------------------------------------
	protected static void TFR_DebugLog(string message)
	{
		if (!s_bDebugLogs)
			return;

		Print("[TFR PARACHUTE BRIDGE] " + message, LogLevel.NORMAL);
	}
}
