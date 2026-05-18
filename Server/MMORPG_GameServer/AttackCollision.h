#pragma once

class AttackCollision
{
public:
	static bool IsInCircle(const Location& attackerLocation, const Location& targetLocation, float range);
	static bool IsInCone(const Location& attackerLocation, const Location& targetLocation, float range, float attackYaw, float halfAngleDegree);
	static bool IsInBox(const Location& attackerLocation, const Location& targetLocation, float halfwidth, float attackYaw, float length);
};

