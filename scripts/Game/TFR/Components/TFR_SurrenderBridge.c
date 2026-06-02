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
// OBJETIVO FRENTE 1:
// - El core TFR no depende de ACE.
// - Un addon opcional TFR_ACE_Compat podrá registrar un provider externo.
// - Si el provider no existe, el core sigue compilando y funcionando con fallback seguro.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
class TFR_SurrenderProvider : Managed
{
	//------------------------------------------------------------------------------------------------
	string GetProviderName()
	{
		return "Unnamed";
	}

	//------------------------------------------------------------------------------------------------
	bool CanProvideSurrender(IEntity targetEntity, IEntity userEntity)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool RequestSurrender(IEntity targetEntity, IEntity userEntity)
	{
		return false;
	}
}

//------------------------------------------------------------------------------------------------
class TFR_SurrenderBridge
{
	protected static bool s_bDebugLogs = false;
	protected static ref TFR_SurrenderProvider s_Provider;

	//------------------------------------------------------------------------------------------------
	static void SetDebugLogs(bool state)
	{
		s_bDebugLogs = state;
	}

	//------------------------------------------------------------------------------------------------
	static void RegisterProvider(TFR_SurrenderProvider provider)
	{
		s_Provider = provider;

		if (s_Provider)
			TFR_DebugLog("Surrender provider registered: " + s_Provider.GetProviderName());
		else
			TFR_DebugLog("Surrender provider cleared through RegisterProvider(null).");
	}

	//------------------------------------------------------------------------------------------------
	static void ClearProvider(TFR_SurrenderProvider provider = null)
	{
		if (provider && s_Provider != provider)
			return;

		if (s_Provider)
			TFR_DebugLog("Surrender provider cleared: " + s_Provider.GetProviderName());

		s_Provider = null;
	}

	//------------------------------------------------------------------------------------------------
	static bool HasExternalSurrenderProvider()
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
	static bool RequestSurrender(IEntity targetEntity, IEntity userEntity)
	{
		if (!targetEntity)
			return false;

		if (!s_Provider)
		{
			TFR_DebugLog("External surrender requested, but no external surrender provider is available.");
			return false;
		}

		if (!s_Provider.CanProvideSurrender(targetEntity, userEntity))
		{
			TFR_DebugLog("Surrender provider rejected request: " + s_Provider.GetProviderName());
			return false;
		}

		bool ok = s_Provider.RequestSurrender(targetEntity, userEntity);

		if (!ok)
			TFR_DebugLog("Surrender provider failed request: " + s_Provider.GetProviderName());

		return ok;
	}

	//------------------------------------------------------------------------------------------------
	protected static void TFR_DebugLog(string message)
	{
		if (!s_bDebugLogs)
			return;

		Print("[TFR SURRENDER BRIDGE] " + message, LogLevel.NORMAL);
	}
}
