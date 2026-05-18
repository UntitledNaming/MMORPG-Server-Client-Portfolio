#include <cmath>
#include "ContentsStruct.h"
#include "AttackCollision.h"

bool AttackCollision::IsInCircle(const Location& attackerLocation, const Location& targetLocation, float range)
{
	float dx = targetLocation.xpos - attackerLocation.xpos;
	float dy = targetLocation.ypos - attackerLocation.ypos;

	float distSq = dx * dx + dy * dy; // 거리 제곱
	float rangeSq = range * range;    // 사거리 제곱

	// 사거리 밖이면 false
	if (distSq > rangeSq)
		return false;

	return true;
}

bool AttackCollision::IsInCone(const Location& attackerLocation, const Location& targetLocation, float range, float attackYaw, float halfAngleDegree)
{
	float dx = targetLocation.xpos - attackerLocation.xpos;
	float dy = targetLocation.ypos - attackerLocation.ypos;

	float distSq = dx * dx + dy * dy; // 거리 제곱
	float rangeSq = range * range;    // 사거리 제곱

	// 사거리 밖이면 false
	if (distSq > rangeSq)
		return false;

	// 공격자와 거리가 매우 가까우면 방향상관없이 맞는 처리
	if (distSq <= 0.0001f)
		return false;

	float halfAngleRad = halfAngleDegree * FieldConst::Pi / 180.0f;

	Vec2 forward;
	forward.m_xpos = cosf(halfAngleRad);
	forward.m_ypos = sinf(halfAngleRad);

	// 공격 방향 단위 벡터와 내 위치에서 타겟 방향으로의 위치벡터의 내적
	// 다른 말로 내 위치에서 타겟 방향으로의 벡터를 공격 방향 단위 벡터 위로 투영시킨값
	// forwrad . toTarget = |forward| * |toTarget| * cosTheta;
	float dot = dx * forward.m_xpos + dy * forward.m_ypos;

	if (dot <= 0.f)
		return false;

	// forward는 단위 벡터임.
	// dot = |toTarget| * cosTheta;
	// cosTheta = dot / | toTarget|
	// cosTheta는 공격 방향 벡터와 공격자 위치에서 타겟방향으로의 벡터의 사잇각임.
	// 이 각도가 설정한 값 아래여야 함.
	// 사잇각 <= HalfAngle을 만족하려면 cos값을 취하면 cos(사잇각) >= cos(HalfAngle)임
	// cosTheta = dot / sqrt(distSq)
	// 지금 cosTheta >= cosHalfAngle을 만족해야 범위 안임.
	// dot / sqrt(distSq) >= cosHalfAngle 인데 양변 제곱하면
	// dot * dot / distSq >= cosHalfAngle * cosHalfAngle
	// dot * dot >= distSq * cosHalfAngle * cosHalfAngle
	float cosHalfAngle = cosf(halfAngleRad);

	return dot * dot >= distSq * cosHalfAngle * cosHalfAngle;
}

bool AttackCollision::IsInBox(const Location& attackerLocation, const Location& targetLocation, float halfwidth, float attackYaw, float length)
{
	float dx = targetLocation.xpos - attackerLocation.xpos;
	float dy = targetLocation.ypos - attackerLocation.ypos;

	float rad = attackYaw * FieldConst::Pi / 180.0f;
	float cosYaw = cosf(rad);
	float sinYaw = sinf(rad);

	float localX = dx * cosYaw + dy * sinYaw;
	float localY = -dx * sinYaw + dy * cosYaw;

	if (localX < 0 || localX > length)
		return false;

	if (fabsf(localY) > halfwidth)
		return false;

	return true;
}
