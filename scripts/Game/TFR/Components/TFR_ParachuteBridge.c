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
// De momento funciona como fallback seguro.
// Más adelante, si hacemos addon opcional TFR_Parachute_Compat,
// ese addon podrá tener su propia lógica con Parachute Framework.
//------------------------------------------------------------------------------------------------

class TFR_ParachuteBridge
{
	protected static bool s_bDebugLogs = false;

	//------------------------------------------------------------------------------------------------
	static void SetDebugLogs(bool state)
	{
		s_bDebugLogs = state;
	}

	//------------------------------------------------------------------------------------------------
	static bool HasExternalParachuteProvider()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	static string GetProviderName()
	{
		return "None";
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

		TFR_DebugLog("Parachute requirements requested, but no external parachute provider is available.");

		return false;
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

		TFR_DebugLog("Auto deploy requested, but no external parachute provider is available.");

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static void TFR_DebugLog(string message)
	{
		if (!s_bDebugLogs)
			return;

		Print("[TFR PARACHUTE BRIDGE] " + message, LogLevel.NORMAL);
	}
}
