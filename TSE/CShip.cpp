//	CShip.cpp
//
//	CShip class

#include "PreComp.h"

static CObjectClass<CShip>g_Class(OBJID_CSHIP, NULL);

#define FUEL_CHECK_CYCLE						4
#define LIFESUPPORT_FUEL_USE_PER_CYCLE			1
#define ARMOR_UPDATE_CYCLE						10
#define GATE_ANIMATION_LENGTH					30
#define PARALYSIS_ARC_COUNT						5

#define MAP_LABEL_X								10
#define MAP_LABEL_Y								(-6)

#define FUEL_GRACE_PERIOD						(30 * 30)

const Metric MAX_MANEUVER_DELAY	=				8.0;
const Metric MANEUVER_MASS_FACTOR =				0.4;
const Metric MAX_SPEED_FOR_DOCKING2 =			(0.04 * 0.04 * LIGHT_SPEED * LIGHT_SPEED);

#define MAX_DELTA								(2.0 * g_KlicksPerPixel)
#define MAX_DELTA2								(MAX_DELTA * MAX_DELTA)
#define MAX_DELTA_VEL							(g_KlicksPerPixel / 2.0)
#define MAX_DELTA_VEL2							(MAX_DELTA_VEL * MAX_DELTA_VEL)
#define MAX_DISTANCE							(400 * g_KlicksPerPixel)

const DWORD MAX_DISRUPT_TIME_BEFORE_DAMAGE =	(60 * g_TicksPerSecond);

#define FIELD_CARGO_SPACE						CONSTLIT("cargoSpace")
#define FIELD_LAUNCHER							CONSTLIT("launcher")
#define FIELD_LAUNCHER_UNID						CONSTLIT("launcherUNID")
#define FIELD_MAX_SPEED							CONSTLIT("maxSpeed")
#define FIELD_MANEUVER							CONSTLIT("maneuver")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_PRIMARY_ARMOR						CONSTLIT("primaryArmor")
#define FIELD_PRIMARY_ARMOR_UNID				CONSTLIT("primaryArmorUNID")
#define FIELD_PRIMARY_WEAPON					CONSTLIT("primaryWeapon")
#define FIELD_PRIMARY_WEAPON_RANGE				CONSTLIT("primaryWeaponRange")
#define FIELD_PRIMARY_WEAPON_RANGE_ADJ			CONSTLIT("primaryWeaponRangeAdj")
#define FIELD_PRIMARY_WEAPON_UNID				CONSTLIT("primaryWeaponUNID")
#define FIELD_SHIELD							CONSTLIT("shield")
#define FIELD_SHIELD_UNID						CONSTLIT("shieldsUNID")
#define FIELD_THRUST_TO_WEIGHT					CONSTLIT("thrustToWeight")

#define PROPERTY_ALWAYS_LEAVE_WRECK				CONSTLIT("alwaysLeaveWreck")
#define PROPERTY_AVAILABLE_DEVICE_SLOTS			CONSTLIT("availableDeviceSlots")
#define PROPERTY_AVAILABLE_NON_WEAPON_SLOTS		CONSTLIT("availableNonWeaponSlots")
#define PROPERTY_AVAILABLE_WEAPON_SLOTS			CONSTLIT("availableWeaponSlots")
#define PROPERTY_BLINDING_IMMUNE				CONSTLIT("blindingImmune")
#define PROPERTY_CARGO_SPACE					CONSTLIT("cargoSpace")
#define PROPERTY_CHARACTER						CONSTLIT("character")
#define PROPERTY_DEVICE_DAMAGE_IMMUNE			CONSTLIT("deviceDamageImmune")
#define PROPERTY_DEVICE_DISRUPT_IMMUNE			CONSTLIT("deviceDisruptImmune")
#define PROPERTY_DISINTEGRATION_IMMUNE			CONSTLIT("disintegrationImmune")
#define PROPERTY_DOCKED_AT_ID					CONSTLIT("dockedAtID")
#define PROPERTY_DOCKING_ENABLED				CONSTLIT("dockingEnabled")
#define PROPERTY_DOCKING_PORT_COUNT				CONSTLIT("dockingPortCount")
#define PROPERTY_DRIVE_POWER					CONSTLIT("drivePowerUse")
#define PROPERTY_EMP_IMMUNE						CONSTLIT("EMPImmune")
#define PROPERTY_FUEL_LEFT      				CONSTLIT("fuelLeft")
#define PROPERTY_FUEL_LEFT_EXACT				CONSTLIT("fuelLeftExact")
#define PROPERTY_HEALER_LEFT        			CONSTLIT("healerLeft")
#define PROPERTY_INTERIOR_HP					CONSTLIT("interiorHP")
#define PROPERTY_MAX_FUEL						CONSTLIT("maxFuel")
#define PROPERTY_MAX_FUEL_EXACT					CONSTLIT("maxFuelExact")
#define PROPERTY_MAX_INTERIOR_HP				CONSTLIT("maxInteriorHP")
#define PROPERTY_MAX_SPEED						CONSTLIT("maxSpeed")
#define PROPERTY_OPEN_DOCKING_PORT_COUNT		CONSTLIT("openDockingPortCount")
#define PROPERTY_OPERATING_SPEED				CONSTLIT("operatingSpeed")
#define PROPERTY_PLAYER_WINGMAN					CONSTLIT("playerWingman")
#define PROPERTY_POWER							CONSTLIT("power")
#define PROPERTY_POWER_USE						CONSTLIT("powerUse")
#define PROPERTY_RADIATION_IMMUNE				CONSTLIT("radiationImmune")
#define PROPERTY_ROTATION						CONSTLIT("rotation")
#define PROPERTY_ROTATION_SPEED					CONSTLIT("rotationSpeed")
#define PROPERTY_SELECTED_LAUNCHER				CONSTLIT("selectedLauncher")
#define PROPERTY_SELECTED_MISSILE				CONSTLIT("selectedMissile")
#define PROPERTY_SELECTED_WEAPON				CONSTLIT("selectedWeapon")
#define PROPERTY_SHATTER_IMMUNE					CONSTLIT("shatterImmune")
#define PROPERTY_THRUST							CONSTLIT("thrust")
#define PROPERTY_THRUST_TO_WEIGHT				CONSTLIT("thrustToWeight")

#define SPEED_EMERGENCY							CONSTLIT("emergency")
#define SPEED_FULL								CONSTLIT("full")
#define SPEED_HALF								CONSTLIT("half")
#define SPEED_QUARTER							CONSTLIT("quarter")

const CG32bitPixel RGB_MAP_LABEL =				CG32bitPixel(255, 217, 128);
const CG32bitPixel RGB_LRS_LABEL =				CG32bitPixel(165, 140, 83);

const Metric MAX_AUTO_TARGET_DISTANCE =			(LIGHT_SECOND * 30.0);

const int MAX_DAMAGE_OVERLAY_COUNT =			10;
const int MAX_DRIVE_DAMAGE_OVERLAY_COUNT =		3;

const int ATTACK_THRESHOLD =					90;

const int TRADE_UPDATE_FREQUENCY =				1801;		//	Interval for checking trade
const int INVENTORY_REFRESHED_PER_UPDATE =		20;			//	% of inventory refreshed on each update frequency

const DWORD CONTROLLER_STANDARDAI =				0x100000 + 8;
const DWORD CONTROLLER_FLEETSHIPAI =			0x100000 + 21;
const DWORD CONTROLLER_FERIANSHIPAI =			0x100000 + 23;
const DWORD CONTROLLER_AUTONAI =				0x100000 + 24;
const DWORD CONTROLLER_GLADIATORAI =			0x100000 + 27;
const DWORD CONTROLLER_FLEETCOMMANDAI =			0x100000 + 28;
const DWORD CONTROLLER_GAIANPROCESSORAI =		0x100000 + 29;
const DWORD CONTROLLER_ZOANTHROPEAI =			0x100000 + 31;
const DWORD CONTROLLER_PLAYERSHIP =				0x100000 + 100;

CShip::CShip (void) : CSpaceObject(&g_Class),
		m_pDocked(NULL),
		m_pController(NULL),
		m_iPowerDrain(0),
		m_iDeviceCount(0),
		m_Devices(NULL),
		m_pEncounterInfo(NULL),
		m_pTrade(NULL),
		m_dwNameFlags(0),
		m_pExitGate(NULL),
		m_pDeferredOrders(NULL)

//	CShip constructor

	{
	}

CShip::~CShip (void)

//	CShip destructor

	{
	if (m_pController)
		delete dynamic_cast<CObject *>(m_pController);

	if (m_Devices)
		delete [] m_Devices;

	if (m_pIrradiatedBy)
		delete m_pIrradiatedBy;

	if (m_pTrade)
		delete m_pTrade;
	}

bool CShip::AbsorbWeaponFire (CInstalledDevice *pWeapon)

//	AbsorbWeaponFire
//
//	Returns TRUE if another device on the ship (e.g., shields) prevent
//	the given weapon from firing

	{
	//	Check to see if shields prevent firing. If they do, they will create an
	//	approprate hit effect.

	CInstalledDevice *pShields = GetNamedDevice(devShields);
	if (pShields && pShields->GetClass()->AbsorbsWeaponFire(pShields, this, pWeapon))
		return true;

	//	Now check to see if energy fields prevent firing

	if (m_Overlays.AbsorbsWeaponFire(pWeapon))
		{
		//	We need to create our own default effect.

		CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_ShieldEffectUNID);
		if (pEffect)
			pEffect->CreateEffect(GetSystem(),
					this,
					pWeapon->GetPos(this),
					GetVel(),
					0);

		return true;
		}

	//	Otherwise, not absorbed

	return false;
	}

void CShip::AddOverlay (COverlayType *pType, int iPosAngle, int iPosRadius, int iRotation, int iLifeLeft, DWORD *retdwID)

//	AddOverlay
//
//	Adds an overlay to the ship

	{
	m_Overlays.AddField(this, pType, iPosAngle, iPosRadius, iRotation, iLifeLeft, retdwID);

	//	Recalc bonuses, etc.

	CalcBounds();
	CalcOverlayImpact();
	CalcArmorBonus();
	CalcDeviceBonus();
    CalcPerformance();

    m_pController->OnStatsChanged();
	m_pController->OnWeaponStatusChanged();
	m_pController->OnShipStatus(IShipController::statusArmorRepaired, -1);
	}

CTradingDesc *CShip::AllocTradeDescOverride (void)

//	AllocTradeDescOverride
//
//	Makes sure that we have the m_pTrade structure allocated.
//	This is an override of the trade desc in the type

	{
	if (m_pTrade == NULL)
		{
		m_pTrade = new CTradingDesc;

		//	Set the same economy type

		CTradingDesc *pBaseTrade = m_pClass->GetTradingDesc();
		if (pBaseTrade)
			{
			m_pTrade->SetEconomyType(pBaseTrade->GetEconomyType());
			m_pTrade->SetMaxCurrency(pBaseTrade->GetMaxCurrency());
			m_pTrade->SetReplenishCurrency(pBaseTrade->GetReplenishCurrency());
			}
		}

	return m_pTrade;
	}

void CShip::Behavior (SUpdateCtx &Ctx)

//	Behavior
//
//	Implements behavior

	{
	DEBUG_TRY

	if (!IsInactive() && !m_fControllerDisabled)
		{
		m_pController->Behavior(Ctx);

		//	If we're targeting the player, then the player is under attack

		CSpaceObject *pTarget;
		if (Ctx.pPlayer 
				&& (pTarget = GetTarget(CItemCtx()))
				&& Ctx.pPlayer->IsEnemy(this)
				&& (g_pUniverse->GetTicks() - GetLastFireTime()) < ATTACK_THRESHOLD
				&& (pTarget == Ctx.pPlayer || pTarget->IsPlayerEscortTarget(Ctx.pPlayer)))
			Ctx.pSystem->SetPlayerUnderAttack();
		}

	DEBUG_CATCH
	}

void CShip::CalcArmorBonus (void)

//	CalcArmorBonus
//
//	Check to see if all the armor segments for the ship are of the same class
//	Mark the m_fComplete flag appropriately for all armor segments.

	{
	int i, j;
	bool bComplete = true;

	//	Generate a list of all segments by type.

	TSortMap<DWORD, TArray<int>> SegmentsByType;
	for (i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pArmor = GetArmorSection(i);
		TArray<int> *pList = SegmentsByType.SetAt(pArmor->GetClass()->GetUNID());
		pList->Insert(i);
		}

	//	If we only have one type, then we've got a complete set.

	bool bCompleteSet = (SegmentsByType.GetCount() == 1);

	//	Loop over all armor segments and compute some values.

	m_iStealth = stealthMax;

	for (i = 0; i < SegmentsByType.GetCount(); i++)
		{
		for (j = 0; j < SegmentsByType[i].GetCount(); j++)
			{
			CInstalledArmor *pArmor = GetArmorSection(SegmentsByType[i][j]);

			//	Set armor complete

			pArmor->SetComplete(this, bCompleteSet);

			//	If this is the first armor of its type, mark it as prime. There
			//	are some functions that need to be done once per type and this
			//	helps us.

			pArmor->SetPrime(this, (j == 0));

			//	Compute stealth

			if (pArmor->GetClass()->GetStealth() < m_iStealth)
				m_iStealth = pArmor->GetClass()->GetStealth();
			}
		}

	//	Let our controller know (but only if we're fully created)

	if (IsCreated())
		m_pController->OnStatsChanged();
	}

void CShip::CalcBounds (void)

//	CalcBounds
//
//	Calculates bounds based on ship image and any effects and sets the object
//	bounds.

	{
	DEBUG_TRY

	//	Start with image bounds

	const CObjectImageArray &Image = m_pClass->GetImage();
	const RECT &rcImageRect = Image.GetImageRect();

	int cxWidth = RectWidth(rcImageRect);
	int cyHeight = RectHeight(rcImageRect);
	
	RECT rcBounds;
	rcBounds.left = -cxWidth / 2;
	rcBounds.right = rcBounds.left + cxWidth;
	rcBounds.top = -cyHeight / 2;
	rcBounds.bottom = rcBounds.top + cyHeight;

	//	Add the effect bounds

	m_Effects.AccumulateBounds(this, m_pClass->GetEffectsDesc(), GetRotation(), &rcBounds);

	//	Overlay bounds

	m_Overlays.AccumulateBounds(this, Image.GetImageViewportSize(), GetRotation(), &rcBounds);

	//	Set bounds

	SetBounds(rcBounds);

	DEBUG_CATCH
	}

void CShip::CalcDeviceBonus (void)

//	CalcDeviceBonus
//
//	Calculate the appropriate bonus for all devices
//	(particularly weapons)

	{
	DEBUG_TRY

	int i, j;

	//	Keep track of duplicate installed devices

	TSortMap<DWORD, int> DeviceTypes;

	//	Loop over all devices

	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty())
			{
            CItemCtx ItemCtx(this, &m_Devices[i]);

			//	Keep track of device types to see if we have duplicates

			bool bNewDevice;
			int *pCount = DeviceTypes.SetAt(m_Devices[i].GetClass()->GetUNID(), &bNewDevice);
			if (bNewDevice)
				*pCount = 1;
			else
				*pCount += 1;

			//	Keep an enhancement stack for this device

			CItemEnhancementStack *pEnhancements = new CItemEnhancementStack;
			TArray<CString> EnhancementIDs;

			//	Add any enhancements on the item itself

			if (!m_Devices[i].GetMods().IsEmpty())
				pEnhancements->Insert(m_Devices[i].GetMods());

			//	Add any slot bonus

			int iSlotBonus = m_Devices[i].GetSlotBonus();
			if (iSlotBonus != 0)
				pEnhancements->InsertHPBonus(iSlotBonus);

			//	Add enhancements from other devices

			for (j = 0; j < GetDeviceCount(); j++)
				if (i != j && !m_Devices[j].IsEmpty())
					{
					//	See if this device enhances us

					if (m_Devices[j].AccumulateEnhancements(this, &m_Devices[i], EnhancementIDs, pEnhancements))
						{
						//	If the device affected something, then we now know what it is

						if (IsPlayer())
							m_Devices[j].GetClass()->GetItemType()->SetKnown();
						}
					}

			//	Add enhancements from armor

			for (j = 0; j < GetArmorSectionCount(); j++)
				{
				CInstalledArmor *pArmor = GetArmorSection(j);
				if (pArmor->AccumulateEnhancements(this, &m_Devices[i], EnhancementIDs, pEnhancements))
					{
					if (IsPlayer())
						pArmor->GetClass()->GetItemType()->SetKnown();
					}
				}

			//	Deal with class specific stuff

			switch (m_Devices[i].GetCategory())
				{
				case itemcatLauncher:
				case itemcatWeapon:
					{
					//	Overlays add a bonus

					int iBonus = m_Overlays.GetWeaponBonus(&m_Devices[i], this);
					if (iBonus != 0)
						pEnhancements->InsertHPBonus(iBonus);
					break;
					}
				}

			//	Set the bonuses
			//	Note that these include any bonuses confered by item enhancements

			m_Devices[i].SetActivateDelay(pEnhancements->CalcActivateDelay(ItemCtx));

			//	Take ownership of the stack.

			m_Devices[i].SetEnhancements(pEnhancements);
			}

	//	Mark devices as duplicate (or not)

	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty())
			{
			int *pCount = DeviceTypes.GetAt(m_Devices[i].GetClass()->GetUNID());
			m_Devices[i].SetDuplicate(*pCount > 1);
			}

	//	Make sure we don't overflow fuel (in case we downgrade the reactor)

	if (!m_fOutOfFuel)
		m_rFuelLeft = Min(m_rFuelLeft, GetMaxFuel());

	DEBUG_CATCH
	}

int CShip::CalcDeviceSlotsInUse (int *retiWeaponSlots, int *retiNonWeapon) const

//	CalcDeviceSlotsInUse
//
//	Returns the number of device slots being used

	{
	int i;
	int iAll = 0;
	int iWeapons = 0;
	int iNonWeapons = 0;

	//	Count the number of slots being used up currently

	for (i = 0; i < GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = GetDevice(i);
		if (!pDevice->IsEmpty())
			{
			int iSlots = pDevice->GetClass()->GetSlotsRequired();
			iAll += iSlots;

			if (pDevice->GetCategory() == itemcatWeapon 
					|| pDevice->GetCategory() == itemcatLauncher)
				iWeapons += iSlots;
			else
				iNonWeapons += iSlots;
			}
		}

	if (retiWeaponSlots)
		*retiWeaponSlots = iWeapons;

	if (retiNonWeapon)
		*retiNonWeapon = iNonWeapons;

	return iAll;
	}

bool CShip::CalcDeviceTarget (STargetingCtx &Ctx, CItemCtx &ItemCtx, CSpaceObject **retpTarget, int *retiFireSolution)

//	CalcDeviceTarget
//
//	Compute the target for this weapon.
//
//	retpTarget is either a valid target or NULL, which means that the weapon has
//	no target (should fire straight).
//
//	retiFireSolution is either an angle or -1. If -1, it means either that the
//	weapon has no target (and should fire straight) or that we did not compute
//	a fire solution.
//
//	We return TRUE if we should fire and FALSE otherwise (automatic weapons
//	don't always fire if they have no target).

	{
	DEBUG_TRY

	CInstalledDevice *pDevice = ItemCtx.GetDevice();

	//	For primary weapons, the target is the controller target.
	//	
	//	NOTE: Selectable means that the weapon is not a secondary weapon
	//	and not a linked-fire weapon.

	if (pDevice->IsSelectable(ItemCtx))
		{
		*retpTarget = m_pController->GetTarget(ItemCtx);
		*retiFireSolution = -1;
		return true;
		}

	//	Otherwise this is a linked fire weapon or a secondary weapon.

	else
		{
		CDeviceClass *pWeapon = ItemCtx.GetDeviceClass();

		//	Get the actual options.

		DWORD dwLinkedFireOptions = pWeapon->GetLinkedFireOptions(ItemCtx);

		//	If our options is "fire always" then our target is always the same
		//	as the primary target.

		if (dwLinkedFireOptions & CDeviceClass::lkfAlways)
			{
			*retpTarget = m_pController->GetTarget(ItemCtx);
			*retiFireSolution = -1;
			return true;
			}

		//	Otherwise, we need to let our controller find a target for this weapon.

		else
			{
			m_pController->GetWeaponTarget(Ctx, ItemCtx, retpTarget, retiFireSolution);

			//	We only fire if we have a target

			return (*retpTarget != NULL);
			}
		}

	DEBUG_CATCH
	}

CSpaceObject::InstallItemResults CShip::CalcDeviceToReplace (const CItem &Item, int iSuggestedSlot, int *retiSlot)

//	CalcDeviceToReplace
//
//	Checks to see if installing the given item REQUIRES us to replace an already
//	installed device.

	{
	int i;

	//	Pre-initialize

	if (retiSlot)
		*retiSlot = -1;

	//	Get the item type

	CDeviceClass *pDevice = Item.GetType()->GetDeviceClass();
	if (pDevice == NULL)
		return insNotInstallable;

	ItemCategories iCategory = pDevice->GetCategory();

	//	See if if this is a device with an assigned slot (like a shield generator
	//	or cargo hold, etc.) and the slot is in use, then we need to replace it.

	if (IsSingletonDevice(iCategory)
			&& !IsDeviceSlotAvailable(iCategory, retiSlot))
		{
		switch (iCategory)
			{
			case itemcatLauncher:
				return insReplaceLauncher;

			case itemcatReactor:
				return insReplaceReactor;

			case itemcatShields:
				return insReplaceShields;

			case itemcatCargoHold:
				return insReplaceCargo;

			case itemcatDrive:
				return insReplaceDrive;

			default:
				ASSERT(false);
				return insNoDeviceSlotsLeft;
			}
		}

	//	If we have no limitation on slots, then we can continue.

	if (m_pClass->GetMaxDevices() == -1
			&& iSuggestedSlot == -1)
		return insOK;

	//	See how many device slots we need

	int iSlotsRequired = pDevice->GetSlotsRequired();

	//	Otherwise, check to make sure that we have enough slots.

	bool bIsWeapon = (iCategory == itemcatWeapon || iCategory == itemcatLauncher);
	bool bIsMisc = (iCategory == itemcatMiscDevice);

	//	Count the number of slots being used up currently

	int iWeapons;
	int iNonWeapons;
	int iAll = CalcDeviceSlotsInUse(&iWeapons, &iNonWeapons);

	//	See how many slots we would need to free in order to install this 
	//	device.

	int iAllSlotsNeeded = (iAll + iSlotsRequired) - m_pClass->GetMaxDevices();
	int iWeaponSlotsNeeded = (bIsWeapon
			&& (m_pClass->GetMaxWeapons() < m_pClass->GetMaxDevices())
				? ((iWeapons + iSlotsRequired) - m_pClass->GetMaxWeapons())
				: 0);
	int iNonWeaponSlotsNeeded = (!bIsWeapon
			&& (m_pClass->GetMaxNonWeapons() < m_pClass->GetMaxDevices())
				? ((iNonWeapons + iSlotsRequired) - m_pClass->GetMaxNonWeapons())
				: 0);

	//	If we have enough space, then we're done

	if (iAllSlotsNeeded <= 0
			&& iWeaponSlotsNeeded <= 0
			&& iNonWeaponSlotsNeeded <= 0
			&& iSuggestedSlot == -1)
		return insOK;

	int iSlotToReplace = -1;

	//	If we passed in a slot to replace, see if freeing that device gives us
	//	enough room.

	if (iSuggestedSlot != -1)
		{
		CInstalledDevice *pDevice = GetDevice(iSuggestedSlot);
		if (!pDevice->IsEmpty())
			{
			bool bThisIsWeapon = (pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher);
			bool bThisIsMisc = (pDevice->GetCategory() == itemcatMiscDevice);
			int iAllSlotsFreed = pDevice->GetClass()->GetSlotsRequired();
			int iWeaponSlotsFreed = (bThisIsWeapon ? iAllSlotsFreed	: 0);
			int iNonWeaponSlotsFreed = (!bThisIsWeapon ? iAllSlotsFreed : 0);

			if (iAllSlotsFreed >= iAllSlotsNeeded
					&& iWeaponSlotsFreed >= iWeaponSlotsNeeded
					&& iNonWeaponSlotsFreed >= iNonWeaponSlotsNeeded)
				iSlotToReplace = iSuggestedSlot;
			}
		}

	//	If we need more space, check to see if we replace an existing device.
	//
	//	NOTE: We only do this for non-player ship; players need to handle this
	//	manually.

	if (iSlotToReplace == -1 && !IsPlayer())
		{
		int iBestLevel;
		int iBestType;

		for (i = 0; i < GetDeviceCount(); i++)
			{
			CInstalledDevice *pDevice = GetDevice(i);
			if (!pDevice->IsEmpty())
				{
				bool bThisIsWeapon = (pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher);
				bool bThisIsMisc = (pDevice->GetCategory() == itemcatMiscDevice);
				int iAllSlotsFreed = pDevice->GetClass()->GetSlotsRequired();
				int iWeaponSlotsFreed = (bThisIsWeapon ? iAllSlotsFreed	: 0);
				int iNonWeaponSlotsFreed = (!bThisIsWeapon ? iAllSlotsFreed : 0);

				int iThisType;
				if (bThisIsMisc)
					iThisType = 3;
				else if (bThisIsWeapon)
					iThisType = 2;
				else
					iThisType = 1;

				int iThisLevel = pDevice->GetLevel();

				//	See if uninstalling this device would be enough; if not, then
				//	don't bother.

				if (iAllSlotsFreed < iAllSlotsNeeded
						|| iWeaponSlotsFreed < iWeaponSlotsNeeded
						|| iNonWeaponSlotsFreed < iNonWeaponSlotsNeeded)
					continue;

				//	See if removing this device is better than removing another one.

				if (iSlotToReplace == -1
						|| (iThisType > iBestType)
						|| (iThisType == iBestType && iThisLevel > iBestLevel))
					{
					iSlotToReplace = i;
					iBestType = iThisType;
					iBestLevel = iThisLevel;
					}
				}
			}
		}

	//	If we found it, then OK. Otherwise, we cannot install

	if (iSlotToReplace != -1)
		{
		if (retiSlot)
			*retiSlot = iSlotToReplace;

		return insReplaceOther;
		}
	else if (iWeaponSlotsNeeded > 0)
		return insNoWeaponSlotsLeft;
	else if (iNonWeaponSlotsNeeded > 0)
		return insNoNonWeaponSlotsLeft;
	else
		return insNoDeviceSlotsLeft;
	}

DWORD CShip::CalcEffectsMask (void)

//	CalcEffectsMask
//
//	Returns a bit mask of all effects to paint.

	{
	DWORD dwEffects = 0;
	if (m_pController->GetThrust() && !IsParalyzed())
		dwEffects |= CObjectEffectDesc::effectThrustMain;
	
	switch (m_Rotation.GetLastManeuver())
		{
		case RotateLeft:
			dwEffects |= CObjectEffectDesc::effectThrustLeft;
			break;

		case RotateRight:
			dwEffects |= CObjectEffectDesc::effectThrustRight;
			break;
		}

	return dwEffects;
	}

int CShip::CalcMaxCargoSpace (void) const

//	CalcMaxCargoSpace
//
//	Returns the max cargo space for this ship (based on class and
//	additional devices)

	{
    return m_Perf.GetCargoDesc().GetCargoSpace();
	}

void CShip::CalcOverlayImpact (void)

//	CalcOverlayImpact
//
//	Calculates the impact of overlays on the ship. This should be called 
//	whenever the set of overlays changes.

	{
	DEBUG_TRY

	COverlayList::SImpactDesc Impact;
	m_Overlays.GetImpact(this, &Impact);

	//	Update our cache

	m_fDisarmedByOverlay = Impact.bDisarm;
	m_fParalyzedByOverlay = Impact.bParalyze;
	m_fSpinningByOverlay = Impact.bSpin;
	m_fDragByOverlay = (Impact.rDrag < 1.0);

	DEBUG_CATCH
	}

void CShip::CalcPerformance (void)

//  CalcPerformance
//
//  Computes ship performance parameters by accumulating all devices, 
//  enhancements, etc.

    {
    int i;

    //  Remember current settings so we can detect if something changed.

    int iOldThrust = m_Perf.GetDriveDesc().GetThrust();
    Metric rOldMaxSpeed = m_Perf.GetDriveDesc().GetMaxSpeed();

    //  We generate a context block and accumulate performance stats from the
    //  class, armor, devices, etc.
    //
    //  These fields are context for the ship that we're computing.

    SShipPerformanceCtx Ctx;
    Ctx.pShip = this;
    Ctx.bDriveDamaged = IsMainDriveDamaged();

	if (m_fEmergencySpeed)
		Ctx.rOperatingSpeedAdj = 1.5;
	else if (m_fHalfSpeed)
		Ctx.rOperatingSpeedAdj = 0.5;
	else if (m_fQuarterSpeed)
		Ctx.rOperatingSpeedAdj = 0.25;
	else
		Ctx.rOperatingSpeedAdj = 1.0;

    //  Start with parameters from the class

    m_pClass->InitPerformance(Ctx);

    //  Accumulate settings from armor

	for (i = 0; i < GetArmorSectionCount(); i++)
		{
        CItemCtx ItemCtx(this, GetArmorSection(i));
        ItemCtx.GetArmor()->AccumulatePerformance(ItemCtx, Ctx);
		}

    //  Accumulate settings from devices

    for (i = 0; i < GetDeviceCount(); i++)
        if (!GetDevice(i)->IsEmpty())
            {
            CItemCtx ItemCtx(this, GetDevice(i));
            ItemCtx.GetDevice()->AccumulatePerformance(ItemCtx, Ctx);
            }

    //  If we're tracking mass, adjust rotation descriptor to compensate for
    //  ship mass.

    if (m_fTrackMass)
        Ctx.RotationDesc.AdjForShipMass(m_pClass->GetHullMass(), GetItemMass());

    //  Now apply the performance parameters to the descriptor

    m_Perf.Init(Ctx);

    //  This recalcs maneuvering

    m_fRecalcRotationAccel = false;

	//	If we upgraded, then we reinitialize the effects

	if (m_Perf.GetDriveDesc().GetThrust() != iOldThrust
            || m_Perf.GetDriveDesc().GetMaxSpeed() != rOldMaxSpeed)
		m_pClass->InitEffects(this, &m_Effects);
    }

void CShip::CalcReactorStats (void)

//	CalcReactorStats
//
//	Computes power consumption and generation

	{
	int i;

	//	Calculate power usage

	m_iPowerDrain = 0;

	//	We always consume some power for life-support

	m_iPowerDrain += 5;

	//	If we're thrusting, then we consume power

	if (!IsParalyzed() && m_pController->GetThrust())
		m_iPowerDrain += m_Perf.GetDriveDesc().GetPowerUse();

	//	Compute power drain of all devices

	for (i = 0; i < GetDeviceCount(); i++)
		m_iPowerDrain += m_Devices[i].CalcPowerUsed(this);

	//	Compute power drain from armor

	for (i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pArmor = GetArmorSection(i);
		m_iPowerDrain += pArmor->GetClass()->CalcPowerUsed(pArmor);
		}
	}

bool CShip::CanAttack (void) const

//	CanAttack
//
//	TRUE if the ship can attack. NOTE: The semantic of CanAttack is whether or not
//	this is the kind of object that seems threatening. Even if the ship cannot actually
//	do damage (perhaps because it has no weapons) we still mark it as CanAttack.

	{
	return !IsIntangible();
	}

bool CShip::CanInstallItem (const CItem &Item, int iSlot, InstallItemResults *retiResult, CString *retsResult, CItem *retItemToReplace)

//	CanInstallItem
//
//	Returns TRUE if we can install the given item.

	{
	bool bCanInstall = false;
	InstallItemResults iResult = insOK;
	CString sResult;
	CItem ItemToReplace;

	//	If this is an armor item, see if we can install it.

	if (Item.IsArmor())
		{
		int iMaxArmor = m_pClass->GetMaxArmorMass();

		//	See if we are compatible

		if (!Item.MatchesCriteria(m_pClass->GetArmorCriteria()))
			iResult = insNotCompatible;

		//	Ask the object if we can install this item

		else if (!FireCanInstallItem(Item, iSlot, &sResult))
			iResult = insCannotInstall;

		//	See if the armor is too heavy

		else if (iMaxArmor && Item.GetMassKg() > iMaxArmor)
			iResult = insArmorTooHeavy;

		//	Fire CanBeInstalled to check for custom conditions

		else if (!Item.FireCanBeInstalled(this, &sResult))
			iResult = insCannotInstall;

		//	Otherwise, we're OK

		else
			{
			bCanInstall = true;
			iResult = insOK;
			}
		}

	//	If this is a device item, see if we can install it.

	else if (Item.IsDevice())
		{
        CItemCtx ItemCtx(&Item);

		//	Get the item type

		CDeviceClass *pDevice = Item.GetType()->GetDeviceClass();
		ItemCategories iCategory = pDevice->GetCategory();

		//	See if we're compatible

		if (!Item.MatchesCriteria(m_pClass->GetDeviceCriteria()))
			iResult = insNotCompatible;

		//	Ask the object if we can install this item

		else if (!FireCanInstallItem(Item, -1, &sResult))
			iResult = insCannotInstall;

		//	Fire CanBeInstalled to check for custom conditions

		else if (!Item.FireCanBeInstalled(this, &sResult))
			iResult = insCannotInstall;

		//	See if the ship's engine core is powerful enough

		else if (GetMaxPower() > 0
				&& pDevice->GetPowerRating(ItemCtx) > GetMaxPower())
			iResult = insReactorTooWeak;

		//	If this is a reactor, then see if the ship class can support it

		else if (iCategory == itemcatReactor
				&& m_pClass->GetMaxReactorPower() > 0
				&& pDevice->GetPowerRating(ItemCtx) > m_pClass->GetMaxReactorPower())
			iResult = insReactorIncompatible;

		//	See if we have enough device slots to install or if we have to
		//	replace an existing device.

		else
			{
			int iRecommendedSlot;
			iResult = CalcDeviceToReplace(Item, iSlot, &iRecommendedSlot);
			switch (iResult)
				{
				case insOK:
					//	We use whatever slot we passed in.
					bCanInstall = true;
					break;

				case insReplaceCargo:
				case insReplaceDrive:
				case insReplaceLauncher:
				case insReplaceOther:
				case insReplaceReactor:
				case insReplaceShields:
					//	If we're replacing a singleton device, then we always use the
					//	recommended slot, regardless of what the caller pass in.

					iSlot = iRecommendedSlot;
					bCanInstall = true;
					break;

				default:
					iSlot = -1;
					bCanInstall = false;
				}

			if (iSlot != -1)
				{
				CItemListManipulator ItemList(GetItemList());
				SetCursorAtDevice(ItemList, iSlot);
				ItemToReplace = ItemList.GetItemAtCursor();

				//	See if the item can be removed. If not, then error

				if (bCanInstall 
						&& (!ItemList.IsCursorValid()
							|| CanRemoveDevice(ItemToReplace, &sResult) != remOK))
					{
					bCanInstall = false;
					iResult = insCannotInstall;
					}
				}
			}
		}

	//	Otherwise, item is not installable.

	else
		{
		bCanInstall = false;
		iResult = insNotInstallable;
		}

	//	Done

	if (retiResult)
		*retiResult = iResult;

	if (retsResult)
		*retsResult = sResult;

	if (retItemToReplace)
		*retItemToReplace = ItemToReplace;

	return bCanInstall;
	}

CShip::RemoveDeviceStatus CShip::CanRemoveDevice (const CItem &Item, CString *retsResult)

//	CanRemoveDevice
//
//	Returns whether or not the device can be removed

	{
	if (!Item.IsInstalled() || !Item.GetType()->IsDevice())
		return remNotInstalled;

	CDeviceClass *pDevice = Item.GetType()->GetDeviceClass();
	if (pDevice == NULL)
		return remNotInstalled;

	//	Ask the item if it can be removed

	if (!Item.FireCanBeUninstalled(this, retsResult))
		return remCannotRemove;

	//	Check to see if we are allowed to remove the item

	else if (!FireCanRemoveItem(Item, -1, retsResult))
		return remCannotRemove;

	//	Check for special device cases

	switch (pDevice->GetCategory())
		{
		case itemcatCargoHold:
			{
			//	Compute how much cargo space we need to be able to hold

			OnComponentChanged(comCargo);
			Metric rCargoSpace = GetCargoMass() + Item.GetMass();

			//	If this is larger than the ship class max, then we cannot remove

			if (rCargoSpace > (Metric)m_pClass->GetHullCargoSpace())
				{
				*retsResult = CONSTLIT("Your ship has too much cargo to be able to replace the cargo hold.");
				return remTooMuchCargo;
				}

			break;
			}
		}

	//	OK

	return remOK;
	}

CurrencyValue CShip::ChargeMoney (DWORD dwEconomyUNID, CurrencyValue iValue)

//	ChargeMoney
//
//	Charge the amount out of the station's balance
//	Returns the remaining balance (or -1 if there is not enough)

	{
	ASSERT(m_pController);

	CCurrencyBlock *pMoney = m_pController->GetCurrencyBlock();
	if (pMoney && pMoney->GetCredits(dwEconomyUNID) >= iValue && iValue >= 0)
		return pMoney->IncCredits(dwEconomyUNID, -iValue);
	else
		return -1;
	}

void CShip::ClearAllTriggered (void)

//	ClearAllTriggered
//
//	Clear the triggered flag on all devices

	{
	for (int i = 0; i < GetDeviceCount(); i++)
		GetDevice(i)->SetTriggered(false);
	}

void CShip::ClearBlindness (bool bNoMessage)

//	ClearBlindness
//
//	Ship is no longer blind

	{
	SetAbility(ablShortRangeScanner, ablRepair, -1, (bNoMessage ? ablOptionNoMessage : 0));
	}

void CShip::ClearDisarmed (void)

//	ClearDisarmed
//
//	Ship is no longer disarmed

	{
	m_iDisarmedTimer = 0;
	}

void CShip::ClearLRSBlindness (void)

//	ClearLRSBlindness
//
//	Ship is no longer LRS blind

	{
	m_iLRSBlindnessTimer = 0;
	}

void CShip::ClearParalyzed (void)

//	ClearParalyzed
//
//	Ship is no longer paralyzed

	{
	m_iParalysisTimer = 0;
	}

void CShip::ConsumeFuel (Metric rFuel, CReactorDesc::EFuelUseTypes iUse)

//	ConsumeFuel
//
//	Consumes some amount of fuel

	{
    if (m_fTrackFuel 
			&& !IsOutOfPower()
			&& m_Perf.GetReactorDesc().UsesFuel())
        {
        Metric rConsumed = Min(m_rFuelLeft, rFuel);

        m_rFuelLeft -= rConsumed;
        m_pController->OnFuelConsumed(rConsumed, iUse);
        }
	}

ALERROR CShip::CreateFromClass (CSystem *pSystem, 
								CShipClass *pClass,
								IShipController *pController,
								CDesignType *pOverride,
								CSovereign *pSovereign,
								const CVector &vPos, 
								const CVector &vVel, 
								int iRotation, 
								SShipGeneratorCtx *pCtx,
								CShip **retpShip)

//	CreateFromClass
//
//	Creates a new ship based on the class.
//
//	pController is owned by the ship if this function is successful.

	{
	DEBUG_TRY

	ALERROR error;
    CString sError;
	CShip *pShip;
	int i;
	CUniverse *pUniv = pSystem->GetUniverse();

	ASSERT(pClass);
	ASSERT(pController);

	pShip = new CShip;
	if (pShip == NULL)
		return ERR_MEMORY;

	//	Initialize

	pShip->Place(vPos, vVel);
	pShip->SetObjectDestructionHook();
	pShip->SetCanBounce();

	if (pClass->GetAISettings().NoFriendlyFire())
		pShip->SetNoFriendlyFire();

	pShip->m_pClass = pClass;
	pShip->m_pController = pController;
	pController->SetShipToControl(pShip);
	pShip->m_pSovereign = pSovereign;
	pShip->m_sName = pClass->GenerateShipName(&pShip->m_dwNameFlags);
	pShip->m_Rotation.Init(pClass->GetRotationDesc(), iRotation);
	pShip->m_iFireDelay = 0;
	pShip->m_iMissileFireDelay = 0;
	pShip->m_iReactorGraceTimer = 0;
	pShip->m_iContaminationTimer = 0;
	pShip->m_iBlindnessTimer = 0;
	pShip->m_iLRSBlindnessTimer = 0;
	pShip->m_iParalysisTimer = 0;
	pShip->m_iDriveDamagedTimer = 0;
	pShip->m_iDisarmedTimer = 0;
	pShip->m_iExitGateTimer = 0;
	pShip->m_pExitGate = NULL;
	pShip->m_pIrradiatedBy = NULL;
	pShip->m_iLastFireTime = 0;
	pShip->m_iLastHitTime = 0;
	pShip->m_rItemMass = 0.0;
	pShip->m_rCargoMass = 0.0;
	pShip->m_pTrade = NULL;

	pShip->m_fOutOfFuel = false;
	pShip->m_fTrackFuel = false;
	pShip->m_fTrackMass = false;
	pShip->m_fRadioactive = false;
	pShip->m_fHasAutopilot = false;
	pShip->m_fDestroyInGate = false;
	pShip->m_fHalfSpeed = false;
	pShip->m_fHasTargetingComputer = false;
	pShip->m_fSRSEnhanced = false;
	pShip->m_fDeviceDisrupted = false;
	pShip->m_fKnown = false;
	pShip->m_fHiddenByNebula = false;
	pShip->m_fIdentified = false;
	pShip->m_fManualSuspended = false;
	pShip->m_fGalacticMap = false;
	pShip->m_fRecalcItemMass = true;
	pShip->m_fRecalcRotationAccel = false;
	pShip->m_fDockingDisabled = false;
	pShip->m_fControllerDisabled = false;
	pShip->m_fParalyzedByOverlay = false;
	pShip->m_fDisarmedByOverlay = false;
	pShip->m_fSpinningByOverlay = false;
	pShip->m_fDragByOverlay = false;
	pShip->m_fAlwaysLeaveWreck = false;
	pShip->m_fOutOfPower = false;
	pShip->m_fFriendlyFireLock = false;
	pShip->m_fEmergencySpeed = false;
	pShip->m_fQuarterSpeed = false;

	//	Shouldn't be able to hit a virtual ship

	if (pClass->IsVirtual())
		pShip->SetCannotBeHit();

	//	Add some built-in stuff for player ships. These can be overridden by
	//	the <Equipment> element.

	const CPlayerSettings *pPlayerSettings = pClass->GetPlayerSettings();
	if (pPlayerSettings)
		{
		pShip->SetAbility(ablAutopilot, ablInstall, -1, 0);
		pShip->SetAbility(ablGalacticMap, ablInstall, -1, 0);
		}

	//	Create items

	if (error = pShip->CreateRandomItems(pClass->GetRandomItemTable(), pSystem))
		return error;

	//	Devices

	for (i = 0; i < devNamesCount; i++)
		pShip->m_NamedDevices[i] = -1;

	CDeviceDescList Devices;
	pClass->GenerateDevices(pSystem->GetLevel(), Devices);

	pShip->m_iDeviceCount = Max(Devices.GetCount(), pClass->GetMaxDevices());
	pShip->m_Devices = new CInstalledDevice [pShip->m_iDeviceCount];

	//	See if we need to call an OnInstall event. For the player
	//	we always do it because we track install statistics.
	//	For other ships, we only call it if an item has OnInstall code

	bool bInstallItemEvent = pController->IsPlayer();

	CItemListManipulator ShipItems(pShip->GetItemList());
	for (i = 0; i < Devices.GetCount(); i++)
		{
		const SDeviceDesc &NewDevice = Devices.GetDeviceDesc(i);

		//	Add item

		ShipItems.AddItem(NewDevice.Item);

		//	Install the device

		pShip->m_Devices[i].InitFromDesc(NewDevice);
		pShip->m_Devices[i].Install(pShip, ShipItems, i, true);

		//	Remember if we need to call OnInstall for this item
		//	All devices need this.

		bInstallItemEvent = true;

		//	Assign to named devices

		switch (pShip->m_Devices[i].GetCategory())
			{
			case itemcatWeapon:
				if (pShip->m_NamedDevices[devPrimaryWeapon] == -1
						&& pShip->m_Devices[i].IsSelectable(CItemCtx(pShip, &pShip->m_Devices[i])))
					pShip->m_NamedDevices[devPrimaryWeapon] = i;
				break;

			case itemcatLauncher:
				if (pShip->m_NamedDevices[devMissileWeapon] == -1
						&& pShip->m_Devices[i].IsSelectable(CItemCtx(pShip, &pShip->m_Devices[i])))
					pShip->m_NamedDevices[devMissileWeapon] = i;
				break;

			case itemcatShields:
				if (pShip->m_NamedDevices[devShields] == -1)
					pShip->m_NamedDevices[devShields] = i;
				break;

			case itemcatDrive:
				if (pShip->m_NamedDevices[devDrive] == -1)
					pShip->m_NamedDevices[devDrive] = i;
				break;

			case itemcatCargoHold:
				if (pShip->m_NamedDevices[devCargo] == -1)
					pShip->m_NamedDevices[devCargo] = i;
				break;

			case itemcatReactor:
				if (pShip->m_NamedDevices[devReactor] == -1)
					pShip->m_NamedDevices[devReactor] = i;
				break;
			}

		//	Add extra items (we do this at the end because this will
		//	modify the cursor)

		ShipItems.AddItems(NewDevice.ExtraItems);
		}

	//	Install equipment

	pClass->InstallEquipment(pShip);

	//	Add the ship to the system (but don't add it to the universe
	//	list--we will add it later in FinishCreation).

	if (error = pShip->AddToSystem(pSystem, true))
		{
		delete pShip;
		return error;
		}

	//	Initialize the armor from the class

    pShip->m_Armor.Install(pShip, pClass->GetArmorDesc(), true);

	//	Remember if we need to call OnInstall for this item
	//	(All armor needs this)

	bInstallItemEvent = true;

	pShip->OnComponentChanged(comCargo);
	pShip->CalcArmorBonus();
	pShip->CalcDeviceBonus();
    pShip->CalcPerformance();

	//	Initialize fuel now that we know our maximum

	pShip->m_rFuelLeft = pShip->GetMaxFuel();

	//	Set the bounds for this object

	const CObjectImageArray &Image = pShip->m_pClass->GetImage();
	pShip->SetBounds(Image.GetImageRect());

	//	Initialize docking ports (if any)

	pShip->m_DockingPorts.InitPorts(pShip, pShip->m_pClass->GetDockingPortPositions());

	//	If we're a ship with 0 thrust then it means that we're a turret, so we 
	//	set the anchor flag so that we don't get pulled by gravity.

	if (!pShip->CanThrust())
		pShip->SetManualAnchor();

	//	If any of our items need an OnInstall call, raise the
	//	event now.

	if (bInstallItemEvent)
		{
		CItemListManipulator AllItems(pShip->GetItemList());
		while (AllItems.MoveCursorForward())
			{
			const CItem &Item = AllItems.GetItemAtCursor();
			if (Item.IsInstalled())
				{
				if (Item.GetType()->GetCategory() == itemcatArmor)
					{
					CInstalledArmor *pArmor = pShip->FindArmor(Item);
					if (pArmor)
						pArmor->FinishInstall(pShip);
					}
				else
					{
					CInstalledDevice *pDevice = pShip->FindDevice(Item);
					if (pDevice)
						pDevice->FinishInstall(pShip);
					}
				}
			}
		}

	//	Set override, just before creation.
	//	
	//	NOTE: We need to call SetOverride even if we have NULL for a handler 
	//	because it also sets event flags (SetEventFlags).

	pShip->SetOverride(pOverride);

	//	Ship interior

	pShip->m_Interior.Init(pClass->GetInteriorDesc());

    //  Initialize the data from the class

    pShip->SetDataFromDataBlock(pClass->GetInitialData());

	//	If we have a generator context, set some more settings

	if (pCtx)
		{
		if (!pCtx->sName.IsBlank())
			pShip->SetName(pCtx->sName, pCtx->dwNameFlags);

		//	Add items to the ship

		if (pCtx->pItems)
			{
            if (pShip->CreateRandomItems(pCtx->pItems, pSystem) != NOERROR)
                pShip->ReportCreateError(CONSTLIT("Unable to create items"));
			}

		//	set any initial data

		if (!pCtx->InitialData.IsEmpty())
			pShip->SetDataFromDataBlock(pCtx->InitialData);
		}

	//	Fire OnCreate

	if (!pSystem->IsCreationInProgress())
		pShip->FinishCreation(pCtx);

	//	If creation is in progress and we have a generator context then we need
	//	to keep it aside for later

	else
		{
		if (pCtx)
			{
			ASSERT(pShip->m_pDeferredOrders == NULL);
			pShip->m_pDeferredOrders = new SShipGeneratorCtx;
			*pShip->m_pDeferredOrders = *pCtx;

			if (pCtx->pOnCreate)
				pShip->m_pDeferredOrders->pOnCreate = pCtx->pOnCreate->Reference();
			}

		//	Make sure we get an OnSystemCreated (and that we get it in the 
		//	proper order relative to our station).

		pSystem->RegisterForOnSystemCreated(pShip);
		}

	//	Done

	if (retpShip)
		*retpShip = pShip;

	return NOERROR;

	DEBUG_CATCH
	}

CurrencyValue CShip::CreditMoney (DWORD dwEconomyUNID, CurrencyValue iValue)

//	CreditMoney
//
//	Credits the amount to the station's balance. Return the new balance.

	{
	ASSERT(m_pController);

	CCurrencyBlock *pMoney = m_pController->GetCurrencyBlock();
	if (pMoney == NULL)
		return -1;

	if (iValue > 0)
		return pMoney->IncCredits(dwEconomyUNID, iValue);
	else
		return pMoney->GetCredits(dwEconomyUNID);
	}

void CShip::DamageCargo (SDamageCtx &Ctx)

//	DamageCargo
//
//	Damages the cargo hold

	{
	//	Create an overlay to spew smoke

	DWORD dwDamageUNID = (mathRandom(1, 100) <= 20 ? UNID_DAMAGED_SITE_SMALL : UNID_DEPREZ_SITE_SMALL);
	COverlayType *pOverlayType = g_pUniverse->FindOverlayType(dwDamageUNID);
	if (pOverlayType
			&& m_Overlays.GetCountOfType(pOverlayType) < MAX_DAMAGE_OVERLAY_COUNT)
		{
		//	Convert from a hit position to an overlay pos

		int iPosAngle;
		int iPosRadius;
		CalcOverlayPos(pOverlayType, Ctx.vHitPos, &iPosAngle, &iPosRadius);

		//	Add the overlay (lasts 10-30 seconds real time)

		AddOverlay(pOverlayType, iPosAngle, iPosRadius, iPosAngle, mathRandom(10 * g_TicksPerSecond, 30 * g_TicksPerSecond));
		}
	}

void CShip::DamageDevice (CInstalledDevice *pDevice, SDamageCtx &Ctx)

//	DamageDevice
//
//	Damages the given device

	{
	//	Damage the device

	DamageItem(pDevice);
	OnDeviceStatus(pDevice, CDeviceClass::failDeviceHitByDamage);

	//	Create an overlay to spew smoke

	COverlayType *pOverlayType = g_pUniverse->FindOverlayType(UNID_DAMAGED_SITE_SMALL);
	if (pOverlayType)
		CSpaceObject::AddOverlay(pOverlayType, Ctx.vHitPos, 180, 9000);
	}

void CShip::DamageDrive (SDamageCtx &Ctx)

//	DamageDrive
//
//	Damages the main drive.

	{
	//	Look for a drive device. If we have it, and it is undamaged, then it is
	//	damaged.

	CInstalledDevice *pDrive = GetNamedDevice(devDrive);
	if (pDrive && !pDrive->IsDamaged())
		{
		DamageDevice(pDrive, Ctx);
		return;
		}

	//	Otherwise, we damage the built-in drive.

	if (m_iDriveDamagedTimer != -1)
		{
		int iDamageTime = mathRandom(1800, 3600);

		//	Increment timer

		m_iDriveDamagedTimer = Min(m_iDriveDamagedTimer + iDamageTime, MAX_SHORT);

		//	Create an overlay to spew smoke

		COverlayType *pOverlayType = g_pUniverse->FindOverlayType(UNID_DAMAGED_SITE_MEDIUM);
		if (pOverlayType
				&& m_Overlays.GetCountOfType(pOverlayType) < MAX_DRIVE_DAMAGE_OVERLAY_COUNT)
			CSpaceObject::AddOverlay(pOverlayType, Ctx.vHitPos, 180, iDamageTime);

		//	Update performance (which also updates effects).

        CalcPerformance();
		}
	}

int CShip::DamageArmor (int iSect, DamageDesc &Damage)

//	DamageArmor
//
//	Damage armor. Returns the number of hits points of damage caused.

	{
	CInstalledArmor *pArmor = GetArmorSection(iSect);
	CItemCtx ItemCtx(this, pArmor);

	SDamageCtx DamageCtx;
	DamageCtx.Damage = Damage;
	DamageCtx.iDamage = Damage.RollDamage();

	//	Adjust the damage for the armor

	pArmor->GetClass()->CalcAdjustedDamage(ItemCtx, DamageCtx);
	if (DamageCtx.iDamage == 0)
		return 0;

	//	Armor takes damage

	int iDamage = Min(DamageCtx.iDamage, pArmor->GetHitPoints());
	if (iDamage == 0)
		return 0;

	pArmor->IncHitPoints(-iDamage);

	//	Tell the controller that we were damaged

	m_pController->OnDamaged(CDamageSource(), pArmor, Damage, iDamage);

	return iDamage;
	}

void CShip::DamageExternalDevice (int iDev, SDamageCtx &Ctx)

//	DamageExternalDevice
//
//	See if an external device got hit

	{
	CInstalledDevice *pDevice = GetDevice(iDev);

	//	If we're already damaged, then nothing more can happen

	if (pDevice->IsEmpty() || pDevice->IsDamaged())
		return;

	//	If the device gets hit, see if it gets damaged

	int iLevel = pDevice->GetLevel();
	int iMaxLevel = 0;
	int iChanceOfDamage = Ctx.iDamage * ((26 - iLevel) * 4) / 100;

	switch (Ctx.Damage.GetDamageType())
		{
		case damageLaser:
		case damageKinetic:
			iMaxLevel = 6;
			break;

		case damageParticle:
		case damageBlast:
			iMaxLevel = 9;
			break;

		case damageIonRadiation:
			iMaxLevel = 12;
			iChanceOfDamage = iChanceOfDamage * 120 / 100;
			break;

		case damageThermonuclear:
			iMaxLevel = 12;
			break;

		case damagePositron:
		case damagePlasma:
			iMaxLevel = 15;
			break;

		case damageAntiMatter:
		case damageNano:
			iMaxLevel = 18;
			break;

		case damageGravitonBeam:
			iMaxLevel = 21;
			iChanceOfDamage = iChanceOfDamage * 75 / 100;
			break;

		case damageSingularity:
			iMaxLevel = 21;
			break;

		case damageDarkAcid:
		case damageDarkSteel:
			iMaxLevel = 24;
			break;

		default:
			iMaxLevel = 27;
		}

	//	If the device is too high-level for the damage type, then nothing
	//	happens

	if (iLevel > iMaxLevel)
		return;

	//	Calculate the chance of damaging the device based on the damage
	//	and the level of the device

	if (mathRandom(1, 100) > iChanceOfDamage)
		return;

	//	Damage the device

	DamageItem(pDevice);
	OnDeviceStatus(pDevice, CDeviceClass::failDeviceHitByDamage);
	}

void CShip::DeactivateShields (void)

//	DeactivateShields
//
//	Lowers shields because some other item on the ship is
//	incompatible with shields (e.g., particular kinds of
//	armor.

	{
	int iShieldDev = m_NamedDevices[devShields];
	if (iShieldDev != -1 && m_Devices[iShieldDev].IsEnabled())
		EnableDevice(iShieldDev, false);
	}

CString CShip::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Return info that might be useful in a crash.

	{
	int i;
	CString sResult;

	sResult.Append(strPatternSubst(CONSTLIT("m_pDocked: %s\r\n"), CSpaceObject::DebugDescribe(m_pDocked)));
	sResult.Append(strPatternSubst(CONSTLIT("m_pExitGate: %s\r\n"), CSpaceObject::DebugDescribe(m_pExitGate)));

	for (i = 0; i < m_DockingPorts.GetPortCount(this); i++)
		{
		CSpaceObject *pDockedObj = m_DockingPorts.GetPortObj(this, i);
		if (pDockedObj)
			sResult.Append(strPatternSubst(CONSTLIT("m_DockingPorts[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(pDockedObj)));
		else if (!m_DockingPorts.IsPortEmpty(this, i))
			sResult.Append(strPatternSubst(CONSTLIT("m_DockingPorts[%d]: Not empty, but NULL object!\r\n"), i));
		}

	try
		{
		sResult.Append(m_pController->DebugCrashInfo());
		}
	catch (...)
		{
		sResult.Append(strPatternSubst(CONSTLIT("Invalid controller: %x\r\n"), (DWORD)m_pController));
		}

	sResult.Append(m_Overlays.DebugCrashInfo());

	return sResult;
	}

void CShip::Decontaminate (void)

//	Decontaminate
//
//	Decontaminates the ship

	{
	if (m_fRadioactive)
		{
		if (m_pIrradiatedBy)
			{
			delete m_pIrradiatedBy;
			m_pIrradiatedBy = NULL;
			}

		m_iContaminationTimer = 0;
		m_fRadioactive = false;
		m_pController->OnShipStatus(IShipController::statusRadiationCleared);
		}
	}

void CShip::DepleteShields (void)

//	DepleteShields
//
//	Deplete shields to 0

	{
	CInstalledDevice *pShields = GetNamedDevice(devShields);
	if (pShields)
		pShields->Deplete(this);
	}

void CShip::DisableAllDevices (void)

//	DisableAllDevices
//
//	Disable all devices that can be disabled.

	{
	int i;

    for (i = 0; i < GetDeviceCount(); i++)
        {
        if (!m_Devices[i].IsEmpty()
            && m_Devices[i].IsEnabled()
            && m_Devices[i].CanBeDisabled(CItemCtx(this, &m_Devices[i])))
            {
            EnableDevice(i, false);
            }
        }
	}

void CShip::DisableDevice (CInstalledDevice *pDevice)

//	DisableDevice
//
//	Disable the given device

	{
	int iDev = FindDeviceIndex(pDevice);
	if (iDev != -1)
		EnableDevice(iDev, false);
	}

void CShip::EnableDevice (int iDev, bool bEnable, bool bSilent)

//	EnableDevice
//
//	Enable or disable a device

	{
	ASSERT(iDev >= 0 && iDev < GetDeviceCount());
	if (iDev < 0 || iDev >= GetDeviceCount())
		return;

	if (!m_Devices[iDev].SetEnabled(this, bEnable))
		return;

	//	Recalc bonuses, etc.

	CalcDeviceBonus();
    CalcPerformance();

    m_pController->OnStatsChanged();
	m_pController->OnWeaponStatusChanged();
	m_pController->OnShipStatus(IShipController::statusArmorRepaired, -1);
	m_pController->OnDeviceEnabledDisabled(iDev, bEnable, bSilent);
	}

CInstalledArmor *CShip::FindArmor (const CItem &Item)

//	FindArmor
//
//	Finds installed armor structure

	{
	if (Item.IsInstalled() && Item.GetType()->IsArmor())
		return GetArmorSection(Item.GetInstalled());
	else
		return NULL;
	}

bool CShip::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Finds a data field and returns its value

	{
	int i;

	if (strEquals(sField, FIELD_CARGO_SPACE))
		*retsValue = strFromInt(CalcMaxCargoSpace());
	else if (strEquals(sField, FIELD_MAX_SPEED))
		*retsValue = strFromInt((int)((100.0 * GetMaxSpeed() / LIGHT_SPEED) + 0.5), false);
	else if (strEquals(sField, FIELD_NAME))
		{
		DWORD dwFlags;
		CString sName = GetName(&dwFlags);
		*retsValue = ::ComposeNounPhrase(sName, 1, NULL_STR, dwFlags, 0x00);
		}
	else if (strEquals(sField, FIELD_PRIMARY_ARMOR))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_ARMOR_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_ARMOR_UNID))
		{
		CArmorClass *pBestArmor = NULL;
        int iBestLevel = 0;
		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pSect = GetArmorSection(i);
            if (pBestArmor == NULL || pSect->GetLevel() > iBestLevel)
                {
                pBestArmor = pSect->GetClass();
                iBestLevel = pSect->GetLevel();
                }
			}
		if (pBestArmor)
			*retsValue = strFromInt(pBestArmor->GetItemType()->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_SHIELD))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_SHIELD_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_SHIELD_UNID))
		{
		CInstalledDevice *pDevice = GetNamedDevice(devShields);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetClass()->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_LAUNCHER))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_LAUNCHER_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_LAUNCHER_UNID))
		{
		CInstalledDevice *pDevice = GetNamedDevice(devMissileWeapon);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetClass()->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_UNID))
		{
		CInstalledDevice *pDevice = GetNamedDevice(devPrimaryWeapon);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetClass()->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_MANEUVER))
		{
		Metric rManeuver = g_SecondsPerUpdate * m_Perf.GetRotationDesc().GetMaxRotationSpeedDegrees();
		*retsValue = strFromInt((int)((rManeuver * 1000.0) + 0.5));
		}
	else if (strEquals(sField, FIELD_THRUST_TO_WEIGHT))
		{
		Metric rMass = GetMass();
		int iRatio = (int)((200.0 * (rMass > 0.0 ? GetThrust() / rMass : 0.0)) + 0.5);
		*retsValue = strFromInt(10 * iRatio);
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_RANGE))
		{
		int iRange = 0;
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			{
			CDeviceClass *pDevice = pItem->GetDeviceClass();
			if (pDevice)
				{
				CWeaponClass *pWeapon = pDevice->AsWeaponClass();
				if (pWeapon)
					iRange = (int)((pWeapon->GetMaxEffectiveRange(NULL, NULL, NULL) / LIGHT_SECOND) + 0.5);
				}
			}
		*retsValue = strFromInt(iRange);
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_RANGE_ADJ))
		{
		int iRange = 0;
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			{
			CDeviceClass *pDevice = pItem->GetDeviceClass();
			if (pDevice)
				{
				CWeaponClass *pWeapon = pDevice->AsWeaponClass();
				if (pWeapon)
					{
					iRange = (int)((pWeapon->GetMaxEffectiveRange(NULL, NULL, NULL) / LIGHT_SECOND) + 0.5);
					iRange = iRange * m_pClass->GetAISettings().GetFireRangeAdj() / 100;
					}
				}
			}
		*retsValue = strFromInt(iRange);
		}
	else
		return m_pClass->FindDataField(sField, retsValue);

	return true;
	}

CInstalledDevice *CShip::FindDevice (const CItem &Item)

//	FindDevice
//
//	Returns the device given the item

	{
	if (Item.IsInstalled() && Item.GetType()->IsDevice())
		{
		int iDevSlot = Item.GetInstalled();
		return &m_Devices[iDevSlot];
		}
	else
		return NULL;
	}

bool CShip::FindDeviceAtPos (const CVector &vPos, CInstalledDevice **retpDevice)

//	FindDeviceAtPos
//
//	Returns a random device within a few pixels of the given position.

	{
	int i;

	const Metric MAX_DIST = 5.0 * g_KlicksPerPixel;
	CVector vUR(vPos.GetX() + MAX_DIST, vPos.GetY() + MAX_DIST);
	CVector vLL(vPos.GetX() - MAX_DIST, vPos.GetY() - MAX_DIST);

	//	Make a list of all devices near the given position

	TArray<CInstalledDevice *> List;
	for (i = 0; i < GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = GetDevice(i);
		if (!pDevice->IsEmpty()
				&& pDevice->GetPos(this).InBox(vUR, vLL))
			List.Insert(pDevice);
		}

	//	Do we have any devices? If so, return a random one.

	if (List.GetCount() > 0)
		{
		if (retpDevice)
			*retpDevice = List[mathRandom(0, List.GetCount() - 1)];

		return true;
		}
	else
		return false;
	}

int CShip::FindDeviceIndex (CInstalledDevice *pDevice) const

//	FindDeviceIndex
//
//	Finds the index for the given device

	{
	for (int i = 0; i < GetDeviceCount(); i++)
		if (pDevice == GetDevice(i))
			return i;

	return -1;
	}

int CShip::FindFreeDeviceSlot (void)

//	FindFreeDeviceSlot
//
//	Returns the index of a free device slot; -1 if none left

	{
	int i;

	for (i = 0; i < GetDeviceCount(); i++)
		if (m_Devices[i].IsEmpty())
			return i;

	//	We need to allocate a new slot

	int iNewDeviceCount = m_iDeviceCount + 1;
	CInstalledDevice *NewDevices = new CInstalledDevice [iNewDeviceCount];

	for (i = 0; i < GetDeviceCount(); i++)
		NewDevices[i] = m_Devices[i];

	delete [] m_Devices;
	m_Devices = NewDevices;
	m_iDeviceCount = iNewDeviceCount;

	return GetDeviceCount() - 1;
	}

bool CShip::FindInstalledDeviceSlot (const CItem &Item, int *retiDev)

//	FindInstalledDeviceSlot
//
//	Finds the device slot for the given item.
//	Returns FALSE if the item is not a device installed on this ship.

	{
	if (Item.GetType() == NULL || !Item.IsInstalled() || !Item.GetType()->IsDevice())
		return false;

	CItemListManipulator ShipItems(GetItemList());
	if (!ShipItems.SetCursorAtItem(Item))
		return false;

	if (retiDev)
		*retiDev = Item.GetInstalled();

	return true;
	}

int CShip::FindNextDevice (int iStart, ItemCategories Category, int iDir)

//	FindNextDevice
//
//	Finds the next device of the given category

	{
	int iStartingSlot;

	//	iStartingSlot is always >= GetDeviceCount() so that we can iterate
	//	backwards if necessary.

	if (iStart == -1)
		iStartingSlot = (iDir == 1 ? GetDeviceCount() : GetDeviceCount() - 1);
	else
		iStartingSlot = GetDeviceCount() + (iStart + iDir);

	//	Loop until we find an appropriate device

	for (int i = 0; i < GetDeviceCount(); i++)
		{
		int iDevice = ((iDir * i) + iStartingSlot) % GetDeviceCount();
		if (!m_Devices[iDevice].IsEmpty() 
				&& m_Devices[iDevice].GetCategory() == Category
				&& m_Devices[iDevice].IsSelectable(CItemCtx(this, &m_Devices[iDevice])))
			return iDevice;
		}

	return -1;
	}

int CShip::FindRandomDevice (bool bEnabledOnly)

//	FindRandomDevice
//
//	Returns a random device

	{
	int i;

	//	Count the number of valid devices

	int iCount = 0;
	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty() 
				&& (!bEnabledOnly || m_Devices[i].IsEnabled()))
			iCount++;

	if (iCount == 0)
		return -1;

	int iDev = mathRandom(1, iCount);

	//	Return the device

	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty()
				&& (!bEnabledOnly || m_Devices[i].IsEnabled()))
			{
			if (--iDev == 0)
				return i;
			}

	return -1;
	}

void CShip::FinishCreation (SShipGeneratorCtx *pCtx, SSystemCreateCtx *pSysCreateCtx)

//	FinishCreation
//
//	If we're created with a system then this is called after the system has
//	completed creation.

	{
	//	Fire OnCreate

	CSpaceObject::SOnCreate OnCreate;
	OnCreate.pCreateCtx = pSysCreateCtx;
	if (pCtx)
		{
		OnCreate.pBaseObj = pCtx->pBase;
		OnCreate.pTargetObj = pCtx->pTarget;
		}

	FireOnCreate(OnCreate);

	//	Set the orders from the generator

	if (pCtx)
		SetOrdersFromGenerator(*pCtx);

	//	We're done with OnCreate events so the AI can cache properties of 
	//	the ship (such as the best weapon).

	m_pController->OnStatsChanged();

	//	If we have a deferred orders structure we need to clean it up.

	if (m_pDeferredOrders)
		{
		if (m_pDeferredOrders->pOnCreate)
			m_pDeferredOrders->pOnCreate->Discard(&g_pUniverse->GetCC());

		delete m_pDeferredOrders;
		m_pDeferredOrders = NULL;

		//	(Remember that pCtx might be m_pDeferredOrders, so we can't use
		//	pCtx after this either.

		pCtx = NULL;
		}

	//	Add the object to the universe. We wait until the end in case
	//	OnCreate ends up setting the name (or something).

	g_pUniverse->GetGlobalObjects().InsertIfTracked(this);
	}

bool CShip::FollowsObjThroughGate (CSpaceObject *pLeader)

//	FollowsObjThroughGate
//
//	Returns true if we follow the leader through a gate

	{
	return m_pController->FollowsObjThroughGate(pLeader);
	}

void CShip::GateHook (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate, bool bAscend)

//	GateHook
//
//	Ship passes through stargate

	{
	//	Reset other gate stuff (this might be set if we re-enter a gate
	//	right after we leave it)

	m_pExitGate = NULL;
	m_iExitGateTimer = 0;

	//	Fire event

	FireOnEnteredGate(pDestNode, sDestEntryPoint, pStargate);

	//	Let the controller handle it

	m_pController->OnEnterGate(pDestNode, sDestEntryPoint, pStargate, bAscend);
	}

AbilityStatus CShip::GetAbility (Abilities iAbility)

//	GetAbility
//
//	Returns the status of the given ability

	{
	switch (iAbility)
		{
		case ablAutopilot:
			return (HasAutopilot() ? ablInstalled : ablUninstalled);

		case ablExtendedScanner:
			return (IsSRSEnhanced() ? ablInstalled : ablUninstalled);

		case ablFriendlyFireLock:
			return (CanTargetFriendlies() ? ablUninstalled : ablInstalled);

		case ablGalacticMap:
			return (m_fGalacticMap ? ablInstalled : ablUninstalled);

		case ablLongRangeScanner:
			return ablInstalled;

		case ablShortRangeScanner:
			return (IsBlind() ? ablDamaged : ablInstalled);

		case ablSystemMap:
			return ablInstalled;

		case ablTargetingSystem:
			return (HasTargetingComputer() ? ablInstalled : ablUninstalled);

		default:
			return ablUninstalled;
		}
	}

CurrencyValue CShip::GetBalance (DWORD dwEconomyUNID)

//	GetBalance
//
//	Returns the amount of money the station has left

	{
	ASSERT(m_pController);

	CCurrencyBlock *pMoney = m_pController->GetCurrencyBlock();
	if (pMoney)
		return (int)pMoney->GetCredits(dwEconomyUNID);
	else
		return 0;
	}

CSpaceObject *CShip::GetBase (void) const

//	GetBase
//
//	Get the base for this ship

	{
	//	If we're docked, then that's our base

	if (m_pDocked)
		return m_pDocked;

	//	Otherwise, ask the controller

	else
		return m_pController->GetBase();
	}

Metric CShip::GetCargoMass (void)

//	GetCargoMass
//
//	Returns the total mass of all items in cargo (in metric tons).

	{
	if (m_fRecalcItemMass)
		{
		m_rItemMass = CalculateItemMass(&m_rCargoMass);
		m_fRecalcItemMass = false;
		}

	return m_rCargoMass;
	}

Metric CShip::GetCargoSpaceLeft (void)

//	GetCargoSpaceLeft
//
//	Returns the amount of cargo space left in tons

	{
	//	Compute total cargo space. Start with the space specified
	//	by the class. Then see if any devices add to it.

	Metric rCargoSpace = (Metric)CalcMaxCargoSpace();
	
	//	Compute cargo mass

	OnComponentChanged(comCargo);

	//	Compute space left

	return Max(0.0, rCargoSpace - GetCargoMass());
	}

int CShip::GetCombatPower (void)

//	GetCombatPower
//
//	Returns the relative strength of this object in combat
//	Scale from 0 to 100 (Note: This works for ships up to
//	level XV).

	{
	return m_pController->GetCombatPower();
	}

int CShip::GetDamageEffectiveness (CSpaceObject *pAttacker, CInstalledDevice *pWeapon)

//	GetDamageEffectiveness
//
//	Returns the effectiveness of the given weapon against this ship.
//
//	< 0		The weapon is ineffective against us.
//	0-99	The weapon is less effective than average.
//	100		The weapon has average effectiveness
//	> 100	The weapon is more effective than average.

	{
	//	First ask the shields, if we have them, and if they are up.

	CInstalledDevice *pShields = GetNamedDevice(devShields);
	if (pShields && GetShieldLevel() > 0)
		return pShields->GetDamageEffectiveness(pAttacker, pWeapon);
	else if (m_Armor.GetSegmentCount() > 0)
		return m_Armor.GetSegment(0).GetDamageEffectiveness(pAttacker, pWeapon);
	else
		return 100;
	}

DamageTypes CShip::GetDamageType (void)

//	GetDamageType
//
//	Returns the type of damage that this ship does

	{
	CInstalledDevice *pWeapon = GetNamedDevice(devPrimaryWeapon);
	if (pWeapon)
		return (DamageTypes)pWeapon->GetDamageType(CItemCtx(this, pWeapon));
	else
		return damageGeneric;
	}

DeviceNames CShip::GetDeviceNameForCategory (ItemCategories iCategory)

//	GetDeviceNameForCategory
//
//	Returns a named device id for the given category

	{
	switch (iCategory)
		{
		case itemcatWeapon:
			return devPrimaryWeapon;

		case itemcatLauncher:
			return devMissileWeapon;

		case itemcatShields:
			return devShields;

		case itemcatDrive:
			return devDrive;

		case itemcatCargoHold:
			return devCargo;

		case itemcatReactor:
			return devReactor;

		default:
			return devNone;
		}
	}

CDesignType *CShip::GetDefaultDockScreen (CString *retsName)

//	GetDockScreen
//
//	Returns the screen on dock (NULL_STR if none)

	{
	//	If docking is disabled or we are docked with some station, then no one
	//	can dock with us.

	if (m_fDockingDisabled 
			|| m_pDocked != NULL
			|| IsDestroyed())
		return NULL;

	//	If no screen, then we're done

	CDesignType *pScreen = m_pClass->GetFirstDockScreen(retsName);
	if (pScreen == NULL)
		return NULL;

	//	If we're moving, then no one can dock.

	if (GetVel().Length2() > MAX_SPEED_FOR_DOCKING2)
		return NULL;

	//	Done

	return pScreen;
	}

CSpaceObject *CShip::GetEscortPrincipal (void) const

//	GetEscortPrincipal
//
//	Returns the ship that is being escorted by this ship (or NULL)

	{
	return m_pController->GetEscortPrincipal();
	}

CString CShip::GetInstallationPhrase (const CItem &Item) const

//	GetInstallationPhrase
//
//	Returns a phrase that describes the installation state
//	of the item.

	{
	if (Item.IsInstalled())
		{
		switch (Item.GetType()->GetCategory())
			{
			case itemcatArmor:
				{
				int iSeg = Item.GetInstalled();
				CString sName = m_pClass->GetHullSectionName(iSeg);

				if (!sName.IsBlank())
					return strPatternSubst(CONSTLIT("Installed as %s armor"), sName);
				else
					return CONSTLIT("Installed as armor");
				}

			case itemcatLauncher:
				return CONSTLIT("Installed as launcher");

			case itemcatWeapon:
				return CONSTLIT("Installed as weapon");

			case itemcatCargoHold:
				return CONSTLIT("Installed as cargo hold expansion");

			case itemcatDrive:
				return CONSTLIT("Installed as drive upgrade");

			case itemcatReactor:
				return CONSTLIT("Installed as reactor");

			case itemcatShields:
				return CONSTLIT("Installed as shield generator");

			case itemcatMiscDevice:
				return CONSTLIT("Installed as device");

			default:
				return CONSTLIT("Installed");
			}
		}

	return CString();
	}

int CShip::GetItemDeviceName (const CItem &Item) const

//	GetItemDeviceName
//
//	Returns the DeviceName of the given item. We assume that the
//	item is installed. Returns -1 if not installed.

	{
	if (Item.IsInstalled())
		{
		//	The device slot that this item is installed in is
		//	stored in the data field.

		int iSlot = Item.GetInstalled();

		//	Look to see if this slot has a name

		for (int i = devFirstName; i < devNamesCount; i++)
			if (m_NamedDevices[i] == iSlot)
				return i;
		}

	//	Not named or not installed

	return -1;
	}

Metric CShip::GetItemMass (void)

//	GetItemMass
//
//	Returns the total mass of all items (in metric tons).

	{
	if (m_fRecalcItemMass)
		{
		m_rItemMass = CalculateItemMass(&m_rCargoMass);
		m_fRecalcItemMass = false;
		}

	return m_rItemMass;
	}

Metric CShip::GetMass (void)

//	GetMass
//
//	Returns the mass of the object in metric tons

	{
	return m_pClass->GetHullMass() + GetItemMass();
	}

Metric CShip::GetMaxAcceleration (void)

//	GetMaxAcceleration
//
//	Returns the maximum possible acceleration (given our current thrust and mass)
//	in klicks per second per second.

	{
	Metric rMass = GetMass();
	if (rMass <= 0.0)
		return 0.0;

	return GetThrust() * 1000.0 / rMass;
	}

Metric CShip::GetMaxFuel (void)

//	GetMaxFuel
//
//	Return the maximum amount of fuel that the reactor can hold

	{
    return m_Perf.GetReactorDesc().GetFuelCapacity();
	}

int CShip::GetMaxPower (void) const

//	GetMaxPower
//
//	Return max power output

	{
    return m_Perf.GetReactorDesc().GetMaxPower();
	}

Metric CShip::GetMaxWeaponRange (void) const

//	GetMaxWeaponRange
//
//	Returns the maximum range of weapons.

	{
	Metric rRange = 0.0;
	CInstalledDevice *pDevice = GetNamedDevice(devPrimaryWeapon);
	if (pDevice)
		rRange = pDevice->GetMaxRange(CItemCtx(const_cast<CShip *>(this), pDevice));

	pDevice = GetNamedDevice(devMissileWeapon);
	if (pDevice)
		rRange = Max(rRange, pDevice->GetMaxRange(CItemCtx(const_cast<CShip *>(this), pDevice)));

	return rRange;
	}

int CShip::GetMissileCount (void)

//	GetMissileCount
//
//	Returns the number of missile variants

	{
	CInstalledDevice *pDevice = GetNamedDevice(devMissileWeapon);
	if (pDevice)
		{
		CDeviceClass *pClass = pDevice->GetClass();
		return pClass->GetValidVariantCount(this, pDevice);
		}
	else
		return 0;
	}

CString CShip::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the ship

	{
	if (m_sName.IsBlank())
		return m_pClass->GetName(retdwFlags);
	else
		{
		if (retdwFlags)
			*retdwFlags = m_dwNameFlags;

		return m_sName;
		}
	}

CInstalledDevice *CShip::GetNamedDevice (DeviceNames iDev) const
	{
	if (m_NamedDevices[iDev] == -1)
		return NULL;
	else
		return &m_Devices[m_NamedDevices[iDev]]; 
	}

CDeviceClass *CShip::GetNamedDeviceClass (DeviceNames iDev)
	{
	if (m_NamedDevices[iDev] == -1)
		return NULL;
	else
		return m_Devices[m_NamedDevices[iDev]].GetClass(); 
	}

CItem CShip::GetNamedDeviceItem (DeviceNames iDev)

//	GetNamedDeviceItem
//
//	Returns the item for the named device

	{
	if (m_NamedDevices[iDev] == -1)
		return CItem();
	else
		{
		CItemListManipulator ItemList(GetItemList());
		SetCursorAtNamedDevice(ItemList, iDev);
		return ItemList.GetItemAtCursor();
		}
	}

int CShip::GetPerception (void)

//	GetPerception
//
//	Returns the ship's current perception level (including external
//	factors such as nebulae and blindness)

	{
	//	calculate perception

	int iPerception = m_pClass->GetAISettings().GetPerception();

	return Max((int)perceptMin, iPerception);
	}

int CShip::GetPowerConsumption (void)

//	GetPowerConsumption
//
//	Returns the amount of power consumed this turn

	{
	if (m_fTrackFuel)
		return m_iPowerDrain;
	else
		return 0;
	}

ICCItem *CShip::GetProperty (CCodeChainCtx &Ctx, const CString &sName)

//	GetProperty
//
//	Returns a property

	{
	int i;
	CCodeChain &CC = g_pUniverse->GetCC();
    ICCItem *pResult;

	if (strEquals(sName, PROPERTY_ALWAYS_LEAVE_WRECK))
		return CC.CreateBool(m_fAlwaysLeaveWreck || m_pClass->GetWreckChance() >= 100);

	else if (strEquals(sName, PROPERTY_AVAILABLE_DEVICE_SLOTS))
		{
		int iAll = CalcDeviceSlotsInUse();

		return CC.CreateInteger(m_pClass->GetMaxDevices() - iAll);
		}
	else if (strEquals(sName, PROPERTY_AVAILABLE_NON_WEAPON_SLOTS))
		{
		int iNonWeapon;
		int iAll = CalcDeviceSlotsInUse(NULL, &iNonWeapon);

		return CC.CreateInteger(Max(0, Min(m_pClass->GetMaxNonWeapons() - iNonWeapon, m_pClass->GetMaxDevices() - iAll)));
		}
	else if (strEquals(sName, PROPERTY_AVAILABLE_WEAPON_SLOTS))
		{
		int iWeapon;
		int iAll = CalcDeviceSlotsInUse(&iWeapon);

		return CC.CreateInteger(Max(0, Min(m_pClass->GetMaxWeapons() - iWeapon, m_pClass->GetMaxDevices() - iAll)));
		}
	else if (strEquals(sName, PROPERTY_BLINDING_IMMUNE))
		{
		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pSection = GetArmorSection(i);
			if (!pSection->GetClass()->IsBlindingDamageImmune(CItemCtx(this, pSection)))
				return CC.CreateNil();
			}

		return (GetArmorSectionCount() > 0 ? CC.CreateTrue() : CC.CreateNil());
		}
	else if (strEquals(sName, PROPERTY_CARGO_SPACE))
		return CC.CreateInteger(CalcMaxCargoSpace());

	else if (strEquals(sName, PROPERTY_CHARACTER))
		{
		CGenericType *pCharacter = m_pClass->GetCharacter();
		return (pCharacter ? CC.CreateInteger(pCharacter->GetUNID()) : CC.CreateNil());
		}
	else if (strEquals(sName, PROPERTY_DEVICE_DAMAGE_IMMUNE) || strEquals(sName, PROPERTY_DEVICE_DISRUPT_IMMUNE))
		{
		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pSection = GetArmorSection(i);
			if (!pSection->GetClass()->IsDeviceDamageImmune(CItemCtx(this, pSection)))
				return CC.CreateNil();
			}

		return (GetArmorSectionCount() > 0 ? CC.CreateTrue() : CC.CreateNil());
		}
	else if (strEquals(sName, PROPERTY_DISINTEGRATION_IMMUNE))
		{
		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pSection = GetArmorSection(i);
			if (!pSection->GetClass()->IsDisintegrationImmune(CItemCtx(this, pSection)))
				return CC.CreateNil();
			}

		return (GetArmorSectionCount() > 0 ? CC.CreateTrue() : CC.CreateNil());
		}
	else if (strEquals(sName, PROPERTY_DOCKED_AT_ID))
		return (m_pDocked ? CC.CreateInteger(m_pDocked->GetID()) : CC.CreateNil());

	else if (strEquals(sName, PROPERTY_DOCKING_ENABLED))
		return CC.CreateBool(SupportsDocking(true));

    else if (strEquals(sName, PROPERTY_DOCKING_PORT_COUNT))
        return CC.CreateInteger(m_DockingPorts.GetPortCount(this));

    else if (strEquals(sName, PROPERTY_HEALER_LEFT))
        return CC.CreateInteger(m_Armor.GetHealerLeft());

	else if (strEquals(sName, PROPERTY_EMP_IMMUNE))
		{
		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pSection = GetArmorSection(i);
			if (!pSection->GetClass()->IsEMPDamageImmune(CItemCtx(this, pSection)))
				return CC.CreateNil();
			}

		return (GetArmorSectionCount() > 0 ? CC.CreateTrue() : CC.CreateNil());
		}

    else if (strEquals(sName, PROPERTY_FUEL_LEFT))
        return CC.CreateInteger(mathRound(GetFuelLeft() / FUEL_UNITS_PER_STD_ROD));

    else if (strEquals(sName, PROPERTY_FUEL_LEFT_EXACT))
        return CC.CreateDouble(GetFuelLeft());

	else if (strEquals(sName, PROPERTY_INTERIOR_HP))
		{
		int iHP;
		m_Interior.GetHitPoints(this, m_pClass->GetInteriorDesc(), &iHP);
		return CC.CreateInteger(iHP);
		}

	else if (strEquals(sName, PROPERTY_MAX_FUEL))
        return CC.CreateInteger(mathRound(GetMaxFuel() / FUEL_UNITS_PER_STD_ROD));

    else if (strEquals(sName, PROPERTY_MAX_FUEL_EXACT))
        return CC.CreateDouble(GetMaxFuel());

	else if (strEquals(sName, PROPERTY_MAX_INTERIOR_HP))
		{
		int iHP;
		int iMaxHP;
		m_Interior.GetHitPoints(this, m_pClass->GetInteriorDesc(), &iHP, &iMaxHP);
		return CC.CreateInteger(iMaxHP);
		}

	else if (strEquals(sName, PROPERTY_OPEN_DOCKING_PORT_COUNT))
		return CC.CreateInteger(GetOpenDockingPortCount());

	else if (strEquals(sName, PROPERTY_OPERATING_SPEED))
		{
		if (m_fEmergencySpeed)
			return CC.CreateString(SPEED_EMERGENCY);
		else if (m_fHalfSpeed)
			return CC.CreateString(SPEED_HALF);
		else if (m_fQuarterSpeed)
			return CC.CreateString(SPEED_QUARTER);
		else
			return CC.CreateString(SPEED_FULL);
		}

	else if (strEquals(sName, PROPERTY_PLAYER_WINGMAN))
		return CC.CreateBool(m_pController->IsPlayerWingman());

    else if (strEquals(sName, PROPERTY_POWER_USE))
        return CC.CreateDouble(GetPowerConsumption() * 100.0);

	else if (strEquals(sName, PROPERTY_RADIATION_IMMUNE))
		{
		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pSection = GetArmorSection(i);
			if (!pSection->GetClass()->IsRadiationImmune(CItemCtx(this, pSection)))
				return CC.CreateNil();
			}

		return (GetArmorSectionCount() > 0 ? CC.CreateTrue() : CC.CreateNil());
		}
	else if (strEquals(sName, PROPERTY_ROTATION))
		return CC.CreateInteger(GetRotation());

	else if (strEquals(sName, PROPERTY_ROTATION_SPEED))
		return CC.CreateDouble(m_Rotation.GetRotationSpeedDegrees(m_Perf.GetRotationDesc()));

	else if (strEquals(sName, PROPERTY_SELECTED_LAUNCHER))
		{
		CItem theItem = GetNamedDeviceItem(devMissileWeapon);
		if (theItem.GetType() == NULL)
			return CC.CreateNil();

		return CreateListFromItem(CC, theItem);
		}
	else if (strEquals(sName, PROPERTY_SELECTED_MISSILE))
		{
		CInstalledDevice *pLauncher = GetNamedDevice(devMissileWeapon);
		if (pLauncher == NULL)
			return CC.CreateNil();

		CItemType *pType;
		pLauncher->GetSelectedVariantInfo(this, NULL, NULL, &pType);
		if (pType == NULL)
			return CC.CreateNil();

		if (pType->IsMissile())
			{
			CItemListManipulator ItemList(GetItemList());
			CItem theItem(pType, 1);
			if (!ItemList.SetCursorAtItem(theItem))
				return CC.CreateNil();

			return CreateListFromItem(CC, ItemList.GetItemAtCursor());
			}

		//	Sometimes a launcher has no ammo (e.g., a disposable missile
		//	launcher). In that case we return the launcher itself.

		else
			{
			CItem theItem = GetNamedDeviceItem(devMissileWeapon);
			if (theItem.GetType() == NULL)
				return CC.CreateNil();

			return CreateListFromItem(CC, theItem);
			}
		}
	else if (strEquals(sName, PROPERTY_SELECTED_WEAPON))
		{
		CItem theItem = GetNamedDeviceItem(devPrimaryWeapon);
		if (theItem.GetType() == NULL)
			return CC.CreateNil();

		return CreateListFromItem(CC, theItem);
		}
	else if (strEquals(sName, PROPERTY_SHATTER_IMMUNE))
		{
		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CItemCtx Ctx(this, GetArmorSection(i));
			if (!Ctx.GetArmorClass()->IsShatterImmune(Ctx))
				return CC.CreateNil();
			}

		return (GetArmorSectionCount() > 0 ? CC.CreateTrue() : CC.CreateNil());
		}

	//	Drive properties

	else if (strEquals(sName, PROPERTY_DRIVE_POWER))
		return CC.CreateInteger(m_Perf.GetDriveDesc().GetPowerUse() * 100);

	else if (strEquals(sName, PROPERTY_MAX_SPEED))
		return CC.CreateInteger((int)((100.0 * GetMaxSpeed() / LIGHT_SPEED) + 0.5));

	else if (strEquals(sName, PROPERTY_THRUST))
		return CC.CreateInteger((int)GetThrust());

	else if (strEquals(sName, PROPERTY_THRUST_TO_WEIGHT))
		{
		Metric rMass = GetMass();
		int iRatio = (int)((200.0 * (rMass > 0.0 ? GetThrust() / rMass : 0.0)) + 0.5);
		return CC.CreateInteger(10 * iRatio);
		}

	//	Reactor properties

	else if (CReactorDesc::IsExportedProperty(sName))
		{
		if (pResult = m_Perf.GetReactorDesc().FindProperty(sName))
			return pResult;
		else
			return CC.CreateNil();
		}

    //  Controller properties

    else if (pResult = m_pController->FindProperty(sName))
        return pResult;

	else
		return CSpaceObject::GetProperty(Ctx, sName);
	}

CString CShip::GetReactorName (void)

//	GetReactorName
//
//	Returns he name of the ship's reactor

	{
	CInstalledDevice *pReactor = GetNamedDevice(devReactor);
	if (pReactor)
		return pReactor->GetClass()->GetItemType()->GetNounPhrase();
	else
		return strPatternSubst(CONSTLIT("%s reactor"), m_pClass->GetShortName());
	}

int CShip::GetShieldLevel (void)

//	GetShieldLevel
//
//	Returns the % shield level of the ship (or -1 if the ship has no shields)

	{
	CInstalledDevice *pShields = GetNamedDevice(devShields);
	if (pShields == NULL)
		return -1;

	int iHP, iMaxHP;
	pShields->GetStatus(this, &iHP, &iMaxHP);
	if (iMaxHP == 0)
		return -1;

	return iHP * 100 / iMaxHP;
	}

int CShip::GetStealth (void) const

//	GetStealth
//
//	Returns the stealth of the ship

	{
	int iStealth = m_iStealth;

	//	+6 stealth if in nebula, which decreases detection range by about 3.

	if (m_fHiddenByNebula)
		iStealth += 6;

	return Min((int)stealthMax, iStealth);
	}

CSpaceObject *CShip::GetTarget (CItemCtx &ItemCtx, bool bNoAutoTarget) const

//	GetTarget
//
//	Returns the target that this ship is attacking

	{
	return m_pController->GetTarget(ItemCtx, bNoAutoTarget);
	}

CCurrencyAndValue CShip::GetTradePrice (CSpaceObject *pProvider)

//	GetTradePrice
//
//	Returns the fair price of this ship as calculated by pProvider, before any
//	markups.

	{
	//	Get the hull value

	const CPlayerSettings *pPlayer = m_pClass->GetPlayerSettings();
	CCurrencyAndValue Value = (pPlayer ? pPlayer->GetHullValue() : CCurrencyAndValue(0, GetDefaultEconomy()));

	//	Add up the value of all installed items

	CItemListManipulator AllItems(GetItemList());
	while (AllItems.MoveCursorForward())
		{
		const CItem &Item = AllItems.GetItemAtCursor();
		if (Item.IsInstalled())
			{
			//	We use the raw value because not all stations sell all items. 
			//	This at least gives us some base price.

			int iItemValue = Max(0, Item.GetTradePrice(pProvider, true));
			if (iItemValue <= 0)
				continue;

			Value.Add(CCurrencyAndValue(iItemValue, Item.GetCurrencyType()));

			//	Need to include install cost

			Value.Add(CCurrencyAndValue(Item.GetType()->GetInstallCost(CItemCtx(Item)), Item.GetCurrencyType()));
			}
		}

	//	Done

	return Value;
	}

int CShip::GetVisibleDamage (void)

//	GetVisibleDamage
//
//	Returns the amount of damage (%) that the object has taken

	{
	int i;

	//	If we have interior structure then we follow a different algorithm

	if (!m_Interior.IsEmpty())
		{
		//	Compute:
		//
		//	The sum of max hit points of all armor segments
		//	The sum of remaining hit points of all armor segments
		//	The percent damage of the worst armor segment

		int iTotalMaxArmor = 0;
		int iTotalArmorLeft = 0;
		int iWorstDamage = 0;

		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pArmor = GetArmorSection(i);

			int iMaxHP = pArmor->GetMaxHP(this);
			iTotalMaxArmor += iMaxHP;

			int iLeft = pArmor->GetHitPoints();
			iTotalArmorLeft += iLeft;

			int iDamage = (iMaxHP > 0 ? 100 - (iLeft * 100 / iMaxHP) : 100);
			if (iDamage > iWorstDamage)
				iWorstDamage = iDamage;
			}

		//	Compute the percent damage of all armor segments.

		int iArmorDamage = (iTotalMaxArmor > 0 ? 100 - (iTotalArmorLeft * 100 / iTotalMaxArmor) : 100);

		//	Compute the percent damage of all interior segments.

		int iMaxInterior;
		int iInteriorLeft;
		m_Interior.GetHitPoints(this, m_pClass->GetInteriorDesc(), &iInteriorLeft, &iMaxInterior);
		int iInteriorDamage = (iMaxInterior > 0 ? 100 - (iInteriorLeft * 100 / iMaxInterior) : 100);

		//	Combine all damage together, scaled so that interior + worst damage are 90%
		//	of the result.

		return ((10 * iArmorDamage) + (45 * iWorstDamage) + (45 * iInteriorDamage)) / 100;
		}

	//	Otherwise, we base it on armor

	else
		{
		int iMaxPercent = 0;

		//	Compute max and actual HP

		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pArmor = GetArmorSection(i);

			int iMaxHP = pArmor->GetMaxHP(this);
			int iDamage = (iMaxHP > 0 ? 100 - (pArmor->GetHitPoints() * 100 / iMaxHP) : 100);
			if (iDamage > iMaxPercent)
				iMaxPercent = iDamage;
			}

		//	Return % damage of the worst armor segment

		return iMaxPercent;
		}
	}

void CShip::GetVisibleDamageDesc (SVisibleDamage &Damage)

//	GetVisibleDamageDesc
//
//	Returns the amount of damage (%) that the object has taken

	{
	int i;

	//	Get shield level

	Damage.iShieldLevel = GetShieldLevel();

	//	If we have interior structure then we follow a different algorithm

	if (!m_Interior.IsEmpty())
		{
		//	Compute:
		//
		//	The sum of max hit points of all armor segments
		//	The sum of remaining hit points of all armor segments
		//	The percent damage of the worst armor segment

		int iTotalMaxArmor = 0;
		int iTotalArmorLeft = 0;
		int iWorstDamage = 0;

		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pArmor = GetArmorSection(i);

			int iMaxHP = pArmor->GetMaxHP(this);
			iTotalMaxArmor += iMaxHP;

			int iLeft = pArmor->GetHitPoints();
			iTotalArmorLeft += iLeft;

			int iDamage = (iMaxHP > 0 ? 100 - (iLeft * 100 / iMaxHP) : 100);
			if (iDamage > iWorstDamage)
				iWorstDamage = iDamage;
			}

		//	Compute the percent damage of all armor segments.

		int iArmorDamage = (iTotalMaxArmor > 0 ? 100 - (iTotalArmorLeft * 100 / iTotalMaxArmor) : 100);

		//	Compute the percent damage of all interior segments.

		int iMaxInterior;
		int iInteriorLeft;
		m_Interior.GetHitPoints(this, m_pClass->GetInteriorDesc(), &iInteriorLeft, &iMaxInterior);

		//	Combine all damage together, scaled so that interior + worst damage are 90%
		//	of the result.

		Damage.iArmorLevel = 100 - (((20 * iArmorDamage) + (80 * iWorstDamage)) / 100);
		Damage.iHullLevel = (iMaxInterior > 0 ? (iInteriorLeft * 100 / iMaxInterior) : -1);
		}

	//	Otherwise, we base it on armor

	else
		{
		int iMaxPercent = 0;

		//	Compute max and actual HP

		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			CInstalledArmor *pArmor = GetArmorSection(i);

			int iMaxHP = pArmor->GetMaxHP(this);
			int iDamage = (iMaxHP > 0 ? 100 - (pArmor->GetHitPoints() * 100 / iMaxHP) : 100);
			if (iDamage > iMaxPercent)
				iMaxPercent = iDamage;
			}

		//	Return % damage of the worst armor segment

		Damage.iArmorLevel = 100 - iMaxPercent;
		Damage.iHullLevel = -1;
		}
	}

bool CShip::GetWeaponIsReady (DeviceNames iDev)

//	GetWeaponIsReady
//
//	Returns TRUE if the weapon is ready

	{
	CInstalledDevice *pWeapon = GetNamedDevice(iDev);
	return (pWeapon && pWeapon->IsReady());
	}

Metric CShip::GetWeaponRange (DeviceNames iDev)

//  GetWeaponRange
//
//  Returns the range of the given weapon.

    {
	CInstalledDevice *pWeapon = GetNamedDevice(iDev);
    if (pWeapon == NULL)
        return 0.0;

    return pWeapon->GetMaxEffectiveRange(this);
    }

bool CShip::HasAttribute (const CString &sAttribute) const

//	HasAttribute
//
//	Returns TRUE if it has the given attribute

	{
	return m_pClass->HasLiteralAttribute(sAttribute);
	}

bool CShip::HasNamedDevice (DeviceNames iDev) const

//	HasNamedDevice
//
//	Returns TRUE if the ship has the named device installed

	{
	return (m_NamedDevices[iDev] != -1);
	}

bool CShip::ImageInObject (const CVector &vObjPos, const CObjectImageArray &Image, int iTick, int iRotation, const CVector &vImagePos)

//	ImageInObject
//
//	Returns TRUE if the given image at the given position intersects this
//	object

	{
	return ImagesIntersect(Image,
			iTick,
			iRotation, 
			vImagePos,
			m_pClass->GetImage(),
			GetSystem()->GetTick(), 
			m_Rotation.GetFrameIndex(), 
			vObjPos);
	}

void CShip::InstallItemAsArmor (CItemListManipulator &ItemList, int iSect)

//	InstallItemAsArmor
//
//	Installs the item at the cursor as armor.

	{
	//	Remember the item that we want to install

	CItem NewArmor = ItemList.GetItemAtCursor();

	//	Find the item that is currently installed in this section
	//	and mark it as being prepared for uninstall.

	bool bFound = false;
	ItemList.ResetCursor();
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.GetType()->GetCategory() == itemcatArmor
				&& Item.GetInstalled() == iSect)
			{
			ASSERT(Item.GetCount() == 1);
			bFound = true;
			break;
			}
		}

	ASSERT(bFound);
	if (!bFound)
		return;

	//	Fire OnUninstall first

	CItem OldArmor = ItemList.GetItemAtCursor();
	OldArmor.FireOnUninstall(this);
	m_pController->OnItemUninstalled(OldArmor);

	//	Find the item in the list (because OnUninstall may have disturbed
	//	the item list.

	if (!ItemList.Refresh(OldArmor))
		return;

	//	Mark the item as about to be uninstalled

	ItemList.SetInstalledAtCursor(-2);
	OldArmor = ItemList.GetItemAtCursor();

	//	How damaged is the current armor?

	bool bDamageOldArmor = IsArmorDamaged(iSect);
	bool bDestroyOldArmor = !IsArmorRepairable(iSect);

	//	Now install the selected item as new armor

	ItemList.Refresh(NewArmor);

	CInstalledArmor *pSect = GetArmorSection(iSect);
	pSect->Install(this, ItemList, iSect);

	//	The item is now known and referenced.

	if (IsPlayer())
		{
		NewArmor.GetType()->SetKnown();
		NewArmor.GetType()->SetShowReference();
		}

	//	Look for the previous item

	if (ItemList.Refresh(OldArmor))
		{
		//	Remove

		ItemList.DeleteAtCursor(1);
		InvalidateItemListAddRemove();

		if (!bDestroyOldArmor)
			{
			OldArmor.SetInstalled(-1);
			OldArmor.SetCount(1);
			if (bDamageOldArmor)
				OldArmor.SetDamaged();
			else
				OldArmor.ClearDamaged();

			ItemList.AddItem(OldArmor);
			}
		}

	//	Restore the cursor to point at the new armor segment

	ItemList.Refresh(NewArmor);

	//	Update

	CalcArmorBonus();
	CalcDeviceBonus();
    CalcPerformance();

	InvalidateItemListState();
    m_pController->OnStatsChanged();
	m_pController->OnShipStatus(IShipController::statusArmorRepaired, iSect);
	}

void CShip::InstallItemAsDevice (CItemListManipulator &ItemList, int iDeviceSlot, int iSlotPosIndex)

//	InstallItemAsDevice
//
//	Installs the item at the cursor as a device

	{
	CDeviceClass *pNewDevice = ItemList.GetItemAtCursor().GetType()->GetDeviceClass();
	ASSERT(pNewDevice);

	DeviceNames iNamedSlot = GetDeviceNameForCategory(pNewDevice->GetCategory());

	//	If necessary, remove previous item in a slot

	int iSlotToReplace;
	CalcDeviceToReplace(ItemList.GetItemAtCursor(), iDeviceSlot, &iSlotToReplace);
	if (iSlotToReplace != -1)
		{
		//	If we're replacing a device, remember the slot position so that
		//	we can preserve it for the new device.
		//	(But only if we're the same kind of device).

		if (m_Devices[iSlotToReplace].GetCategory() == ItemList.GetItemAtCursor().GetType()->GetCategory())
			iSlotPosIndex = m_Devices[iSlotToReplace].GetSlotPosIndex();

		//	If we're upgrading/downgrading a reactor, then remember the old fuel level

		Metric rOldFuel = -1.0;
		if (iNamedSlot == devReactor)
			rOldFuel = GetFuelLeft();

		//	Remove the item

		CItem ItemToInstall = ItemList.GetItemAtCursor();
		CItemListManipulator RemoveItem(GetItemList());
		SetCursorAtDevice(RemoveItem, iSlotToReplace);
		RemoveItemAsDevice(RemoveItem);
		ItemList.Refresh(ItemToInstall);

		//	Reset the fuel level (we are effectively transfering the fuel to the
		//	new reactor. Note that on a downgrade, we will clip the fuel to the
		//	maximum when we do a CalcDeviceBonus).

		if (rOldFuel != -1.0 && !m_fOutOfFuel)
			m_rFuelLeft = rOldFuel;
		}

	//	Look for a free slot to install to

	if (iDeviceSlot == -1)
		{
		iDeviceSlot = FindFreeDeviceSlot();
		if (iDeviceSlot == -1)
			{
			ASSERT(false);
			return;
			}
		}

	CInstalledDevice *pDevice = &m_Devices[iDeviceSlot];
    CItemCtx ItemCtx(this, pDevice);

	//	Update the structure

	pDevice->Install(this, ItemList, iDeviceSlot);

	//	If we have a slot positing index, set it now

	if (iSlotPosIndex != -1)
		pDevice->SetSlotPosIndex(iSlotPosIndex);

	//	Adjust the named devices

	switch (pDevice->GetCategory())
		{
		case itemcatWeapon:
			if (pDevice->IsSelectable(ItemCtx))
				m_NamedDevices[devPrimaryWeapon] = iDeviceSlot;
			m_pController->OnWeaponStatusChanged();
			break;

		case itemcatLauncher:
			if (pDevice->IsSelectable(ItemCtx))
				m_NamedDevices[devMissileWeapon] = iDeviceSlot;
			m_pController->OnWeaponStatusChanged();
			break;

		case itemcatShields:
			m_NamedDevices[devShields] = iDeviceSlot;
			//	If we just installed a shield generator, start a 0 energy
			pDevice->Reset(this);
			m_pController->OnShipStatus(IShipController::statusArmorRepaired, -1);
			break;

		case itemcatDrive:
			m_NamedDevices[devDrive] = iDeviceSlot;
			break;

		case itemcatCargoHold:
			m_NamedDevices[devCargo] = iDeviceSlot;
			break;

		case itemcatReactor:
			m_NamedDevices[devReactor] = iDeviceSlot;
			break;
		}

	//	Recalc bonuses

	CalcDeviceBonus();
    CalcPerformance();

    m_pController->OnStatsChanged();
	InvalidateItemListState();
	}

bool CShip::IsAngryAt (CSpaceObject *pObj)

//	IsAngryAt
//
//	Returns true if the ship is angry at the given object

	{
	if (IsEnemy(pObj))
		return true;

	return m_pController->IsAngryAt(pObj);
	}

bool CShip::IsArmorDamaged (int iSect)

//	IsArmorDamaged
//
//	Returns TRUE if the given armor section is damaged

	{
	CInstalledArmor *pSect = GetArmorSection(iSect);
	return (pSect->GetHitPoints() < pSect->GetMaxHP(this));
	}

bool CShip::IsArmorRepairable (int iSect)

//	IsArmorRepairable
//
//	Returns TRUE if the given armor section can be repaired

	{
	CInstalledArmor *pSect = GetArmorSection(iSect);
	return (pSect->GetHitPoints() >= (pSect->GetMaxHP(this) / 4));
	}

bool CShip::IsDeviceSlotAvailable (ItemCategories iItemCat, int *retiSlot)

//	IsDeviceSlotAvailable
//
//	Returns TRUE if it is possible to install a new device. If the slot is full
//	then we return the device slot that we need to replace.

	{
	int i;

	switch (iItemCat)
		{
		case itemcatLauncher:
		case itemcatReactor:
		case itemcatShields:
		case itemcatCargoHold:
		case itemcatDrive:
			{
			//	Look for a device of the given slot category.

			for (i = 0; i < GetDeviceCount(); i++)
				if (!m_Devices[i].IsEmpty()
						&& m_Devices[i].GetCategory() == iItemCat)
					{
					if (retiSlot)
						*retiSlot = i;
					return false;
					}

			return true;
			}

		default:
			ASSERT(retiSlot == NULL);
			return true;
		}
	}

bool CShip::IsFuelCompatible (const CItem &Item)

//	IsFuelCompatible
//
//	Returns TRUE if the given fuel is compatible

	{
	return m_Perf.GetReactorDesc().IsFuelCompatible(Item);
	}

bool CShip::IsPlayer (void) const

//	IsPlayer
//
//	Returns TRUE if this is the player ship

	{
	return m_pController->IsPlayer();
	}

bool CShip::IsRadiationImmune (void)

//	IsRadiationImmune
//
//	Return TRUE if the ship is immune to radiation. The ship is immune if all
//	armor segments are immune.

	{
	for (int i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pSection = GetArmorSection(i);
		if (!pSection->GetClass()->IsRadiationImmune(CItemCtx(this, pSection)))
			return false;
		}

	return true;
	}

bool CShip::IsSingletonDevice (ItemCategories iItemCat)

//	IsSingletonDevice
//
//	Returns TRUE if the given item category has a dedicated slot (e.g., shields, 
//	drive, etc.)

	{
	switch (iItemCat)
		{
		case itemcatLauncher:
		case itemcatReactor:
		case itemcatShields:
		case itemcatCargoHold:
		case itemcatDrive:
			return true;
		
		default:
			return false;
		}
	}

bool CShip::IsWeaponAligned (DeviceNames iDev, CSpaceObject *pTarget, int *retiAimAngle, int *retiFireAngle, int *retiFacingAngle)

//	IsWeaponAligned
//
//	Returns TRUE if the weapon is aligned on target

	{
	CInstalledDevice *pWeapon = GetNamedDevice(iDev);
	if (pWeapon)
		{
		int iAimAngle;
		bool bAligned = pWeapon->IsWeaponAligned(this, pTarget, &iAimAngle, retiFireAngle);

		if (retiAimAngle)
			*retiAimAngle = iAimAngle;

		//	Return the direction that the ship should face in order to fire.

		if (retiFacingAngle)
			*retiFacingAngle = (iAimAngle + 360 - pWeapon->GetRotation()) % 360;

		return bAligned;
		}
	else
		{
		if (retiAimAngle)
			*retiAimAngle = -1;

		if (retiFireAngle)
			*retiFireAngle = -1;

		if (retiFacingAngle)
			*retiFacingAngle = -1;

		return false;
		}
	}

void CShip::MakeBlind (int iTickCount)

//	MakeBlind
//
//	Ship is blind

	{
	SetAbility(ablShortRangeScanner, ablDamage, iTickCount, 0);
	}

void CShip::MakeDisarmed (int iTickCount)

//	MakeDisarmed
//
//	Ship cannot use weapons

	{
	if (m_iDisarmedTimer != -1)
		{
		if (iTickCount == -1)
			m_iDisarmedTimer = -1;
		else
			m_iDisarmedTimer += iTickCount;
		}
	}

void CShip::MakeLRSBlind (int iTickCount)

//	MakeLRSBlind
//
//	Ship is LRS blind

	{
	if (m_iLRSBlindnessTimer != -1)
		{
		if (iTickCount == -1)
			m_iLRSBlindnessTimer = -1;
		else
			m_iLRSBlindnessTimer += iTickCount;
		}
	}

void CShip::MakeParalyzed (int iTickCount)

//	MakeParalyzed
//
//	Ship is paralyzed

	{
	if (m_iParalysisTimer != -1)
		{
		if (iTickCount == -1)
			m_iParalysisTimer = -1;
		else
			m_iParalysisTimer = Min(m_iParalysisTimer + iTickCount, MAX_SHORT);
		}
	}

void CShip::MakeRadioactive (void)

//	MakeRadioactive
//
//	Ship is radioactive

	{
	if (!m_fRadioactive)
		{
		m_iContaminationTimer = (IsPlayer() ? 180 : 60) * g_TicksPerSecond;
		m_fRadioactive = true;
		m_pController->OnShipStatus(IShipController::statusRadiationWarning, m_iContaminationTimer);
		}
	}

void CShip::MarkImages (void)

//	MarkImages
//
//	Mark images that are in use.
	
	{
	DEBUG_TRY

	int i;

	//	Mark images, but do not mark default class devices (since we mark them
	//	ourselves).

	m_pClass->MarkImages(false);

	//	Mark all the installed items that we have (this will load things like
	//	shield and missile effects).

	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty())
			m_Devices[i].GetClass()->MarkImages();

	DEBUG_CATCH
	}

void CShip::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	If another object got destroyed, we do something

	{
	//	Give the controller a chance to handle it

	m_pController->OnObjDestroyed(Ctx);

	//	If what we're docked with got destroyed, clear it

	if (m_pDocked == Ctx.pObj)
		m_pDocked = NULL;

	//	If this object is docked with us, remove it from the
	//	docking table.

	if (m_pClass->HasDockingPorts())
		m_DockingPorts.OnObjDestroyed(this, Ctx.pObj);

	//	If our exit gate got destroyed, then we're OK (this can happen if
	//	a carrier gets destroyed while gunships are being launched)

	if (m_pExitGate == Ctx.pObj)
		{
		Place(m_pExitGate->GetPos());

		m_iExitGateTimer = 0;
		if (!IsVirtual())
			ClearCannotBeHit();
		FireOnEnteredSystem(m_pExitGate);
		m_pExitGate = NULL;
		}

	//	Irradiated by

	if (m_pIrradiatedBy)
		m_pIrradiatedBy->OnObjDestroyed(Ctx.pObj);
	}

bool CShip::ObjectInObject (const CVector &vObj1Pos, CSpaceObject *pObj2, const CVector &vObj2Pos)

//	ObjectInObject
//
//	Returns TRUE if the given object intersects this object

	{
	return pObj2->ImageInObject(vObj2Pos,
			m_pClass->GetImage(),
			GetSystem()->GetTick(),
			m_Rotation.GetFrameIndex(),
			vObj1Pos);
	}

void CShip::OnAscended (void)

//	OnAscended
//
//	Ship has ascended out of the system.

	{
	//	Kill any event handlers (these are often used to provide behavior in
	//	a specific system).

	SetOverride(NULL);

	//	Cancel all orders in case we're keeping pointers to other system
	//	objects.

	m_pController->CancelAllOrders();

	//	Clear out docking variables (we've already undocked, because we've 
	//	already sent an EnterGate message to all other objects in the system).

	m_DockingPorts.OnNewSystem(NULL);
	m_pDocked = NULL;
	m_pExitGate = NULL;
	}

void CShip::OnBounce (CSpaceObject *pBarrierObj, const CVector &vPos)

//	OnBounce
//
//	Ship hit a barrier

	{
	m_pController->OnHitBarrier(pBarrierObj, vPos);
	}

DWORD CShip::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	if (!IsInactive())
		return m_pController->OnCommunicate(pSender, iMessage, pParam1, dwParam2);
	else
		return resNoAnswer;
	}

void CShip::OnComponentChanged (ObjectComponentTypes iComponent)

//	OnComponentChanged
//
//	Some part of the object has changed

	{
	switch (iComponent)
		{
		case comArmor:
		case comShields:
			m_pController->OnShipStatus(IShipController::statusArmorRepaired, -1);
			break;

		case comCargo:
			{
			//	Calculate new mass
            //
            //  NOTE: In this case we defer recalculating performance until update.

			m_fRecalcItemMass = true;
            if (m_fTrackMass)
                m_fRecalcRotationAccel = true;

			//	If one of our weapons doesn't have a variant selected, then
			//	try to select it now (if we just got some new ammo, this will
			//	select the ammo)

			int i;
			for (i = 0; i < GetDeviceCount(); i++)
				{
				CInstalledDevice *pDevice = GetDevice(i);
				if (!pDevice->IsVariantSelected(this))
					pDevice->SelectFirstVariant(this);
				}

			//	Update weapons display (in case it changed)

			m_pController->OnWeaponStatusChanged();
			break;
			}

		case comDrive:
            CalcPerformance();
			break;

		case comReactor:
			CalcPerformance();
			break;

		default:
			m_pController->OnComponentChanged(iComponent);
		}
	}

EDamageResults CShip::OnDamage (SDamageCtx &Ctx)

//	Damage
//
//	Ship takes damage from the given source

	{
	int i;

	//	Roll for damage

	Ctx.iDamage = Ctx.Damage.RollDamage();
	if (Ctx.iDamage == 0)
		return damageNoDamage;

	bool bIsPlayer = IsPlayer();

	//	We're hit

	m_iLastHitTime = g_pUniverse->GetTicks();

	//	Map the direction that we got hit from to a ship-relative
	//	direction (i.e., adjust for the ship's rotation)

	int iHitDir = VectorToPolar(Ctx.vHitPos - GetPos());
	int iHitAngle = AngleMod(iHitDir - GetRotation());

	//	Figure out which section of armor got hit

	Ctx.iSectHit = m_pClass->GetHullSectionAtAngle(iHitAngle);
	CInstalledArmor *pArmor = ((Ctx.iSectHit != -1 && Ctx.iSectHit < GetArmorSectionCount()) ? GetArmorSection(Ctx.iSectHit) : NULL);

	//	Tell our controller that someone hit us

	m_pController->OnAttacked(Ctx.Attacker.GetObj(), Ctx);

	//	OnAttacked event

	if (HasOnAttackedEvent())
		{
		FireOnAttacked(Ctx);
		if (IsDestroyed())
			return damageDestroyed;
		}

	GetSystem()->FireOnSystemObjAttacked(Ctx);

	//	See if the damage is blocked by some external defense

	Ctx.iOverlayHitDamage = Ctx.iDamage;
	if (m_Overlays.AbsorbDamage(this, Ctx))
		{
		if (IsDestroyed())
			return damageDestroyed;
		else if (Ctx.iDamage == 0)
			return damageNoDamage;
		}

	//	If this is a momentum attack then we are pushed

	int iMomentum;
	if ((iMomentum = Ctx.Damage.GetMomentumDamage())
			&& m_pDocked == NULL)
		{
		CVector vAccel = PolarToVector(Ctx.iDirection, -10 * iMomentum * iMomentum);
		Accelerate(vAccel, g_MomentumConstant);
		ClipSpeed(GetMaxSpeed());
		}

	//	Let our shield generators take a crack at it

	Ctx.iShieldHitDamage = Ctx.iDamage;
	for (i = 0; i < GetDeviceCount(); i++)
		{
		bool bAbsorbed = m_Devices[i].AbsorbDamage(this, Ctx);

		//	If this is the player, report stats

		if (bIsPlayer
				&& Ctx.iAbsorb > 0)
			{
			CItem *pItem = m_Devices[i].GetItem();
			if (pItem)
				GetController()->OnItemDamaged(*pItem, Ctx.iAbsorb);
			}

		//	If destroyed or completely absorbed, we're done

		if (IsDestroyed())
			return damageDestroyed;
		else if (bAbsorbed)
			return damageAbsorbedByShields;
		}

	//	Let any overlays take damage

	if (m_Overlays.Damage(this, Ctx))
		{
		if (IsDestroyed())
			return damageDestroyed;
		else if (Ctx.iDamage == 0)
			return damageNoDamage;
		}

	//	Damage any devices that are outside the hull (e.g., Patch Spiders)
	//	Ignore devices with overlays because they get damaged in the overlay
	//	damage section.

	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty() 
				&& m_Devices[i].IsExternal()
				&& m_Devices[i].GetOverlay() == NULL)
			{
			//	The chance that the device got hit depends on the number of armor segments
			//	A device takes up 1/9th of the surface area of a segment.

			if (mathRandom(1, GetArmorSectionCount() * 9) == 7)
				DamageExternalDevice(i, Ctx);
			}

	//	Let the armor handle it

	Ctx.iArmorHitDamage = Ctx.iDamage;
	if (pArmor)
		{
		EDamageResults iResult = pArmor->AbsorbDamage(this, Ctx);

		//	If special result, we're done.

		if (iResult != damageArmorHit)
			return iResult;
		}

	//	Tell our attacker that we got hit

	CSpaceObject *pOrderGiver = Ctx.GetOrderGiver();
	if (pOrderGiver && pOrderGiver->CanAttack())
		pOrderGiver->OnObjDamaged(Ctx);

	//	If armor absorbed all the damage, then we're OK

	if (Ctx.iDamage == 0)
		{
		//	Tell the controller that we were damaged

		m_pController->OnDamaged(Ctx.Attacker, pArmor, Ctx.Damage, Ctx.iArmorHitDamage);
		return damageArmorHit;
		}

	//	Otherwise, if we have interior compartments (like a capital ship) then
	//	we do damage.

	else if (!m_Interior.IsEmpty())
		{
		EDamageResults iResult = m_Interior.Damage(this, m_pClass->GetInteriorDesc(), Ctx);
		if (iResult == damageDestroyed)
			{
			if (!OnDestroyCheck(Ctx.Damage.GetCause(), Ctx.Attacker))
				return damageArmorHit;

			Destroy(Ctx.Damage.GetCause(), Ctx.Attacker);
			}

		return iResult;
		}

	//	Otherwise we're in big trouble

	else
		{
		//	Figure out which areas of the ship got affected

		const CShipArmorSegmentDesc *pSect = ((Ctx.iSectHit != -1 && Ctx.iSectHit < m_pClass->GetHullSectionCount()) ? &m_pClass->GetHullSection(Ctx.iSectHit) : NULL);
		DWORD dwDamage = (pSect ? pSect->GetCriticalArea() : CShipClass::sectCritical);

		//	If this is a non-critical hit, then there is still a random
		//	chance that it will destroy the ship

		if (!(dwDamage & CShipClass::sectCritical))
			{
			int iChanceOfDeath = 5;

			//	We only care about mass destruction damage

            int iWMDDamage = mathAdjust(Ctx.iDamage, Ctx.Damage.GetMassDestructionAdj());

			//	Compare the amount of damage that we are taking with the
			//	original strength (HP) of the armor. Increase the chance
			//	of death appropriately.

			int iMaxHP = pArmor->GetMaxHP(this);
			if (iMaxHP > 0)
				iChanceOfDeath += 20 * iWMDDamage / iMaxHP;

			//	Roll the dice

			if (mathRandom(1, 100) <= iChanceOfDeath)
				dwDamage |= CShipClass::sectCritical;
			}

		//	Ship is destroyed!

		if (dwDamage & CShipClass::sectCritical)
			{
			if (!OnDestroyCheck(Ctx.Damage.GetCause(), Ctx.Attacker))
				return damageArmorHit;

			Destroy(Ctx.Damage.GetCause(), Ctx.Attacker);
			return damageDestroyed;
			}

		//	Otherwise, this is a non-critical hit

		else
			{
			return damageArmorHit;
			}
		}
	}

bool CShip::OnDestroyCheck (DestructionTypes iCause, const CDamageSource &Attacker)

//	OnDestroyCheck
//
//	Checks to see if any devices/powers prevent us from being destroyed.
//	Returns TRUE if the ship is destroyed; FALSE otherwise

	{
	//	Check to see if the controller can prevent the destruction
	//	(For the player, this will invoke Domina powers, if possible)

	if (!m_pController->OnDestroyCheck(iCause, Attacker))
		return false;

	return true;
	}

void CShip::OnDeviceStatus (CInstalledDevice *pDev, CDeviceClass::DeviceNotificationTypes iEvent)

//	OnDeviceStatus
//
//	Called when a device fails in some way

	{
	//	Some events we handle ourselves

	switch (iEvent)
		{
		//	If we just used up the last of our ammo, automatically select the 
		//	next missile. NOTE: This will also select the next missile for any
		//	linked launchers.

		case CDeviceClass::statusUsedLastAmmo:
			{
			CInstalledDevice *pMainLauncher = GetNamedDevice(devMissileWeapon);

			//	If this is the standard missile launcher or a linked-fire 
			//	launcher compatible with the standard missile launcher, then we
			//	select the next missile.

			if (pMainLauncher == pDev 
					|| (pMainLauncher && pMainLauncher->GetClass() == pDev->GetClass()))
				ReadyNextMissile();

			//	Otherwise, if this is a launcher, we just switch its missiles.
			//	This handles the case where we have heterogeneous missile launchers.

			else if (pDev->GetCategory() == itemcatLauncher)
				pDev->SelectNextVariant(this);

			break;
			}
		}

	//	Call controller and let it handle things.

	m_pController->OnDeviceStatus(pDev, iEvent);
	}

void CShip::OnDestroyed (SDestroyCtx &Ctx)

//	OnDestroyed
//
//	Ship has been destroyed

	{
	DEBUG_TRY

	//	Figure out if we're creating a wreck or not

	bool bCreateWreck = (Ctx.iCause != removedFromSystem)
			&& (Ctx.iCause != ascended)
			&& (Ctx.iCause != enteredStargate)
			&& (Ctx.iCause != killedByDisintegration)
			&& (Ctx.iCause != killedByShatter)
			&& (Ctx.iCause != killedByGravity)
			&& (Ctx.bResurrectPending
				|| Ctx.iCause == killedByPowerFailure
				|| Ctx.iCause == killedByRadiationPoisoning
				|| Ctx.iCause == killedByRunningOutOfFuel
				|| m_fAlwaysLeaveWreck
				|| mathRandom(1, 100) <= m_pClass->GetWreckChance());

	//	Create wreck (Note: CreateWreck may not create a wreck
	//	because not all destructions leave a wreck. In that case,
	//	pWreck will be NULL).

	if (bCreateWreck)
		m_pClass->CreateWreck(this, &Ctx.pWreck);

	//	Run OnDestroy script (we run OnObjDestroyed on items first to give
	//	them a chance to do something before the object does; e.g., some objects
	//	destroy item on their OnDestroy).

	m_Overlays.FireOnObjDestroyed(this, Ctx);
	FireOnItemObjDestroyed(Ctx);
	FireOnDestroy(Ctx);

	//	Script might have destroyed the wreck

	if (Ctx.pWreck && Ctx.pWreck->IsDestroyed())
		Ctx.pWreck = NULL;

	//	Tell controller

	m_pController->OnDestroyed(Ctx);

	//	Tell the controller that we created a wreck (Note: I'm not sure if
	//	this needs to be here--maybe we can move this to when the wreck
	//	gets created.

	if (Ctx.pWreck)
		m_pController->OnWreckCreated(Ctx.pWreck);

	//	Release any docking objects

	if (m_pClass->HasDockingPorts())
		m_DockingPorts.OnDestroyed();

	//	Create an effect appropriate to the cause of death

	switch (Ctx.iCause)
		{
		case removedFromSystem:
		case enteredStargate:
		case ascended:
			//	No effect
			break;

		case killedByPowerFailure:
		case killedByRadiationPoisoning:
		case killedByRunningOutOfFuel:
		case killedByOther:
			//	No effect
			break;

		case killedByDisintegration:
			CDisintegrationEffect::Create(GetSystem(),
					GetPos(),
					GetVel(),
					m_pClass->GetImage(),
					0,
					m_Rotation.GetFrameIndex(),
					NULL);
			break;

		case killedByGravity:
		case killedByShatter:
			{
			int iTick = GetSystem()->GetTick();
			if (IsTimeStopped())
				iTick = GetDestiny();

			const CObjectImageArray &Image = GetImage();
			if (Image.IsLoaded())
				{
				CFractureEffect *pEffect;

				CFractureEffect::Create(GetSystem(),
						GetPos(),
						GetVel(),
						Image,
						iTick,
						m_Rotation.GetFrameIndex(),
						CFractureEffect::styleExplosion,
						&pEffect);

				if (Ctx.iCause == killedByGravity && Ctx.Attacker.GetObj())
					pEffect->SetAttractor(Ctx.Attacker.GetObj());
				}

			g_pUniverse->PlaySound(this, g_pUniverse->FindSound(g_ShipExplosionSoundUNID));
			break;
			}

		default:
			m_pClass->CreateExplosion(this, Ctx.pWreck);
		}

	DEBUG_CATCH
	}

void CShip::OnDocked (CSpaceObject *pObj)

//	OnDocked
//
//	Object has docked

	{
	m_pDocked = pObj;

	//	If we've docked with a radioactive object then we become radioactive
	//	unless our armor is immune

	if (pObj->IsRadioactive() && !IsRadiationImmune())
		MakeRadioactive();

	//	Tell our controller

	m_pController->OnDocked(pObj);
	}

void CShip::OnDockedObjChanged (CSpaceObject *pLocation)

//	OnDockedObjChanged
//
//	This is called when the object that we're currently docked
//	with has changed the items that it has

	{
	m_pController->OnDockedObjChanged(pLocation);
	}

CSpaceObject *CShip::OnGetOrderGiver (void)

//	GetOrderGiver
//
//	Returns the object that is the ultimate order giver
//	(e.g., for an auton, the order giver is the ship that controls
//	the auton).

	{
	return m_pController->GetOrderGiver();
	}

void CShip::OnHitByDeviceDamage (void)

//	OnHitByDeviceDamage
//
//	A random device on the ship is damaged

	{
	//	We pick a random installed device

	CItemCriteria Criteria;
	CItem::ParseCriteria(CONSTLIT("dI +canBeDamaged:true;"), &Criteria);

	CItemListManipulator ItemList(GetItemList());
	SetCursorAtRandomItem(ItemList, Criteria);

	//	If we picked one, damage it

	if (ItemList.IsCursorValid())
		{
		DamageItem(ItemList);
		OnDeviceStatus(FindDevice(ItemList.GetItemAtCursor()), CDeviceClass::failDeviceHitByDamage);
		}
	}

void CShip::OnHitByDeviceDisruptDamage (DWORD dwDuration)

//	OnHitByDeviceDisruptDamage
//
//	A random device on the ship is disrupted

	{
	//	We pick a random installed device

	CItemCriteria Criteria;
	CItem::ParseCriteria(CONSTLIT("dI +canBeDamaged:true;"), &Criteria);

	CItemListManipulator ItemList(GetItemList());
	SetCursorAtRandomItem(ItemList, Criteria);

	//	If we picked one, disrupt it

	if (ItemList.IsCursorValid())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		CInstalledDevice *pDevice = FindDevice(Item);

		//	If the device is disabled or damaged, then nothing happens

		if (!pDevice->IsEnabled() || pDevice->IsDamaged())
			NULL;

		//	Otherwise, if the device is already disrupted, then there is
		//	a chance that it will be damaged

		else if (pDevice->IsDisrupted()
				&& pDevice->GetDisruptedDuration() > MAX_DISRUPT_TIME_BEFORE_DAMAGE)
			{
			DamageItem(ItemList);
			OnDeviceStatus(pDevice, CDeviceClass::failDamagedByDisruption);
			}

		//	Otherwise, disrupt the device

		else
			{
			DisruptItem(ItemList, dwDuration);
			OnDeviceStatus(pDevice, CDeviceClass::failDeviceHitByDisruption);
			}
		}
	}

void CShip::OnHitByRadioactiveDamage (SDamageCtx &Ctx)

//	OnHitByRadioactiveDamage
//
//	Ship was hit by radioactive damage (and armor could not repel it)
	
	{
	//	If we're not radioactive, make ourselves radioactive

	if (!IsRadioactive())
		{
		//	Remember the object that hit us so that we can report
		//	it back if/when we are destroyed.

		if (m_pIrradiatedBy == NULL)
			m_pIrradiatedBy = new CDamageSource;

		*m_pIrradiatedBy = Ctx.Attacker;
		m_pIrradiatedBy->SetCause(killedByRadiationPoisoning);

		//	Radioactive

		MakeRadioactive();
		}
	}

void CShip::OnItemEnhanced (CItemListManipulator &ItemList)

//	OnItemEnhanced
//
//	The item at the cursor has been enhanced

	{
	//	Deal with installed items

	if (ItemList.GetItemAtCursor().IsInstalled())
		{
		//	Update item mass in case the mass of this item changed

		m_fRecalcItemMass = true;

        //  Recalc performance

		CalcArmorBonus();
		CalcDeviceBonus();
        CalcPerformance();

		//	Update UI

		InvalidateItemListState();
        m_pController->OnStatsChanged();
		m_pController->OnWeaponStatusChanged();
		m_pController->OnShipStatus(IShipController::statusArmorRepaired, -1);
		}
	}

void CShip::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Do stuff when station moves

	{
	//	If the station is moving then make sure all docked ships
	//	move along with it.

	if (m_pClass->HasDockingPorts())
		m_DockingPorts.MoveAll(this);

	//	Move effects

	if (WasPainted())
		{
		bool bRecalcBounds;
		m_Effects.Move(this, vOldPos, &bRecalcBounds);

		//	Recalculate bounds, if necessary

		if (bRecalcBounds)
			CalcBounds();
		}
	}

void CShip::OnNewSystem (CSystem *pSystem)

//	OnNewSystem
//
//	Ship has moved from one system to another

	{
	//	If we have any objects docked with us, then remove them.

	m_DockingPorts.OnNewSystem(pSystem);

	//	Let the controller handle it.

	m_pController->OnNewSystem(pSystem);
	}

void CShip::OnObjEnteredGate (CSpaceObject *pObj, CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	OnObjEnteredGate
//
//	This is called when another object enters a stargate

	{
	//	Let the controller handle it

	m_pController->OnObjEnteredGate(pObj, pDestNode, sDestEntryPoint, pStargate);
	}

void CShip::OnPaint (CG32bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint the ship

	{
	int i;

	//	Figure out which effects we need to paint

	DWORD dwEffects = CalcEffectsMask();

	//	Set up context

	CViewportPaintCtxSmartSave Save(Ctx);
	Ctx.iTick = (IsTimeStopped() ? GetDestiny() : g_pUniverse->GetTicks());
	Ctx.iVariant = m_Rotation.GetFrameIndex();
	Ctx.iRotation = GetRotation();
	Ctx.iDestiny = GetDestiny();

	bool bPaintThrust = (m_pController->GetThrust() && !IsParalyzed() && ((Ctx.iTick + GetDestiny()) % 4) != 0);
	const CObjectImageArray *pImage;
	pImage = &m_pClass->GetImage();

	//	See if we're invisible in SRS

	DWORD byInvisible;
	int iRangeIndex = GetDetectionRangeIndex(Ctx.iPerception);
	if (iRangeIndex >= 6 
			&& Ctx.pCenter
			&& Ctx.pCenter != this)
		{
		Metric rRange = RangeIndex2Range(iRangeIndex);
		Metric rDist = Ctx.pCenter->GetDistance(this);
		if (rDist <= rRange)
			byInvisible = 0;
		else
			{
			byInvisible = 255 - Min(254, (int)((rDist - rRange) / g_KlicksPerPixel) * 2);
			bPaintThrust = false;
			}
		}
	else
		byInvisible = 0;

	//	Paints overlay background

	m_Overlays.PaintBackground(Dest, x, y, Ctx);

	//	Paint all effects behind the ship

	if (!byInvisible)
		{
		Ctx.bInFront = false;
		m_Effects.Paint(Ctx, m_pClass->GetEffectsDesc(), dwEffects, Dest, x, y);
		}

	//	Paint the ship

	m_pClass->Paint(Dest, 
			x, 
			y, 
			Ctx.XForm, 
			m_Rotation.GetFrameIndex(), 
			Ctx.iTick,
			bPaintThrust,
			IsRadioactive(),
			byInvisible
			);

	//	Paint effects in front of the ship.

	if (!byInvisible)
		{
		Ctx.bInFront = true;
		m_Effects.Paint(Ctx, m_pClass->GetEffectsDesc(), dwEffects, Dest, x, y);
		}

	//	Paint energy fields

	m_Overlays.Paint(Dest, pImage->GetImageViewportSize(), x, y, Ctx);

	//	If paralyzed, draw energy arcs

	if (ShowParalyzedEffect())
		{
		Metric rSize = (Metric)RectWidth(m_pClass->GetImage().GetImageRect()) / 2;
		for (i = 0; i < PARALYSIS_ARC_COUNT; i++)
			{
			//	Compute the beginning of this arc

			int iAngle = ((GetDestiny() + Ctx.iTick) * (15 + i * 7)) % 360;
			Metric rRadius = rSize * (((GetDestiny() + Ctx.iTick) * (i * 3716)) % 1000) / 1000.0;
			CVector vFrom(PolarToVector(iAngle, rRadius));

			//	Compute the end of the arc

			iAngle = ((GetDestiny() + Ctx.iTick + 1) * (15 + i * 7)) % 360;
			rRadius = rSize * (((GetDestiny() + Ctx.iTick + 1) * (i * 3716)) % 1000) / 1000.0;
			CVector vTo(PolarToVector(iAngle, rRadius));

			//	Draw

			DrawLightning(Dest,
					(int)vFrom.GetX() + x,
					(int)vFrom.GetY() + y,
					(int)vTo.GetX() + x,
					(int)vTo.GetY() + y,
					CG32bitPixel(0x00, 0xa9, 0xff),
					16,
					0.4);
			}
		}

	//	Known

	m_fKnown = true;

	//	Identified

	m_fIdentified = true;

	//	Debug info

#ifdef DEBUG
	m_pController->DebugPaintInfo(Dest, x, y, Ctx);
#endif

#ifdef DEBUG_WEAPON_POS
	for (i = 0; i < GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = GetDevice(i);
		if (!pDevice->IsEmpty()
				&& (pDevice->GetCategory() == itemcatWeapon
					|| pDevice->GetCategory() == itemcatLauncher))
			{
			CVector vPos = pDevice->GetPos(this);
			int xPos, yPos;
			Ctx.XForm.Transform(vPos, &xPos, &yPos);

			Dest.DrawDot(xPos, yPos, CG32bitPixel(255, 255, 0), markerMediumCross);
			}
		}
#endif
	}

void CShip::OnPaintAnnotations (CG32bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaintAnnotations
//
//	Paint additional annotations.

	{
	m_Overlays.PaintAnnotations(Dest, x, y, Ctx);
	}

void CShip::OnPaintMap (CMapViewportCtx &Ctx, CG32bitImage &Dest, int x, int y)

//	Paint
//
//	Paint the ship

	{
	if (IsHidden())
		return;

	//	Do not paint ships on the map unless we are the point of view

	if (GetUniverse()->GetPOV() == this)
		{
		m_pClass->PaintMap(Ctx,
				Dest, 
				x, 
				y, 
				m_Rotation.GetFrameIndex(), 
				GetSystem()->GetTick(),
				m_pController->GetThrust() && !IsParalyzed(),
				IsRadioactive()
				);
		}

	//	Or if it has docking services and the player knows about it

	else if (m_fKnown && m_pClass->HasDockingPorts())
		{
		CG32bitPixel rgbColor;
		if (IsEnemy(GetUniverse()->GetPOV()))
			rgbColor = CG32bitPixel(255, 0, 0);
		else
			rgbColor = CG32bitPixel(0, 192, 0);

		Dest.DrawDot(x+1, y+1, 0, markerSmallSquare);
		Dest.DrawDot(x, y, rgbColor, markerSmallFilledSquare);

		if (m_sMapLabel.IsBlank())
			{
			DWORD dwFlags;
			CString sName = GetName(&dwFlags);
			m_sMapLabel = ::ComposeNounPhrase(sName, 1, NULL_STR, dwFlags, nounTitleCapitalize);
			}

		g_pUniverse->GetNamedFont(CUniverse::fontMapLabel).DrawText(Dest, 
				x + MAP_LABEL_X + 1, 
				y + MAP_LABEL_Y + 1, 
				0,
				m_sMapLabel);
		g_pUniverse->GetNamedFont(CUniverse::fontMapLabel).DrawText(Dest, 
				x + MAP_LABEL_X, 
				y + MAP_LABEL_Y, 
				RGB_MAP_LABEL,
				m_sMapLabel);
		}
	}

void CShip::OnPlayerObj (CSpaceObject *pPlayer)

//	OnPlayerObj
//
//	Player has entered the system

	{
	m_pController->OnPlayerObj(pPlayer);
	}

void CShip::OnPlayerChangedShips (CSpaceObject *pOldShip, SPlayerChangedShipsCtx &Options)

//	OnPlayerChangedShips
//
//	Player has changed ships

	{
	m_pController->OnPlayerChangedShips(pOldShip, Options);
	}

void CShip::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	DWORD		Class UNID
//	DWORD		m_pSovereign (CSovereign ref)
//	CString		m_sName;
//	DWORD		m_dwNameFlags
//	CIntegralRotation	m_Rotation
//	DWORD		low = m_iFireDelay; hi = m_iMissileFireDelay
//	DWORD		low = m_iReactorGraceTimer; hi = m_iContaminationTimer
//	DWORD		low = m_iBlindnessTimer; hi = m_iParalysisTimer
//	DWORD		low = m_iExitGateTimer; hi = m_iDisarmedTimer
//	DWORD		low = m_iLRSBlindnessTimer; hi = m_iDriveDamagedTimer
//	Metric		m_rFuelLeft
//	Metric		m_rItemMass (V2)
//	Metric		m_rCargoMass
//	DWORD		m_pDocked (CSpaceObject ref)
//	DWORD		m_pExitGate (CSpaceObject ref)
//	DWORD		m_iLastFireTime
//	DWORD		m_iLastHitTime
//	DWORD		flags
//
//	CArmorSystem m_Armor
//
//	DWORD		m_iStealth
//
//	DWORD		Number of devices
//	DWORD		device: class UNID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iTimeUntilReady
//	DWORD		device: flags
//
//	DWORD		No of energy fields
//	DWORD		field: type UNID
//	DWORD		field: iLifeLeft
//	IEffectPainter field: pPainter
//
//	CShipInterior
//	CDockingPorts
//	CTradeDesc	m_pTrade
//
//	DWORD		No of registered objects
//	DWORD		registered object (CSpaceObject ref)
//	DWORD		encounter info (CStationType UNID) (V2)
//
//	CDamageSource m_pIrradiatedBy (only if flag set)
//
//	DWORD		Controlled ObjID
//	Controller Data

	{
#ifdef DEBUG_LOAD
	::OutputDebugString("CShip::OnReadFromStream\n");
#endif
	int i;
	DWORD dwLoad;

	//	Load class

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pClass = g_pUniverse->FindShipClass(dwLoad);

	//	In previous versions there was a bug in which some effects were not
	//	read properly. In those cases, the class UNID is invalid.
	//	If m_pClass is indeed NULL, then skip forward until we find
	//	a valid class

	if (m_pClass == NULL && Ctx.dwVersion < 15)
		{
		int iCount = 20;
		while (iCount-- > 0 && m_pClass == NULL)
			{
			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_pClass = g_pUniverse->FindShipClass(dwLoad);
			}
		}

	//	Load misc stuff

	CSystem::ReadSovereignRefFromStream(Ctx, &m_pSovereign);

	//	Load name

	m_sName.ReadFromStream(Ctx.pStream);
	if (Ctx.dwVersion >= 26)
		Ctx.pStream->Read((char *)&m_dwNameFlags, sizeof(DWORD));
	else
		{
		if (!m_sName.IsBlank())
			m_dwNameFlags = m_pClass->GetShipNameFlags();
		else
			m_dwNameFlags = 0;
		}

	//	Load rotation

	if (Ctx.dwVersion >= 97)
		m_Rotation.ReadFromStream(Ctx, m_pClass->GetRotationDesc());
	else
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_Rotation.Init(m_pClass->GetRotationDesc(), (int)LOWORD(dwLoad));
		}

	//	Load more

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iFireDelay = (int)LOWORD(dwLoad);
	m_iMissileFireDelay = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iReactorGraceTimer = (int)LOWORD(dwLoad);
	m_iContaminationTimer = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iBlindnessTimer = (int)LOWORD(dwLoad);
	m_iParalysisTimer = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iExitGateTimer = (int)LOWORD(dwLoad);
	m_iDisarmedTimer = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iLRSBlindnessTimer = (int)LOWORD(dwLoad);
	m_iDriveDamagedTimer = (int)HIWORD(dwLoad);
	if (Ctx.dwVersion >= 123)
		Ctx.pStream->Read((char *)&m_rFuelLeft, sizeof(Metric));
	else
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_rFuelLeft = (Metric)(int)dwLoad;
		}

	if (Ctx.dwVersion >= 2)
		Ctx.pStream->Read((char *)&m_rItemMass, sizeof(Metric));
	Ctx.pStream->Read((char *)&m_rCargoMass, sizeof(Metric));
	CSystem::ReadObjRefFromStream(Ctx, &m_pDocked);
	CSystem::ReadObjRefFromStream(Ctx, &m_pExitGate);
	if (Ctx.dwVersion >= 42)
		Ctx.pStream->Read((char *)&m_iLastFireTime, sizeof(DWORD));
	else
		m_iLastFireTime = 0;

	if (Ctx.dwVersion >= 96)
		Ctx.pStream->Read((char *)&m_iLastHitTime, sizeof(DWORD));
	else
		m_iLastHitTime = 0;

	//	Load flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fOutOfFuel =				((dwLoad & 0x00000001) ? true : false);
	m_fRadioactive =			((dwLoad & 0x00000002) ? true : false);
	m_fHasAutopilot =			((dwLoad & 0x00000004) ? true : false);
	m_fDestroyInGate =			((dwLoad & 0x00000008) ? true : false);
	m_fHalfSpeed =				((dwLoad & 0x00000010) ? true : false);
	m_fHasTargetingComputer =	((dwLoad & 0x00000020) ? true : false);
	m_fTrackFuel =				((dwLoad & 0x00000040) ? true : false);
	//	0x00000080 unused at version 77
	m_fSRSEnhanced =			((dwLoad & 0x00000100) ? true : false);
	m_fRecalcItemMass =			((dwLoad & 0x00000200) ? true : false);
	m_fKnown =					((dwLoad & 0x00000400) ? true : false);
	m_fHiddenByNebula =			((dwLoad & 0x00000800) ? true : false);
	m_fTrackMass =				((dwLoad & 0x00001000) ? true : false);
	if (Ctx.dwVersion >= 14)
		m_fIdentified =			((dwLoad & 0x00002000) ? true : false);
	else
		m_fIdentified = true;
	m_fManualSuspended =		((dwLoad & 0x00004000) ? true : false);
	bool bIrradiatedBy =		((dwLoad & 0x00008000) ? true : false);
	m_fGalacticMap =			((dwLoad & 0x00010000) ? true : false);
	m_fDockingDisabled =		((dwLoad & 0x00020000) ? true : false);
	m_fControllerDisabled =		((dwLoad & 0x00040000) ? true : false);
	m_fParalyzedByOverlay =		((dwLoad & 0x00080000) ? true : false);
	m_fDisarmedByOverlay =		((dwLoad & 0x00100000) ? true : false);
	m_fSpinningByOverlay =		((dwLoad & 0x00200000) ? true : false);
	m_fDragByOverlay =			((dwLoad & 0x00400000) ? true : false);
	m_fAlwaysLeaveWreck =		((dwLoad & 0x00800000) ? true : false);
	m_fOutOfPower =				((dwLoad & 0x01000000) ? true : false);
	m_fFriendlyFireLock =		((dwLoad & 0x02000000) ? true : false);
	m_fEmergencySpeed =			((dwLoad & 0x04000000) ? true : false);
	m_fQuarterSpeed =			((dwLoad & 0x08000000) ? true : false);

	//	We will compute CalcPerformance at the bottom anyways
	m_fRecalcRotationAccel = false;

	//	This is recomputed every tick
	m_fDeviceDisrupted = false;

	//	Load armor

    m_Armor.ReadFromStream(Ctx, this);

	//	Stealth

	if (Ctx.dwVersion >= 5)
		Ctx.pStream->Read((char *)&m_iStealth, sizeof(DWORD));
	else
		m_iStealth = stealthNormal;

	//	Initialize named devices

	for (int j = devFirstName; j < devNamesCount; j++)
		m_NamedDevices[j] = -1;

	//	Load devices

	if (Ctx.dwVersion >= 7)
		Ctx.pStream->Read((char *)&m_iDeviceCount, sizeof(DWORD));
	else
		{
		CDeviceDescList Devices;
		m_pClass->GenerateDevices(1, Devices);
		m_iDeviceCount = Max(Devices.GetCount(), m_pClass->GetMaxDevices());
		}

	m_Devices = new CInstalledDevice [m_iDeviceCount];
	for (i = 0; i < m_iDeviceCount; i++)
		{
		m_Devices[i].ReadFromStream(this, Ctx);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad != 0xffffffff)
			m_NamedDevices[dwLoad] = i;

		if (Ctx.dwVersion < 29)
			m_Devices[i].SetDeviceSlot(i);
		}

	//	Previous versions stored drive desc UNID

    if (Ctx.dwVersion < 127)
	    Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

	//	Energy fields

	m_Overlays.ReadFromStream(Ctx, this);

	//	Ship interior

	if (Ctx.dwVersion >= 91)
		m_Interior.ReadFromStream(this, m_pClass->GetInteriorDesc(), Ctx);

	//	Docking ports and registered objects

	m_DockingPorts.ReadFromStream(this, Ctx);

#ifdef DEBUG
	if (m_pClass->HasDockingPorts()
			&& m_DockingPorts.GetPortCount(this) == 0)
		{
		m_DockingPorts.InitPorts(this, m_pClass->GetDockingPortPositions());
		}
#endif

	//	Trade desc

	if (Ctx.dwVersion >= 105)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad != 0xffffffff)
			{
			m_pTrade = new CTradingDesc;
			m_pTrade->ReadFromStream(Ctx);
			}
		else
			m_pTrade = NULL;
		}
	else
		m_pTrade = NULL;

	//	Registered objects / subscriptions

	if (Ctx.dwVersion < 77)
		{
		DWORD dwCount;
		Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
		if (dwCount)
			{
			for (i = 0; i < (int)dwCount; i++)
				{
				DWORD dwObjID;
				Ctx.pStream->Read((char *)&dwObjID, sizeof(DWORD));

				TArray<CSpaceObject *> *pList = Ctx.Subscribed.SetAt(dwObjID);
				pList->Insert(this);
				}
			}
		}

	//	Encounter UNID

	if (Ctx.dwVersion >= 2)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad)
			m_pEncounterInfo = g_pUniverse->FindStationType(dwLoad);
		}

	//	Calculate item mass, if appropriate. We do this for previous versions
	//	that did not store item mass.
	
	if (Ctx.dwVersion < 2)
		m_rItemMass = CalculateItemMass(&m_rCargoMass);

	//	Irradiation source

	if (bIrradiatedBy)
		{
		m_pIrradiatedBy = new CDamageSource;
		m_pIrradiatedBy->ReadFromStream(Ctx);
		}
	else
		m_pIrradiatedBy = NULL;

	//	Controller

	if (Ctx.dwVersion >= 108)
		{
		CString sAI;
		sAI.ReadFromStream(Ctx.pStream);
		m_pController = g_pUniverse->CreateShipController(sAI);
		}
	else
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

		//	In previous versions we saved the object ID, so we need to convert:

		switch (dwLoad)
			{
			case CONTROLLER_STANDARDAI:
				m_pController = new CStandardShipAI;
				break;

			case CONTROLLER_FLEETSHIPAI:
				m_pController = new CFleetShipAI;
				break;

			case CONTROLLER_FERIANSHIPAI:
				m_pController = new CFerianShipAI;
				break;

			case CONTROLLER_AUTONAI:
				m_pController = new CAutonAI;
				break;

			case CONTROLLER_GLADIATORAI:
				m_pController = new CGladiatorAI;
				break;

			case CONTROLLER_FLEETCOMMANDAI:
				m_pController = new CFleetCommandAI;
				break;

			case CONTROLLER_GAIANPROCESSORAI:
				m_pController = new CGaianProcessorAI;
				break;

			case CONTROLLER_ZOANTHROPEAI:
				m_pController = new CZoanthropeAI;
				break;

			case CONTROLLER_PLAYERSHIP:
				m_pController = g_pUniverse->CreateShipController(CONSTLIT("player"));
				break;

			default:
				::kernelDebugLogMessage("Unable to find AI controller: %x.", dwLoad);
				m_pController = NULL;
				ASSERT(false);
			}
		}

	m_pController->ReadFromStream(Ctx, this);

    //  Recompute performance (because we don't save it).

    CalcPerformance();

	//	Initialize effects

	m_pClass->InitEffects(this, &m_Effects);
	}

void CShip::OnSetEventFlags (void)

//	OnGetEventFlags
//
//	Sets the flags the cache whether the object has certain events

	{
	SetHasInterSystemEvent(FindEventHandler(CONSTLIT("OnPlayerLeftSystem")) 
			|| FindEventHandler(CONSTLIT("OnPlayerEnteredSystem")));
	SetHasOnOrderChangedEvent(FindEventHandler(CONSTLIT("OnOrderChanged")));
	SetHasOnOrdersCompletedEvent(FindEventHandler(CONSTLIT("OnOrdersCompleted")));
	SetHasOnSubordinateAttackedEvent(FindEventHandler(CONSTLIT("OnSubordinateAttacked")));
	}

void CShip::OnStationDestroyed (const SDestroyCtx &Ctx)

//	OnStationDestroyed
//
//	Station in the system has been destroyed

	{
	m_pController->OnStationDestroyed(Ctx);
	}

void CShip::OnSystemCreated (SSystemCreateCtx &CreateCtx)

//	OnSystemCreated
//
//	The system has just been created

	{
	FinishCreation(m_pDeferredOrders, &CreateCtx);
	}

void CShip::OnSystemLoaded (void)

//	OnSystemLoaded
//
//	The system has just been loaded

	{
	m_pController->OnSystemLoaded(); 
	}

void CShip::OnUpdate (SUpdateCtx &Ctx, Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

    {
    DEBUG_TRY

    int i;
    bool bOverlaysChanged = false;
    bool bWeaponStatusChanged = false;
    bool bArmorStatusChanged = false;
    bool bCalcDeviceBonus = false;
    bool bCargoChanged = false;
    bool bCalcPerformance = false;
    bool bBoundsChanged = false;

    //	If we passed through a gate, then destroy ourselves

    if (m_fDestroyInGate)
        {
		UpdateDestroyInGate();
        return;
        }

    //	If we're in a gate, then all we do is update the in-gate timer

    if (IsInactive())
        {
		UpdateInactive();
        return;
        }

    //	Allow docking

    if (m_pClass->HasDockingPorts())
        m_DockingPorts.UpdateAll(Ctx, this);

    //	Initialize

    int iTick = GetSystem()->GetTick() + GetDestiny();

    //	Trade

    if ((iTick % TRADE_UPDATE_FREQUENCY) == 0)
        UpdateTrade(Ctx, INVENTORY_REFRESHED_PER_UPDATE);

    //	Check controls

    if (!IsParalyzed())
        {
        //	See if we're firing. Note that we fire before we rotate so that the
        //	fire behavior code can know which way we're aiming.

        if (!IsDisarmed())
            {
            STargetingCtx TargetingCtx;

            for (i = 0; i < GetDeviceCount(); i++)
                {
                CInstalledDevice *pDevice = GetDevice(i);
                if (pDevice->IsTriggered() && pDevice->IsReady())
                    {
                    CItemCtx DeviceCtx(this, pDevice);
                    bool bSourceDestroyed = false;
                    bool bConsumedItems = false;

                    //	Compute the target for this weapon.

                    CSpaceObject *pTarget;
                    int iFireAngle;
                    if (!CalcDeviceTarget(TargetingCtx, DeviceCtx, &pTarget, &iFireAngle))
                        {
                        //	Do not consume power, even though we're triggered.

                        pDevice->SetLastActivateSuccessful(false);
                        continue;
                        }

                    //	Set the target on the device. We need to do this for 
                    //	repeating weapons.

                    pDevice->SetFireAngle(iFireAngle);
                    pDevice->SetTarget(pTarget);

                    //	Fire

                    bool bSuccess = pDevice->Activate(this,
                        pTarget,
                        &bSourceDestroyed,
                        &bConsumedItems);
                    if (IsDestroyed())
                        return;

                    //	If we succeeded then it means that the weapon fired successfully

                    if (bSuccess)
                        {
                        if (bConsumedItems)
                            {
                            bWeaponStatusChanged = true;
                            bCargoChanged = true;
                            }

                        //	Remember the last time we fired a weapon

                        if (pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher)
                            m_iLastFireTime = g_pUniverse->GetTicks();

                        //	Set delay for next activation

                        SetFireDelay(pDevice);
                        }
                    }
                }
            }
        else
            {
            if (m_iDisarmedTimer > 0)
                m_iDisarmedTimer--;
            }

        //	Update rotation

        m_Rotation.Update(m_Perf.GetRotationDesc(), m_pController->GetManeuver());

        //	See if we're accelerating

        if (IsInertialess())
            {
            if (m_pController->GetThrust())
                {
                CVector vVel = PolarToVector(GetRotation(), GetMaxSpeed());
                SetVel(vVel);
                }
            else if (CanBeControlled())
                ClipSpeed(0.0);
            }
        else if (m_pController->GetThrust())
            {
            CVector vAccel = PolarToVector(GetRotation(), GetThrust());

            Accelerate(vAccel, rSecondsPerTick);

            //	Check to see if we're exceeding the ship's speed limit. If we
            //	are then adjust the speed

            ClipSpeed(GetMaxSpeed());
            }
        else if (m_pController->GetStopThrust())
            {
            //	Stop thrust is proportional to main engine thrust and maneuverability

            Metric rManeuverAdj = Min((Metric)0.5, m_Perf.GetRotationDesc().GetManeuverRatio());
            Metric rThrust = rManeuverAdj * GetThrust();

            AccelerateStop(rThrust, rSecondsPerTick);
            }
        }

    //	If we're paralyzed, rotate in one direction

    else if (ShowParalyzedEffect())
        {
        //	Rotate wildly

        if (m_pDocked == NULL)
            m_Rotation.Update(m_Perf.GetRotationDesc(), ((GetDestiny() % 2) ? RotateLeft : RotateRight));

        //	Slow down

        SetVel(CVector(GetVel().GetX() * g_SpaceDragFactor, GetVel().GetY() * g_SpaceDragFactor));

        if (m_iParalysisTimer > 0)
            m_iParalysisTimer--;
        }

    //	Otherwise, we're paralyzed (by an overlay most-likely)

    else
        {
        //	Spin wildly

        if (m_pDocked == NULL && m_fSpinningByOverlay)
            m_Rotation.Update(m_Perf.GetRotationDesc(), ((GetDestiny() % 2) ? RotateLeft : RotateRight));
        }

    //	Slow down if an overlay is imposing drag

    if (m_fDragByOverlay
            && !ShowParalyzedEffect())
        {
        //	We're too lazy to store the drag coefficient, so we recalculate it here.
        //
        //	(Since it's rare to have this, it shouldn't been too much of a performance
        //	penalty. But if necessary we have add a special function to just get the
        //	drag coefficient from the overlay list.)

        COverlayList::SImpactDesc Impact;
        m_Overlays.GetImpact(this, &Impact);

        SetVel(CVector(GetVel().GetX() * Impact.rDrag, GetVel().GetY() * Impact.rDrag));
        }

    //	Drive damage timer

    if (m_iDriveDamagedTimer > 0
            && (--m_iDriveDamagedTimer == 0))
        {
        //	If drive is repaired, update performance

        bCalcPerformance = true;
        }

    //	Update armor

    if ((iTick % ARMOR_UPDATE_CYCLE) == 0)
        {
        for (i = 0; i < GetArmorSectionCount(); i++)
            {
            bool bModified;
            CInstalledArmor *pArmor = GetArmorSection(i);
            pArmor->GetClass()->Update(pArmor, this, iTick, &bModified);
            if (bModified)
                bArmorStatusChanged = true;
            }
        }

    //	Update each device

	CDeviceClass::SDeviceUpdateCtx DeviceCtx(iTick);
    m_fDeviceDisrupted = false;
    for (i = 0; i < GetDeviceCount(); i++)
        {
		DeviceCtx.ResetOutputs();

        m_Devices[i].Update(this, DeviceCtx);

        if (DeviceCtx.bSourceDestroyed)
            return;

        if (DeviceCtx.bConsumedItems)
            {
            bWeaponStatusChanged = true;
            bCargoChanged = true;
            }

        if (DeviceCtx.bDisrupted)
            m_fDeviceDisrupted = true;

		//	If the device was repaired, then we need to recalc performance, etc.

		if (DeviceCtx.bRepaired)
			bCalcDeviceBonus = true;

		if (DeviceCtx.bSetDisabled && m_Devices[i].SetEnabled(this, false))
			{
			bCalcDeviceBonus = true;
			bCalcPerformance = true;

			bWeaponStatusChanged = true;
			bArmorStatusChanged = true;

			m_pController->OnDeviceEnabledDisabled(i, false);
			}
        }

    //	Update reactor

	if (m_fTrackFuel)
		if (!UpdateFuel(iTick))
			return;

    //	Radiation

    if (m_fRadioactive)
        {
        m_iContaminationTimer--;
        if (m_iContaminationTimer > 0)
            {
			m_pController->OnShipStatus(IShipController::statusRadiationWarning, m_iContaminationTimer);
            }
        else
            {
            if (m_pIrradiatedBy)
                Destroy(m_pIrradiatedBy->GetCause(), *m_pIrradiatedBy);
            else
                Destroy(killedByRadiationPoisoning, CDamageSource(NULL, killedByRadiationPoisoning));

            //	Shouldn't do anything else after being destroyed
            return;
            }
        }

    //	Blindness

    if (m_iBlindnessTimer > 0)
        if (--m_iBlindnessTimer == 0)
            m_pController->OnBlindnessChanged(false);

    //	LRS blindness

    if (m_iLRSBlindnessTimer > 0)
        --m_iLRSBlindnessTimer;

    //	Energy fields

    if (!m_Overlays.IsEmpty())
        {
        bool bModified;

        m_Overlays.Update(this, m_pClass->GetImage().GetImageViewportSize(), GetRotation(), &bModified);
        if (CSpaceObject::IsDestroyedInUpdate())
            return;
        else if (bModified)
            {
            bOverlaysChanged = true;
            bWeaponStatusChanged = true;
            bArmorStatusChanged = true;
            bCalcDeviceBonus = true;
            bBoundsChanged = true;
            }
        }

    //	Check space environment

    if ((iTick % ENVIRONMENT_ON_UPDATE_CYCLE) == ENVIRONMENT_ON_UPDATE_OFFSET)
        {
        CSpaceEnvironmentType *pEnvironment = GetSystem()->GetSpaceEnvironment(GetPos());
        if (pEnvironment)
            {
            //	See if our LRS is affected

            if (pEnvironment->IsLRSJammer())
                {
                MakeLRSBlind(ENVIRONMENT_ON_UPDATE_CYCLE);
                m_fHiddenByNebula = true;
                }
            else
                m_fHiddenByNebula = false;

            //	See if the environment causes drag

            Metric rDrag = pEnvironment->GetDragFactor();
            if (rDrag != 1.0)
                {
                SetVel(CVector(GetVel().GetX() * rDrag,
                    GetVel().GetY() * rDrag));
                }

            //	Update

            if (pEnvironment->HasOnUpdateEvent())
                {
                CString sError;
                if (pEnvironment->FireOnUpdate(this, &sError) != NOERROR)
                    SendMessage(NULL, sError);
                }
            }
        else
            {
            m_fHiddenByNebula = false;
            }
        }

    //	Update effects. For efficiency, we only update our effects if we painted
    //	last tick.

    if (WasPainted())
        m_Effects.Update(this, m_pClass->GetEffectsDesc(), GetRotation(), CalcEffectsMask());

    //	Invalidate

    if (bBoundsChanged)
        CalcBounds();

    if (bOverlaysChanged)
        CalcOverlayImpact();

    if (bCalcDeviceBonus)
        CalcDeviceBonus();

	if (m_fTrackMass && bCargoChanged)
		OnComponentChanged(comCargo);

    if (bOverlaysChanged || bCalcDeviceBonus || bArmorStatusChanged || m_fRecalcRotationAccel || bCalcPerformance)
        CalcPerformance();

    if (bOverlaysChanged || bCalcDeviceBonus || bArmorStatusChanged || bCalcPerformance)
        m_pController->OnStatsChanged();

	if (bWeaponStatusChanged)
		m_pController->OnWeaponStatusChanged();

	if (bArmorStatusChanged)
		m_pController->OnShipStatus(IShipController::statusArmorRepaired, -1);
	
	if (bCargoChanged)
		InvalidateItemListAddRemove();

	//	Update

	if (m_iFireDelay > 0)
		m_iFireDelay--;

	if (m_iMissileFireDelay > 0)
		m_iMissileFireDelay--;

	DEBUG_CATCH
	}

void CShip::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	DWORD		Class UNID
//	DWORD		m_pSovereign (CSovereign ref)
//	CString		m_sName
//	DWORD		m_dwNameFlags
//	CIntegralRotation m_Rotation
//	DWORD		low = m_iFireDelay; hi = m_iMissileFireDelay
//	DWORD		low = m_iReactorGraceTimer; hi = m_iContaminationTimer
//	DWORD		low = m_iBlindnessTimer; hi = m_iParalysisTimer
//	DWORD		low = m_iExitGateTimer; hi = m_iDisarmedTimer
//	DWORD		low = m_iLRSBlindnessTimer; hi = m_iDriveDamagedTimer
//	Metric		m_rFuelLeft
//	Metric		m_rItemMass
//	Metric		m_rCargoMass
//	DWORD		m_pDocked (CSpaceObject ref)
//	DWORD		m_pExitGate (CSpaceObject ref)
//	DWORD		m_iLastFireTime
//	DWORD		m_iLastHitTime
//	DWORD		flags
//
//  CArmorSystem m_Armor
//
//	DWORD		m_iStealth
//
//	DWORD		device: class UNID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iTimeUntilReady
//	DWORD		device: flags
//	DWORD		named device index (0xffffffff if not named)
//
//	DWORD		drivedesc UNID
//	
//	DWORD		No of energy fields
//	DWORD		field: type UNID
//	DWORD		field: iLifeLeft
//
//	CShipInterior
//	CDockingPorts
//
//	DWORD		No of registered objects
//	DWORD		registered object (CSpaceObject ref)
//	DWORD		encounter info (CStationType UNID)
//
//	CDamageSource m_pIrradiatedBy (only if flag set)
//
//	DWORD		Controller ObjID
//	Controller Data

	{
	int i;
	DWORD dwSave;

	dwSave = m_pClass->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);

	m_sName.WriteToStream(pStream);
	pStream->Write((char *)&m_dwNameFlags, sizeof(DWORD));

	m_Rotation.WriteToStream(pStream);

	dwSave = MAKELONG(m_iFireDelay, m_iMissileFireDelay);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iReactorGraceTimer, m_iContaminationTimer);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iBlindnessTimer, m_iParalysisTimer);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iExitGateTimer, m_iDisarmedTimer);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iLRSBlindnessTimer, m_iDriveDamagedTimer);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_rFuelLeft, sizeof(Metric));
	pStream->Write((char *)&m_rItemMass, sizeof(Metric));
	pStream->Write((char *)&m_rCargoMass, sizeof(Metric));
	WriteObjRefToStream(m_pDocked, pStream);
	WriteObjRefToStream(m_pExitGate, pStream);
	pStream->Write((char *)&m_iLastFireTime, sizeof(DWORD));
	pStream->Write((char *)&m_iLastHitTime, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_fOutOfFuel ?			0x00000001 : 0);
	dwSave |= (m_fRadioactive ?			0x00000002 : 0);
	dwSave |= (m_fHasAutopilot ?		0x00000004 : 0);
	dwSave |= (m_fDestroyInGate ?		0x00000008 : 0);
	dwSave |= (m_fHalfSpeed ?			0x00000010 : 0);
	dwSave |= (m_fHasTargetingComputer ? 0x00000020 : 0);
	dwSave |= (m_fTrackFuel ?			0x00000040 : 0);
	//	0x00000080 unused at version 77
	dwSave |= (m_fSRSEnhanced ?			0x00000100 : 0);
	dwSave |= (m_fRecalcItemMass ?		0x00000200 : 0);
	dwSave |= (m_fKnown ?				0x00000400 : 0);
	dwSave |= (m_fHiddenByNebula ?		0x00000800 : 0);
	dwSave |= (m_fTrackMass ?			0x00001000 : 0);
	dwSave |= (m_fIdentified ?			0x00002000 : 0);
	dwSave |= (m_fManualSuspended ?		0x00004000 : 0);
	dwSave |= (m_pIrradiatedBy ?		0x00008000 : 0);
	dwSave |= (m_fGalacticMap ?			0x00010000 : 0);
	dwSave |= (m_fDockingDisabled ?		0x00020000 : 0);
	dwSave |= (m_fControllerDisabled ?	0x00040000 : 0);
	dwSave |= (m_fParalyzedByOverlay ?	0x00080000 : 0);
	dwSave |= (m_fDisarmedByOverlay ?	0x00100000 : 0);
	dwSave |= (m_fSpinningByOverlay ?	0x00200000 : 0);
	dwSave |= (m_fDragByOverlay ?		0x00400000 : 0);
	dwSave |= (m_fAlwaysLeaveWreck ?	0x00800000 : 0);
	dwSave |= (m_fOutOfPower ?			0x01000000 : 0);
	dwSave |= (m_fFriendlyFireLock ?	0x02000000 : 0);
	dwSave |= (m_fEmergencySpeed ?		0x04000000 : 0);
	dwSave |= (m_fQuarterSpeed ?		0x08000000 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Armor

    m_Armor.WriteToStream(pStream);

	//	Stealth

	pStream->Write((char *)&m_iStealth, sizeof(DWORD));

	//	Devices

	dwSave = GetDeviceCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	for (i = 0; i < GetDeviceCount(); i++)
		{
		m_Devices[i].WriteToStream(pStream);

		int j;
		for (j = devFirstName; j < devNamesCount; j++)
			if (m_NamedDevices[j] == i)
				break;

		dwSave = ((j == devNamesCount) ? 0xffffffff : j);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	//	Energy fields

	m_Overlays.WriteToStream(pStream);

	//	Ship interior

	m_Interior.WriteToStream(pStream);

	//	Docking ports

	m_DockingPorts.WriteToStream(this, pStream);

	//	Trade desc

	if (m_pTrade)
		{
		dwSave = 1;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		m_pTrade->WriteToStream(pStream);
		}
	else
		{
		dwSave = 0xffffffff;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	//	Encounter info

	dwSave = (m_pEncounterInfo ? m_pEncounterInfo->GetUNID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Irradiated by

	if (m_pIrradiatedBy)
		m_pIrradiatedBy->WriteToStream(GetSystem(), pStream);

	//	Controller

	if (m_pController)
		m_pController->WriteToStream(pStream);
	else
		{
		dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}

bool CShip::OrientationChanged (void)

//	OrientationChanged
//
//	Returns TRUE if the object's orientation changed this tick

	{
	return false;
	}

void CShip::PaintLRSBackground (CG32bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRSBackground
//
//	Paints the object on an LRS

	{
	DEBUG_TRY

	if (IsHidden())
		return;

	if (m_fKnown && m_pClass->HasDockingPorts())
		{
		if (m_sMapLabel.IsBlank())
			{
			DWORD dwFlags;
			CString sName = GetName(&dwFlags);
			m_sMapLabel = ::ComposeNounPhrase(sName, 1, NULL_STR, dwFlags, nounTitleCapitalize);
			}

		g_pUniverse->GetNamedFont(CUniverse::fontMapLabel).DrawText(Dest, 
				x + MAP_LABEL_X, 
				y + MAP_LABEL_Y, 
				RGB_LRS_LABEL,
				m_sMapLabel);
		}

	DEBUG_CATCH_MSG1("Crash in CShip::PaintLRSBackground: type: %08x", m_pClass->GetUNID());
	}

void CShip::PaintLRSForeground (CG32bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRS
//
//	Paints the object on an LRS

	{
	DEBUG_TRY

	if (IsHidden())
		return;

	//	Object is known if we can scan it.

	SetKnown();

	//	Paint red if enemy, blue otherwise

	CG32bitPixel rgbColor = GetSymbolColor();
	Dest.DrawDot(x, y, 
			rgbColor, 
			markerSmallRound);

	//	Identified

	m_fIdentified = true;

	DEBUG_CATCH_MSG1("Crash in CShip::PaintLRSForeground: type: %08x", m_pClass->GetUNID());
	}

bool CShip::PointInObject (const CVector &vObjPos, const CVector &vPointPos)

//	PointInObject
//
//	Returns TRUE if the given point is inside the object

	{
	DEBUG_TRY

	//	Figure out the coordinates of vPos relative to the center of the
	//	ship, in pixels.

	CVector vOffset = vPointPos - vObjPos;
	int x = (int)((vOffset.GetX() / g_KlicksPerPixel) + 0.5);
	int y = -(int)((vOffset.GetY() / g_KlicksPerPixel) + 0.5);

	//	Ask the image if the point is inside or not

	return m_pClass->GetImage().PointInImage(x, y, GetSystem()->GetTick(), m_Rotation.GetFrameIndex());

	DEBUG_CATCH
	}

bool CShip::PointInObject (SPointInObjectCtx &Ctx, const CVector &vObjPos, const CVector &vPointPos)

//	PointInObject
//
//	Returns TRUE if the given point is inside the object

	{
	DEBUG_TRY

	//	Figure out the coordinates of vPos relative to the center of the
	//	ship, in pixels.

	CVector vOffset = vPointPos - vObjPos;
	int x = (int)((vOffset.GetX() / g_KlicksPerPixel) + 0.5);
	int y = -(int)((vOffset.GetY() / g_KlicksPerPixel) + 0.5);

	//	Ask the image if the point is inside or not

	return m_pClass->GetImage().PointInImage(Ctx, x, y);

	DEBUG_CATCH
	}

void CShip::PointInObjectInit (SPointInObjectCtx &Ctx)

//	PointInObjectInit
//
//	Initializes context for PointInObject (for improved performance in loops)

	{
	m_pClass->GetImage().PointInImageInit(Ctx, GetSystem()->GetTick(), m_Rotation.GetFrameIndex());
	}

void CShip::ProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program)

//	ProgramDamage
//
//	Take damage from a program

	{
	switch (Program.iProgram)
		{
		case progShieldsDown:
			{
			CInstalledDevice *pShields = GetNamedDevice(devShields);
			if (pShields)
				{
				//	The chance of success is 50% plus 10% for every level
				//	that the program is greater than the shields

				int iSuccess = 50 + 10 * (Program.iAILevel - pShields->GetLevel());
				if (mathRandom(1, 100) <= iSuccess)
					pShields->Deplete(this);
				}

			break;
			}

		case progDisarm:
			{
			CInstalledDevice *pWeapon = GetNamedDevice(devPrimaryWeapon);
			if (pWeapon)
				{
				//	The chance of success is 50% plus 10% for every level
				//	that the program is greater than the primary weapon

				int iSuccess = 50 + 10 * (Program.iAILevel - pWeapon->GetLevel());
				if (mathRandom(1, 100) <= iSuccess)
					MakeDisarmed(Program.iAILevel * mathRandom(30, 60));
				}
			break;
			}

		case progReboot:
			break;

		case progCustom:
			{
			//	The chance of success is based on the AI level of the hacker
			//	and the cyber defense levels of the target (us)

			int iChance = 90 + 10 * (Program.iAILevel - GetCyberDefenseLevel());

			//	If the program succeeds, execute the program

			if (mathRandom(1, 100) <= iChance)
				Program.pCtx->pCC->Eval(Program.pCtx, Program.ProgramCode);

			break;
			}
		}

	//	Tell controller that we were attacked

	m_pController->OnProgramDamage(pHacker, Program);
	}

void CShip::ReactorOverload (void)

//	ReactorOverload
//
//	This is called every FUEL_CHECK_CYCLE when the reactor is overloading

	{
	int i;

	//	There is a 1 in 10 chance that something bad will happen
	//	(or, if the overload is severe, something bad always happens)

	if (mathRandom(1, 100) <= 10
			|| (m_iPowerDrain > 2 * m_Perf.GetReactorDesc().GetMaxPower()))
		{
		//	See if there is an OnReactorOverload event that will
		//	handle this.

		CInstalledDevice *pReactor = GetNamedDevice(devReactor);
		CItem *pReactorItem = (pReactor ? pReactor->GetItem() : NULL);
		if (pReactorItem && pReactorItem->FireOnReactorOverload(this))
			return;

		//	Choose the device that drains the most

		CItemListManipulator ItemList(GetItemList());

		int iBestDev = -1;
		int iBestPower = 0;
		for (i = 0; i < GetDeviceCount(); i++)
			{
			if (!m_Devices[i].IsEmpty() 
					&& m_Devices[i].IsEnabled()
					&& m_Devices[i].CanBeDisabled(CItemCtx(this, &m_Devices[i])))
				{
				SetCursorAtDevice(ItemList, i);

				CItem Item = ItemList.GetItemAtCursor();
				int iDevicePower = m_Devices[i].GetPowerRating(CItemCtx(&Item, this));
				if (iDevicePower > iBestPower)
					{
					iBestDev = i;
					iBestPower = iDevicePower;
					}
				}
			}

		//	Disable the device

		if (iBestDev != -1)
			EnableDevice(iBestDev, false);
		}
	}

void CShip::ReadyFirstMissile (void)

//	ReadyFirstMissile
//
//	Selects the first missile

	{
	int i;

	CInstalledDevice *pDevice = GetNamedDevice(devMissileWeapon);
	if (pDevice)
		{
		pDevice->SelectFirstVariant(this);

		//	If we have any linked missile launchers, then select them also.

		for (i = 0; i < GetDeviceCount(); i++)
			{
			CInstalledDevice *pLinkedDevice = GetDevice(i);
			CItemCtx Ctx(this, pLinkedDevice);

			if (!pLinkedDevice->IsEmpty()
					&& pLinkedDevice != pDevice
					&& pLinkedDevice->GetClass() == pDevice->GetClass()
					&& pLinkedDevice->IsLinkedFire(Ctx, itemcatLauncher))
				pLinkedDevice->SelectFirstVariant(this);
			}
		}
	}

void CShip::ReadyFirstWeapon (void)

//	ReadyFirstWeapon
//
//	Selects the first primary weapon

	{
	int iNextWeapon = FindNextDevice(-1, itemcatWeapon);
	if (iNextWeapon != -1)
		{
		m_NamedDevices[devPrimaryWeapon] = iNextWeapon;

		CInstalledDevice *pDevice = GetNamedDevice(devPrimaryWeapon);
		CDeviceClass *pClass = pDevice->GetClass();
		pClass->ValidateSelectedVariant(this, pDevice);
		}
	}

void CShip::ReadyNextMissile (int iDir)

//	ReadyNextMissile
//
//	Selects the next missile

	{
	int i;

	CInstalledDevice *pDevice = GetNamedDevice(devMissileWeapon);
	if (pDevice)
		{
		pDevice->SelectNextVariant(this, iDir);

		//	If we have any linked missile launchers, then select them also

		for (i = 0; i < GetDeviceCount(); i++)
			{
			CInstalledDevice *pLinkedDevice = GetDevice(i);
			CItemCtx Ctx(this, pLinkedDevice);

			if (!pLinkedDevice->IsEmpty()
					&& pLinkedDevice != pDevice
					&& pLinkedDevice->GetClass() == pDevice->GetClass()
					&& pLinkedDevice->IsLinkedFire(Ctx, itemcatLauncher))
				pLinkedDevice->SelectNextVariant(this, iDir);
			}
		}
	}

void CShip::ReadyNextWeapon (int iDir)

//	ReadyNextWeapon
//
//	Selects the next primary weapon

	{
	int iNextWeapon = FindNextDevice(m_NamedDevices[devPrimaryWeapon], itemcatWeapon, iDir);
	if (iNextWeapon != -1)
		{
		m_NamedDevices[devPrimaryWeapon] = iNextWeapon;

		CInstalledDevice *pDevice = GetNamedDevice(devPrimaryWeapon);
		CDeviceClass *pClass = pDevice->GetClass();
		pClass->ValidateSelectedVariant(this, pDevice);
		}
	}

void CShip::RechargeItem (CItemListManipulator &ItemList, int iCharges)

//	RechargeItem
//
//	Recharges the given item

	{
	const CItem &Item = ItemList.GetItemAtCursor();

	//	Set charges

	int iNewCharges = Max(0, ItemList.GetItemAtCursor().GetCharges() + iCharges);
	ItemList.SetChargesAtCursor(iNewCharges, 1);
	InvalidateItemListState();

	//	If the item is installed, then we update bonuses

	if (Item.IsInstalled())
		{
        if (Item.IsDevice())
            {
            CalcDeviceBonus();

            m_pController->OnStatsChanged();
            m_pController->OnWeaponStatusChanged();
            }
		}
	}

void CShip::Refuel (Metric rFuel)

//	Refuel
//
//	Refuels the ship

	{
	if (m_fOutOfFuel)
		{
		m_rFuelLeft = 0.0;
		m_fOutOfFuel = false;
		}

	m_rFuelLeft = Min(GetMaxFuel(), m_rFuelLeft + rFuel);
	}

void CShip::Refuel (const CItem &Fuel)

//	Refuel
//
//	Refuels the ship

	{
	CItemType *pFuelType = Fuel.GetType();
	Metric rFuelPerItem = strToDouble(pFuelType->GetData(), 0.0, NULL);
	Metric rFuel = rFuelPerItem * Fuel.GetCount();

	Refuel(rFuel);

	//	Invoke refueling code

	SEventHandlerDesc Event;
	if (pFuelType->FindEventHandlerItemType(CItemType::evtOnRefuel, &Event))
		{
		CCodeChainCtx Ctx;

		//	Define parameters

		Ctx.SaveAndDefineSourceVar(this);
		Ctx.SaveAndDefineItemVar(Fuel);

		ICCItem *pResult = Ctx.Run(Event);
		if (pResult->IsError())
			SendMessage(NULL, pResult->GetStringValue());

		Ctx.Discard(pResult);
		}
	}

ALERROR CShip::RemoveItemAsDevice (CItemListManipulator &ItemList)

//	RemoveItemAsDevice
//
//	Uninstalls the item.
//
//	Note: Our callers rely on the fact that we leave the item cursor on the
//	uninstalled item.

	{
	//	Get the item at the cursor

	const CItem &Item = ItemList.GetItemAtCursor();
	CItemType *pType = Item.GetType();

	//	Validate it

	if (pType == NULL)
		return ERR_FAIL;

	if (!Item.IsInstalled())
		return ERR_FAIL;

	if (pType->GetDeviceClass() == NULL)
		return ERR_FAIL;

	//	Get the device slot that this item is at

	int iDevSlot = Item.GetInstalled();
	CInstalledDevice *pDevice = &m_Devices[iDevSlot];
	ItemCategories DevCat = pDevice->GetCategory();

	//	Clear the device

	pDevice->Uninstall(this, ItemList);

	//	Adjust named devices list

	switch (DevCat)
		{
		case itemcatWeapon:
			if (m_NamedDevices[devPrimaryWeapon] == iDevSlot)
				m_NamedDevices[devPrimaryWeapon] = FindNextDevice(iDevSlot, itemcatWeapon);
			m_pController->OnWeaponStatusChanged();
			break;

		case itemcatLauncher:
			m_NamedDevices[devMissileWeapon] = -1;
			m_pController->OnWeaponStatusChanged();
			break;

		case itemcatShields:
			m_NamedDevices[devShields] = -1;
			m_pController->OnShipStatus(IShipController::statusArmorRepaired, -1);
			break;

		case itemcatDrive:
			m_NamedDevices[devDrive] = -1;
			break;

		case itemcatCargoHold:
			m_NamedDevices[devCargo] = -1;
			break;

		case itemcatReactor:
			m_NamedDevices[devReactor] = -1;
			break;
		}

	//	Recalc bonuses

	CalcDeviceBonus();
    CalcPerformance();

	InvalidateItemListState();
    m_pController->OnStatsChanged();

	return NOERROR;
	}

void CShip::RemoveOverlay (DWORD dwID)

//	RemoveOverlay
//
//	Removes an overlay from the ship
	
	{
	m_Overlays.RemoveField(this, dwID);

	//	NOTE: No need to recalc bonuses or overlap impact because the overlay
	//	is not actually removed until Update (at which point we recalc).
	}

void CShip::RepairAllArmor (void)

//	RepairAllArmor
//
//	Repair all the ship's armor

	{
	for (int i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pSection = GetArmorSection(i);
		pSection->SetHitPoints(pSection->GetMaxHP(this));
		m_pController->OnShipStatus(IShipController::statusArmorRepaired, i);
		}
	}

void CShip::RepairArmor (int iSect, int iHitPoints, int *retiHPRepaired)

//	RepairArmor
//
//	Repairs the armor

	{
	CInstalledArmor *pSection = GetArmorSection(iSect);
	int iDamage = pSection->GetMaxHP(this) - pSection->GetHitPoints();

	if (iHitPoints == -1)
		iHitPoints = iDamage;
	else
		iHitPoints = min(iDamage, iHitPoints);

	pSection->IncHitPoints(iHitPoints);

	m_pController->OnShipStatus(IShipController::statusArmorRepaired, iSect);

	if (retiHPRepaired)
		*retiHPRepaired = iHitPoints;
	}

void CShip::RepairDamage (int iHitPoints)

//	RepairDamage
//
//	Repairs the given number of hit points of damage

	{
	bool bRepaired = false;
	int iSect = 0;
	while (iHitPoints > 0 && iSect < GetArmorSectionCount())
		{
		CInstalledArmor *pSect = GetArmorSection(iSect);
		int iDamage = pSect->GetMaxHP(this) - pSect->GetHitPoints();
		if (iDamage > 0)
			{
			int iRepair = min(iDamage, iHitPoints);
			pSect->IncHitPoints(iRepair);
			iHitPoints -= iRepair;
			bRepaired = true;
			}

		iSect++;
		}

	if (bRepaired)
		m_pController->OnShipStatus(IShipController::statusArmorRepaired, -1);
	}

ALERROR CShip::ReportCreateError (const CString &sError) const

//  ReportCreateError
//
//  Reports an error while creating a ship.

    {
    ::kernelDebugLogMessage("Error creating ship %08x: %s", (m_pClass ? m_pClass->GetUNID() : 0), sError);
    return ERR_FAIL;
    }

bool CShip::RequestDock (CSpaceObject *pObj, int iPort)

//	RequestDock
//
//	pObj requests docking services with the station. Returns TRUE
//	if docking is engaged.

	{
	//	If time has stopped for this object, then we cannot allow docking

	if (IsTimeStopped())
		{
		pObj->SendMessage(this, CONSTLIT("Unable to dock"));
		return false;
		}

	//	If the object requesting docking services is an enemy,
	//	then deny docking services.

	if (IsEnemy(pObj))
		{
		pObj->SendMessage(this, CONSTLIT("Docking request denied"));
		return false;
		}

	//	Get the nearest free port

	return m_DockingPorts.RequestDock(this, pObj, iPort);
	}

void CShip::RevertOrientationChange (void)

//	RevertOrientationChange
//
//	Revert the ship's rotation this tick

	{
	}

DeviceNames CShip::SelectWeapon (int iDev, int iVariant)

//	SelectWeapon
//
//	Selects the given weapon to fire. Returns whether the named
//	class for this weapon

	{
	CInstalledDevice *pWeapon = GetDevice(iDev);

	if (pWeapon == NULL)
		return devNone;
	else if (pWeapon->GetCategory() == itemcatWeapon)
		{
		m_NamedDevices[devPrimaryWeapon] = iDev;
		m_pController->OnWeaponStatusChanged();
		return devPrimaryWeapon;
		}
	else if (pWeapon->GetCategory() == itemcatLauncher)
		{
		m_NamedDevices[devMissileWeapon] = iDev;
		ReadyFirstMissile();
		while (iVariant > 0)
			{
			ReadyNextMissile();
			iVariant--;
			}

		m_pController->OnWeaponStatusChanged();
		return devMissileWeapon;
		}
	else
		return devNone;
	}

void CShip::SendMessage (CSpaceObject *pSender, const CString &sMsg)

//	SendMessage
//
//	Receives a message from some other object

	{
	//	Send this message to the player, if necessary

	if (IsPlayer())
		{
		IPlayerController *pPlayer = g_pUniverse->GetPlayer();
		if (pPlayer)
			pPlayer->OnMessageFromObj(pSender, sMsg);
		}
	}

bool CShip::SetAbility (Abilities iAbility, AbilityModifications iModification, int iDuration, DWORD dwOptions)

//	SetAbility
//
//	Changes the given ability. Returns TRUE if the ability's status changed.

	{
	switch (iAbility)
		{
		case ablAutopilot:
			{
			if (iModification == ablInstall)
				{
				bool bChanged = !m_fHasAutopilot;
				m_fHasAutopilot = true;
				return bChanged;
				}
			else if (iModification == ablRemove)
				{
				bool bChanged = m_fHasAutopilot;
				m_fHasAutopilot = false;
				return bChanged;
				}
			else
				return false;
			}

		case ablExtendedScanner:
			{
			if (iModification == ablInstall)
				{
				bool bChanged = !m_fSRSEnhanced;
				m_fSRSEnhanced = true;
				return bChanged;
				}
			else if (iModification == ablRemove)
				{
				bool bChanged = m_fSRSEnhanced;
				m_fSRSEnhanced = false;
				return bChanged;
				}
			else
				return false;
			}

		case ablFriendlyFireLock:
			{
			if (iModification == ablInstall)
				{
				bool bChanged = !m_fFriendlyFireLock;
				m_fFriendlyFireLock = true;
				return bChanged;
				}
			else if (iModification == ablRemove)
				{
				bool bChanged = m_fFriendlyFireLock;
				m_fFriendlyFireLock = false;
				return bChanged;
				}
			else
				return false;
			}

		case ablGalacticMap:
			{
			if (iModification == ablInstall)
				{
				bool bChanged = !m_fGalacticMap;
				m_fGalacticMap = true;
				return bChanged;
				}
			else if (iModification == ablRemove)
				{
				bool bChanged = m_fGalacticMap;
				m_fGalacticMap = false;
				return bChanged;
				}
			else
				return false;
			}

		case ablShortRangeScanner:
			{
			if (iModification == ablDamage)
				{
				bool bChanged = (m_iBlindnessTimer == 0);
				if (m_iBlindnessTimer != -1)
					{
					if (iDuration == -1)
						m_iBlindnessTimer = -1;
					else
						m_iBlindnessTimer += iDuration;
					}

				if (bChanged)
					m_pController->OnBlindnessChanged(true, ((dwOptions & ablOptionNoMessage) ? true : false));

				return bChanged;
				}
			else if (iModification == ablRepair)
				{
				bool bChanged = (m_iBlindnessTimer != 0);
				m_iBlindnessTimer = 0;

				if (bChanged)
					m_pController->OnBlindnessChanged(false, ((dwOptions & ablOptionNoMessage) ? true : false));

				return bChanged;
				}
			else
				return false;
			}

		case ablTargetingSystem:
			{
			if (iModification == ablInstall)
				{
				bool bChanged = !m_fHasTargetingComputer;
				m_fHasTargetingComputer = true;
				return bChanged;
				}
			else if (iModification == ablRemove)
				{
				bool bChanged = m_fHasTargetingComputer;
				m_fHasTargetingComputer = false;
				return bChanged;
				}
			else
				return false;
			}

		default:
			return false;
		}
	}

void CShip::SetCommandCode (ICCItem *pCode)

//	SetCommandCode
//
//	Sets command code for the ship

	{
	m_pController->SetCommandCode(pCode);
	}

void CShip::SetController (IShipController *pController, bool bFreeOldController)

//	SetController

	{
	ASSERT(pController);

	if (bFreeOldController && m_pController)
		delete dynamic_cast<CObject *>(m_pController);

	m_pController = pController;
	m_pController->SetShipToControl(this);
	m_pController->OnStatsChanged();
	}

void CShip::SetCursorAtArmor (CItemListManipulator &ItemList, int iSect)

//	SetCursorAtArmor
//
//	Set the item list cursor to the item for the given armor segment

	{
	ItemList.ResetCursor();

	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.GetType()->IsArmor()
				&& Item.GetInstalled() == iSect)
			{
			ASSERT(Item.GetCount() == 1);
			break;
			}
		}
	}

void CShip::SetCursorAtDevice (CItemListManipulator &ItemList, int iDev)

//	SetCursorAtDevice
//
//	Set the item list cursor to the item for the given device

	{
	ItemList.ResetCursor();

	CInstalledDevice *pDevice = GetDevice(iDev);
	if (pDevice->IsEmpty())
		return;

	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.GetType() == pDevice->GetClass()->GetItemType()
				&& Item.GetInstalled() == iDev)
			{
			ASSERT(Item.GetCount() == 1);
			break;
			}
		}
	}

void CShip::SetCursorAtNamedDevice (CItemListManipulator &ItemList, DeviceNames iDev)

//	SetCursorAtNamedDevice
//
//	Set the item list cursor to the given named device

	{
	if (m_NamedDevices[iDev] != -1)
		SetCursorAtDevice(ItemList, m_NamedDevices[iDev]);
	}

void CShip::SetFireDelay (CInstalledDevice *pWeapon, int iDelay)

//	SetFireDelay
//
//	Sets the fire delay for the weapon based on the intrinsic
//	fire rate of the weapon and the AISettings

	{
	if (iDelay == -1)
		pWeapon->SetTimeUntilReady(m_pController->GetFireRateAdj() * pWeapon->GetActivateDelay(this) / 10);
	else
		pWeapon->SetTimeUntilReady(iDelay);
	}

void CShip::SetInGate (CSpaceObject *pGate, int iTickCount)

//	SetInGate
//
//	While timer is on, ship is inside of stargate. When timer expires
//	the ship comes out (with gate flash)

	{
	//	Set in gate context (Note: It is OK if we get called here while
	//	IsInGate is true--this can happen if the player passes back and forth
	//	through the same gate).

	m_pExitGate = pGate;
	m_iExitGateTimer = GATE_ANIMATION_LENGTH + iTickCount;
	SetCannotBeHit();
	}

void CShip::SetOrdersFromGenerator (SShipGeneratorCtx &Ctx)

//	SetOrdersFromGenerator
//
//	Configures ship from a generator element

	{
	//	Call create code

	if (Ctx.pOnCreate)
		{
		CCodeChainCtx CCCtx;

		CCCtx.SaveAndDefineSourceVar(this);
		CCCtx.DefineSpaceObject(CONSTLIT("aBaseObj"), Ctx.pBase);
		CCCtx.DefineSpaceObject(CONSTLIT("aTargetObj"), Ctx.pTarget);

		ICCItem *pResult = CCCtx.Run(Ctx.pOnCreate);	//	LATER:Event
		if (pResult->IsError())
			ReportEventError(CONSTLIT("local OnCreate"), pResult);
		CCCtx.Discard(pResult);
		}

	//	Give the ship a chance to set orders

	FireOnCreateOrders(Ctx.pBase, Ctx.pTarget);

	//	If the ship still has no orders, then set them based on orders= attribute

	if (GetController()->GetCurrentOrderEx() == IShipController::orderNone)
		{
		CSpaceObject *pOrderTarget = NULL;
		bool bDockWithBase = false;
		bool bIsSubordinate = false;
		bool bNeedsDockOrder = false;
		switch (Ctx.iOrder)
			{
			case IShipController::orderNone:
				//	If a ship has no orders and it has a base, then dock with the base
				if (Ctx.pBase 
						&& Ctx.pBase->SupportsDocking()
						&& GetController()->GetCurrentOrderEx() == IShipController::orderNone)
					{
					bDockWithBase = true;
					bNeedsDockOrder = true;
					}
				break;

			case IShipController::orderDock:
				pOrderTarget = ((Ctx.pBase && Ctx.pBase->SupportsDocking()) ? Ctx.pBase : NULL);
				bDockWithBase = true;
				break;

			case IShipController::orderGuard:
				pOrderTarget = Ctx.pBase;
				bIsSubordinate = true;
				bDockWithBase = true;
				break;

			case IShipController::orderMine:
			case IShipController::orderPatrol:
			case IShipController::orderSentry:
				pOrderTarget = Ctx.pBase;
				bIsSubordinate = true;
				break;

			case IShipController::orderGateOnThreat:
				pOrderTarget = Ctx.pBase;
				bNeedsDockOrder = true;
				bDockWithBase = true;
				break;

			case IShipController::orderGate:
				//	For backwards compatibility...
				if (Ctx.pBase)
					{
					Ctx.iOrder = IShipController::orderGateOnThreat;
					pOrderTarget = Ctx.pBase;
					bNeedsDockOrder = true;
					bDockWithBase = true;
					}
				else
					{
					//	OK if this is NULL...we just go to closest gate
					pOrderTarget = Ctx.pTarget;
					}

				break;

			case IShipController::orderEscort:
			case IShipController::orderFollow:
				pOrderTarget = Ctx.pBase;
				break;

			case IShipController::orderDestroyTarget:
			case IShipController::orderAimAtTarget:
			case IShipController::orderDestroyTargetHold:
			case IShipController::orderAttackStation:
			case IShipController::orderBombard:
				pOrderTarget = Ctx.pTarget;
				break;
			}

		//	If we're creating a station and its ships and if we need to dock with 
		//	the base, then position the ship at a docking port

		if ((Ctx.dwCreateFlags & SShipCreateCtx::SHIPS_FOR_STATION) && bDockWithBase)
			{
			ASSERT(Ctx.pBase);
			if (Ctx.pBase)
				Ctx.pBase->PlaceAtRandomDockPort(this);
			else
				kernelDebugLogMessage("Base expected for ship class %x", GetType()->GetUNID());
			}

		//	Otherwise, if we need a dock order to get to our base, add that now
		//	(before the main order)

		else if (bNeedsDockOrder)
			{
			if (Ctx.pBase)
				GetController()->AddOrder(IShipController::orderDock, Ctx.pBase, IShipController::SData());
			else
				kernelDebugLogMessage("Unable to add ship order %d to ship class %x; no target specified", IShipController::orderDock, GetType()->GetUNID());
			}

		//	Add main order

		if (Ctx.iOrder != IShipController::orderNone)
			{
			//	If this order requires a target, make sure we have one

			bool bTargetRequired;
			OrderHasTarget(Ctx.iOrder, &bTargetRequired);
			if (bTargetRequired && pOrderTarget == NULL)
				kernelDebugLogMessage("Unable to add ship order %d to ship class %x; no target specified", Ctx.iOrder, GetType()->GetUNID());

			//	Add the order

			else
				GetController()->AddOrder(Ctx.iOrder, pOrderTarget, Ctx.OrderData);
			}

		//	If necessary, append an order to attack nearest enemy ships

		if (Ctx.dwCreateFlags & SShipCreateCtx::ATTACK_NEAREST_ENEMY)
			GetController()->AddOrder(IShipController::orderAttackNearestEnemy, NULL, IShipController::SData());

		//	If this ship is ordered to guard then it counts as a subordinate

		if (bIsSubordinate && Ctx.pBase && !Ctx.pBase->IsEnemy(this))
			Ctx.pBase->AddSubordinate(this);
		}
	}

bool CShip::SetProperty (const CString &sName, ICCItem *pValue, CString *retsError)

//	SetProperty
//
//	Sets an object property

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (strEquals(sName, PROPERTY_ALWAYS_LEAVE_WRECK))
		{
		m_fAlwaysLeaveWreck = !pValue->IsNil();
		return true;
		}
	else if (strEquals(sName, PROPERTY_DOCKING_ENABLED))
		{
		m_fDockingDisabled = pValue->IsNil();
		return true;
		}
    else if (strEquals(sName, PROPERTY_FUEL_LEFT))
        {
        m_rFuelLeft = Max(0.0, Min(pValue->GetIntegerValue() * FUEL_UNITS_PER_STD_ROD, GetMaxFuel()));
        return true;
        }

    else if (strEquals(sName, PROPERTY_FUEL_LEFT_EXACT))
        {
        m_rFuelLeft = Max(0.0, Min(pValue->GetDoubleValue(), GetMaxFuel()));
        return true;
        }

    else if (strEquals(sName, PROPERTY_HEALER_LEFT))
        {
        m_Armor.SetHealerLeft(pValue->GetIntegerValue());
        return true;
        }

	else if (strEquals(sName, PROPERTY_INTERIOR_HP))
		{
		m_Interior.SetHitPoints(this, m_pClass->GetInteriorDesc(), pValue->GetIntegerValue());
		return true;
		}
	else if (strEquals(sName, PROPERTY_OPERATING_SPEED))
		{
		CString sSpeed = pValue->GetStringValue();
		if (strEquals(sSpeed, SPEED_EMERGENCY))
			SetMaxSpeedEmergency();
		else if (strEquals(sSpeed, SPEED_FULL))
			ResetMaxSpeed();
		else if (strEquals(sSpeed, SPEED_HALF))
			SetMaxSpeedHalf();
		else if (strEquals(sSpeed, SPEED_QUARTER))
			SetMaxSpeedQuarter();
		else
			{
			*retsError = strPatternSubst(CONSTLIT("Invalid speed setting: %s"), sSpeed);
			return false;
			}

		return true;
		}
	else if (strEquals(sName, PROPERTY_PLAYER_WINGMAN))
		{
		m_pController->SetPlayerWingman(!pValue->IsNil());
		return true;
		}
	else if (strEquals(sName, PROPERTY_ROTATION))
		{
		SetRotation(pValue->GetIntegerValue());
		return true;
		}
	else if (strEquals(sName, PROPERTY_ROTATION_SPEED))
		{
		m_Rotation.SetRotationSpeedDegrees(m_Perf.GetRotationDesc(), pValue->GetDoubleValue());
		return true;
		}

	else if (strEquals(sName, PROPERTY_SELECTED_MISSILE))
		{
		//	Nil means that we don't want to make a change

		if (pValue->IsNil())
			return true;

		CInstalledDevice *pLauncher = GetNamedDevice(devMissileWeapon);
		if (pLauncher == NULL)
			{
			*retsError = CONSTLIT("No launcher installed.");
			return false;
			}

		CItem Item = GetItemFromArg(CC, pValue);
		CItemListManipulator ShipItems(GetItemList());
		if (!ShipItems.SetCursorAtItem(Item))
			{
			*retsError = CONSTLIT("Item is not on ship.");
			return false;
			}

		//	If the item is the same as the launcher then it means that the
		//	launcher has no ammo. In this case, we succeed.

		if (Item.GetType() == pLauncher->GetItem()->GetType())
			return true;

		//	Otherwise we figure out what ammo variant this is.

		int iVariant = pLauncher->GetClass()->GetAmmoVariant(Item.GetType());
		if (iVariant == -1)
			{
			*retsError = CONSTLIT("Item is not compatible with launcher.");
			return false;
			}

		SelectWeapon(m_NamedDevices[devMissileWeapon], iVariant);
		return true;
		}
	else if (strEquals(sName, PROPERTY_SELECTED_WEAPON))
		{
		//	Nil means that we don't want to make a change

		if (pValue->IsNil())
			return true;

		int iDev;
		if (!FindInstalledDeviceSlot(GetItemFromArg(CC, pValue), &iDev))
			{
			*retsError = CONSTLIT("Item is not an installed device on ship.");
			return false;
			}

		if (m_Devices[iDev].GetCategory() != itemcatWeapon)
			{
			*retsError = CONSTLIT("Item is not a weapon.");
			return false;
			}

		SelectWeapon(iDev, 0);
		return true;
		}
	else
		return CSpaceObject::SetProperty(sName, pValue, retsError);
	}

void CShip::SetWeaponTriggered (DeviceNames iDev, bool bTriggered)

//	SetWeaponTriggered
//
//	Sets the trigger on the given device on or off. We also trigger any
//	associated linked-fire devices.

	{
	int i;

	CInstalledDevice *pPrimaryDevice = GetNamedDevice(iDev);	//	OK if NULL.
	ItemCategories iCat = (iDev == devMissileWeapon ? itemcatLauncher : itemcatWeapon);

	//	Loop over all devices and activate the appropriate ones

	for (i = 0; i < GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = GetDevice(i);
		CItemCtx Ctx(this, pDevice);

		//	If this is the primary device, or if it is a device that
		//	is linked to the primary device, then activate it.

		if (!pDevice->IsEmpty()
				&& (pDevice == pPrimaryDevice
					|| (pDevice->IsLinkedFire(Ctx, iCat))))
			pDevice->SetTriggered(bTriggered);
		}
	}

void CShip::SetWeaponTriggered (CInstalledDevice *pWeapon, bool bTriggered)

//	SetWeaponTriggered
//
//	Sets the trigger on the given device on or off. We also trigger any
//	associated linked-fire devices.

	{
	int i;

	ItemCategories iCat = pWeapon->GetCategory();

	//	Loop over all devices and activate the appropriate ones

	for (i = 0; i < GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = GetDevice(i);
		CItemCtx Ctx(this, pDevice);

		//	If this is the primary device, or if it is a device that
		//	is linked to the primary device, then activate it.

		if (!pDevice->IsEmpty()
				&& (pDevice == pWeapon 
					|| (pDevice->IsLinkedFire(Ctx, iCat))))
			pDevice->SetTriggered(bTriggered);
		}
	}

void CShip::Undock (void)

//	Undock
//
//	Undock from station

	{
	if (m_pDocked)
		{
		m_pDocked->Undock(this);
		m_pDocked = NULL;

		//	Update our cargo mass (in case it has changed during docking)

		OnComponentChanged(comCargo);
		}
	}

void CShip::Undock (CSpaceObject *pObj)

//	Undock
//
//	Undocks from the station

	{
	m_DockingPorts.Undock(this, pObj);
	}

void CShip::UninstallArmor (CItemListManipulator &ItemList)

//	UninstallArmor
//
//	Uninstalls the armor at the cursor

	{
	}

void CShip::UpdateArmorItems (void)

//	UpdateArmorItems
//
//	Make sure that all the armor items have the correct damage flag

	{
	CItemListManipulator ItemList(GetItemList());
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.GetType()->GetCategory() == itemcatArmor)
			{
			CInstalledArmor *pSect = GetArmorSection(Item.GetInstalled());
			bool bDamaged = (pSect->GetHitPoints() < pSect->GetMaxHP(this));
			if (Item.IsDamaged() != bDamaged)
				{
				ItemList.SetDamagedAtCursor(bDamaged);
				InvalidateItemListState();
				}
			}
		}
	}

void CShip::UpdateDestroyInGate (void)

//	UpdateDestroyInGate
//
//	Called in OnUpdate when m_fDestroyInGate is true.

	{
    const CAISettings *pAI = m_pController->GetAISettings();

    //	If we're supposed to ascend on gate, then ascend now

    if (m_pClass->GetCharacter() != NULL
            || (pAI && pAI->AscendOnGate()))
        {
        ResetMaxSpeed();
        m_fDestroyInGate = false;
        GetSystem()->AscendObject(this);
        }

    //	If we're already suspended, then instead of destroying the ship, remove
    //	it from the system and re-add it. We need to do this so that all
    //	OnObjDestroyed notifications go out, etc.
    //
    //	Note that this will also clear out any events registered for the ship.

    else if (IsSuspended())
        {
        CSystem *pSystem = GetSystem();
        if (pSystem)
            {
            Remove(enteredStargate, CDamageSource());
            AddToSystem(pSystem);
            m_fDestroyInGate = false;
            }
        }
    else
        Destroy(enteredStargate, CDamageSource());
	}

bool CShip::UpdateFuel (int iTick)

//	UpdateFuel
//
//	Update fuel. We return FALSE if the ship is destroyed and we cannot continue
//	with update.
//
//	NOTE: iTick is a local system tick, not the global universe tick.

	{
	//	If we're out of power, then see if power is restored before we die from
	//	life-support failure.

	if (m_fOutOfPower)
		{
		//	If we've got power back, then we're OK

		if (m_Perf.GetReactorDesc().GetMaxPower() > 0)
			{
			m_fOutOfPower = false;
			m_pController->OnShipStatus(IShipController::statusReactorRestored);
			}

        //	Countdown grace period

        else if (--m_iReactorGraceTimer <= 0.0)
            {
            Destroy(killedByPowerFailure, CDamageSource(NULL, killedByPowerFailure));

            //	Shouldn't do anything else after being destroyed

            return false;
            }
        else
            m_pController->OnShipStatus(IShipController::statusLifeSupportWarning, m_iReactorGraceTimer / g_TicksPerSecond);
		}

	//	If we're out of fuel, then count down until we die

	else if (m_fOutOfFuel)
		{
        //	Countdown grace period

        if (--m_iReactorGraceTimer <= 0.0)
            {
            Destroy(killedByRunningOutOfFuel, CDamageSource(NULL, killedByRunningOutOfFuel));

            //	Shouldn't do anything else after being destroyed

            return false;
            }
        else
            m_pController->OnShipStatus(IShipController::statusLifeSupportWarning, m_iReactorGraceTimer / g_TicksPerSecond);
		}

	//	Otherwise, consume fuel

	else
		{
        CalcReactorStats();

        //	Consume fuel

        ConsumeFuel(m_iPowerDrain / m_Perf.GetReactorDesc().GetEfficiency());

        //	Make sure everything is running smoothly.

        if ((iTick % FUEL_CHECK_CYCLE) == 0)
            {
            Metric rFuelLeft;
			int iMaxPower = m_Perf.GetReactorDesc().GetMaxPower();

			//	If we're consuming more power than the reactor can output, then 
			//	we overload.

			if (m_iPowerDrain > iMaxPower)
				{
                m_pController->OnShipStatus(IShipController::statusReactorOverloadWarning, iTick / FUEL_CHECK_CYCLE);

                //	Consequences of reactor overload

                ReactorOverload();
				}

			//	If our reactor has 0 power, then we can't even run life-support,
			//	so we die.

			if (iMaxPower == 0)
				{
				m_fOutOfPower = true;
				DisableAllDevices();
				m_iReactorGraceTimer = FUEL_GRACE_PERIOD;
                m_pController->OnShipStatus(IShipController::statusReactorPowerFailure);
				}

			//	If we don't consume fuel, then no need to check further

			else if (!m_Perf.GetReactorDesc().UsesFuel())
				NULL;

			//	If we have no fuel left, then we may die

            else if ((rFuelLeft = GetFuelLeft()) <= 0.0)
                {
                //	See if the player has any fuel on board. If they do, then there
                //	is a small grace period

                if (HasFuelItem())
                    {
                    m_fOutOfFuel = true;
					DisableAllDevices();
                    m_iReactorGraceTimer = FUEL_GRACE_PERIOD;
                    m_pController->OnShipStatus(IShipController::statusFuelLowWarning, -1);
                    }

                //	Otherwise, the player is out of luck

                else
                    {
                    Destroy(killedByRunningOutOfFuel, CDamageSource(NULL, killedByRunningOutOfFuel));

                    //	Shouldn't do anything else after being destroyed
                    return false;
                    }
                }

			//	If we have low fuel, then warn the player.

            else if (rFuelLeft < (GetMaxFuel() / 8.0))
	            m_pController->OnShipStatus(IShipController::statusFuelLowWarning, iTick / FUEL_CHECK_CYCLE);
            }
        }

	//	Done

	return true;
	}

void CShip::UpdateInactive (void)

//	UpdateInactive
//
//	Updates if we are inactive

	{
    if (IsInGate() && !IsSuspended())
        {
        //	Gate effect

        if (m_iExitGateTimer == GATE_ANIMATION_LENGTH)
            if (m_pExitGate)
                m_pExitGate->OnObjLeaveGate(this);

        //	Done?

        if (--m_iExitGateTimer == 0)
            {
			if (m_pExitGate)
				Place(m_pExitGate->GetPos());

            if (!IsVirtual())
                ClearCannotBeHit();
            FireOnEnteredSystem(m_pExitGate);
            m_pExitGate = NULL;
            }
        }
	}
