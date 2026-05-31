//------------------------------------------------------------------------------------------------
// TFR_SurrenderBridge.c
// TFR OPERATIONS / FRAMEWORK
//
// Bridge interno SIN dependencia directa de ACE.
//
// IMPORTANTE:
// - No usar ACE aquí.
// - No usar RPC_ACE aquí.
// - Este archivo debe compilar aunque ACE Core / ACE Surrender estén rotos o no existan.
//
// De momento funciona como fallback seguro.
// Más adelante, si hacemos addon opcional TFR_ACE_Compat,
// ese addon podrá tener la llamada real a ACE.
//------------------------------------------------------------------------------------------------

class TFR_SurrenderBridge
{
	protected static bool s_bDebugLogs = false;

	//------------------------------------------------------------------------------------------------
	static void SetDebugLogs(bool state)
	{
		s_bDebugLogs = state;
	}

	//------------------------------------------------------------------------------------------------
	static bool HasExternalSurrenderProvider()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	static string GetProviderName()
	{
		return "None";
	}

	//------------------------------------------------------------------------------------------------
	static bool RequestSurrender(IEntity targetEntity, IEntity userEntity)
	{
		if (!targetEntity)
			return false;

		TFR_DebugLog("External surrender requested, but no external surrender provider is available.");

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static void TFR_DebugLog(string message)
	{
		if (!s_bDebugLogs)
			return;

		Print("[TFR SURRENDER BRIDGE] " + message, LogLevel.NORMAL);
	}
}
