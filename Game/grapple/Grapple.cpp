#include "stdafx.h"
#include "grapple/Grapple.h"

#include "grapple/GrappleWire.h"

Grapple::~Grapple()
{
	DeleteGO(m_wire);
}

void Grapple::Init()
{
	m_wire = NewGO<GrappleWire>(0, "grappleWire");
}

void Grapple::UpdateLock(const Vector3& position, const Vector3& chestPosition, const Vector3& aimOrigin, const Vector3& aimDirection)
{
	m_locker.Update(position, chestPosition, aimOrigin, aimDirection);
}

void Grapple::SetHandPosition(const Vector3& hand)
{
	m_wire->SetHandPosition(hand);
}

GrappleWire& Grapple::GetWire()
{
	return *m_wire;
}
