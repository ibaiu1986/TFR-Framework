//------------------------------------------------------------------------------------------------
// TFR_OperationsRadialGameModeComponent.c
// TFR OPERATIONS
//
// Componente de GameMode que registra el menú radial dedicado de TFR Operations.
//
// VA EN GAMEMODE.
//
// DEDICATED SERVER CHECK:
// - En servidor dedicado no registra UI porque System.IsConsoleApp() corta.
// - El antiguo BOOT SERVERCHECK queda sustituido por Debug Logs opcional.
//
// SERVER POLISH:
// - No imprime logs normales por defecto.
// - Mantiene warnings útiles si el menú no está configurado.
// - No cambia la lógica de registro del menú radial.
// - No cambia EOnFrame.
//------------------------------------------------------------------------------------------------

[EntityEditorProps(category: "TFR Operations/Radial", description: "Dedicated TFR Operations radial menu holder.")]
class TFR_OperationsRadialGameModeComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class TFR_OperationsRadialGameModeComponent : ScriptComponent
{
	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute(desc: "Dedicated TFR Operations radial menu instance.")]
	protected ref TFR_OperationsRadialMenu m_Menu;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		LogTFR("GameMode component reached. IsConsoleApp=" + System.IsConsoleApp().ToString() + " Owner=" + owner.ToString());

		if (System.IsConsoleApp())
			return;

		if (!m_Menu)
		{
			Print("[TFR Operations Radial] GameMode component: menu is not configured.", LogLevel.WARNING);
			return;
		}

		TFR_OperationsRadialMenu.SetTFROperationsRadialMenu(m_Menu);

		SetEventMask(owner, EntityEvent.FRAME);

		LogTFR("Dedicated operations radial menu registered.");
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_Menu)
		{
			ClearEventMask(owner, EntityEvent.FRAME);
			return;
		}

		m_Menu.Update(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	TFR_OperationsRadialMenu GetMenu()
	{
		return m_Menu;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (TFR_OperationsRadialMenu.GetTFROperationsRadialMenu() == m_Menu)
			TFR_OperationsRadialMenu.SetTFROperationsRadialMenu(null);

		ClearEventMask(owner, EntityEvent.FRAME);

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFR(string message)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR Operations Radial GameMode] " + message, LogLevel.NORMAL);
	}
}
