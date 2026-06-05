//------------------------------------------------------------------------------------------------
// TFR_AreaEnvironmentData.c
// TFR FRAMEWORK / TFR OPERATIONS
//
// Runtime data container for scanned TFR area environment information.
//
// No gameplay logic here.
// No loops here.
// No Workbench setup here.
//
// Used by:
// - TFR_AreaEntity
// - future Environment Scanner
// - future SpawnDirector area rules
// - future AmbientAI systems
//------------------------------------------------------------------------------------------------

class TFR_AreaEnvironmentData : Managed
{
	ref array<vector> m_aRoadPositions;
	ref array<vector> m_aRoadSidePositions;
	ref array<vector> m_aRoadIntersectionPositions;
	ref array<vector> m_aBuildingPositions;
	ref array<vector> m_aBuildingInteriorPositions;
	ref array<vector> m_aBuildingEntrancePositions;
	ref array<vector> m_aOpenGroundPositions;
	ref array<vector> m_aInfantrySafePositions;
	ref array<vector> m_aVehicleSafePositions;
	ref array<vector> m_aParkingPositions;
	ref array<vector> m_aCoverPositions;

	ref array<float> m_aRoadDirections;

	bool m_bScanned;
	int m_iLastScanTick;

	//------------------------------------------------------------------------------------------------
	void TFR_AreaEnvironmentData()
	{
		m_aRoadPositions = new array<vector>();
		m_aRoadSidePositions = new array<vector>();
		m_aRoadIntersectionPositions = new array<vector>();
		m_aBuildingPositions = new array<vector>();
		m_aBuildingInteriorPositions = new array<vector>();
		m_aBuildingEntrancePositions = new array<vector>();
		m_aOpenGroundPositions = new array<vector>();
		m_aInfantrySafePositions = new array<vector>();
		m_aVehicleSafePositions = new array<vector>();
		m_aParkingPositions = new array<vector>();
		m_aCoverPositions = new array<vector>();
		m_aRoadDirections = new array<float>();

		m_bScanned = false;
		m_iLastScanTick = 0;
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aRoadPositions.Clear();
		m_aRoadSidePositions.Clear();
		m_aRoadIntersectionPositions.Clear();
		m_aBuildingPositions.Clear();
		m_aBuildingInteriorPositions.Clear();
		m_aBuildingEntrancePositions.Clear();
		m_aOpenGroundPositions.Clear();
		m_aInfantrySafePositions.Clear();
		m_aVehicleSafePositions.Clear();
		m_aParkingPositions.Clear();
		m_aCoverPositions.Clear();
		m_aRoadDirections.Clear();

		m_bScanned = false;
		m_iLastScanTick = 0;
	}

	//------------------------------------------------------------------------------------------------
	void MarkScanned()
	{
		m_bScanned = true;
		m_iLastScanTick = System.GetTickCount();
	}

	//------------------------------------------------------------------------------------------------
	bool IsScanned()
	{
		return m_bScanned;
	}

	//------------------------------------------------------------------------------------------------
	int GetLastScanTick()
	{
		return m_iLastScanTick;
	}

	//------------------------------------------------------------------------------------------------
	int GetTotalKnownPositions()
	{
		int count = 0;

		count += m_aRoadPositions.Count();
		count += m_aRoadSidePositions.Count();
		count += m_aRoadIntersectionPositions.Count();
		count += m_aBuildingPositions.Count();
		count += m_aBuildingInteriorPositions.Count();
		count += m_aBuildingEntrancePositions.Count();
		count += m_aOpenGroundPositions.Count();
		count += m_aInfantrySafePositions.Count();
		count += m_aVehicleSafePositions.Count();
		count += m_aParkingPositions.Count();
		count += m_aCoverPositions.Count();

		return count;
	}
}
