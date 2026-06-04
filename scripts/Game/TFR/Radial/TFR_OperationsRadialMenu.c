//------------------------------------------------------------------------------------------------
// TFR_OperationsRadialMenu.c
// TFR OPERATIONS
//
// Menú radial dedicado de TFR Operations.
//
// No usa SCR_RadialMenu.GlobalRadialMenu().
// Evita conflictos con los radiales vanilla.
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class TFR_OperationsRadialMenu : SCR_RadialMenu
{
	protected static TFR_OperationsRadialMenu s_TFROperationsRadialMenu;

	//------------------------------------------------------------------------------------------------
	static void SetTFROperationsRadialMenu(TFR_OperationsRadialMenu menu)
	{
		s_TFROperationsRadialMenu = menu;
	}

	//------------------------------------------------------------------------------------------------
	static TFR_OperationsRadialMenu GetTFROperationsRadialMenu()
	{
		return s_TFROperationsRadialMenu;
	}

	//------------------------------------------------------------------------------------------------
	override void SetMenuDisplay(SCR_SelectionMenuDisplay display = null)
	{
		if (!display)
		{
			SCR_HUDManagerComponent hud = GetGame().GetHUDManager();

			if (hud)
				display = SCR_SelectionMenuDisplay.Cast(hud.FindInfoDisplay(SCR_RadialMenuDisplay));
		}

		super.SetMenuDisplay(display);

		if (m_RadialControllerInputs)
			ChangeDisplaySize(m_RadialControllerInputs.m_bUseLargeSize, m_RadialControllerInputs.m_fCustomSize);
	}

	//------------------------------------------------------------------------------------------------
	void TFR_OperationsRadialMenu()
	{
	}
}
