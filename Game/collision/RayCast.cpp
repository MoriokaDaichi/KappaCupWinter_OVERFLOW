#include "stdafx.h"
#include "collision/RayCast.h"

namespace
{
	/**
	 * プレイヤー自身とゴーストを無視して、一番近い当たりを探す。
	 * エンジンの PhysicsWorld::RayTest は法線が取れず、プレイヤー自身にも当たるため、自作している。
	 */
	struct IgnoreCharacterRayCallback : public btCollisionWorld::ClosestRayResultCallback
	{
		IgnoreCharacterRayCallback(const btVector3& from, const btVector3& to)
			: ClosestRayResultCallback(from, to){}

		/** Bullet が「この相手と当たり判定をするか」を聞いてくる。false を返すと無視される */
		bool needsCollision(btBroadphaseProxy* proxy) const override
		{
			// m_clientObject は void* なので、Bullet のオブジェクトに戻してから調べる
			const btCollisionObject* obj = static_cast<btCollisionObject*>(proxy->m_clientObject);
			
			// キャラクター（プレイヤー自身など）と、見えない判定の箱（ゴースト）は当てない。
			// 条件はエンジンの CharacterController.cpp と同じ
			if (obj->getUserIndex() == enCollisionAttr_Character
				|| obj->getInternalType() == btCollisionObject::CO_GHOST_OBJECT)
			{
				return false;
			}
			// それ以外は、Bullet 本来の判定（衝突フィルター）に任せる
			return ClosestRayResultCallback::needsCollision(proxy);
		}
	};

	constexpr float MIN_SEGMENT_LENGTH_SQ = 0.0001f;	///< これより短い線分（長さの2乗）は点として扱う（0 で割らないため）

	/** 0〜1 に収める */
	float Clamp01(const float value)
	{
		return min(max(value, 0.0f), 1.0f);
	}

	/** Bullet のベクトルをエンジンのベクトルに変える */
	Vector3 ToVector3(const btVector3& v)
	{
		return Vector3(v.x(), v.y(), v.z());
	}
}

namespace nsCollision
{
	RayHitInfo RayCast(const Vector3& start, const Vector3& end)
	{
		const btVector3 from(start.x, start.y, start.z);
		const btVector3 to(end.x, end.y, end.z);

		// 物理ワールドにレイを飛ばす。結果は callback に入る
		IgnoreCharacterRayCallback callback(from, to);
		PhysicsWorld::GetInstance()->GetDynamicWorld()->rayTest(from, to, callback);

		RayHitInfo hit;

		// 何にも当たらなければ、初期値（isHit = false, type = INVALID_TYPE）のまま返す
		if (!callback.hasHit())
		{
			return hit;
		}

		hit.isHit = true;
		hit.position = ToVector3(callback.m_hitPointWorld);
		hit.normal = ToVector3(callback.m_hitNormalWorld);
		// 当たったものの種類。setUserIndex で付けた番号（地面は 0、壁などは EnCollisionType）
		hit.type = callback.m_collisionObject->getUserIndex();

		return hit;
	}

	bool SegmentHitsCapsule(const Vector3& start, const Vector3& end, const Vector3& capsuleBottom, const Vector3& capsuleTop, const float radius, Vector3& outPoint)
	{
		// 線分どうしの一番近い点を求める。s は線分の上の割合、t はカプセルの軸の上の割合（どちらも 0〜1）
		const Vector3 segment = end - start;
		const Vector3 axis = capsuleTop - capsuleBottom;
		const Vector3 fromBottom = start - capsuleBottom;
		const float segmentSq = segment.Dot(segment);
		const float axisSq = axis.Dot(axis);
		const float axisDot = axis.Dot(fromBottom);

		float s = 0.0f;
		float t = 0.0f;
		if (segmentSq <= MIN_SEGMENT_LENGTH_SQ)
		{
			// 線分が点のとき
			t = (axisSq <= MIN_SEGMENT_LENGTH_SQ) ? 0.0f : Clamp01(axisDot / axisSq);
		}
		else if (axisSq <= MIN_SEGMENT_LENGTH_SQ)
		{
			// カプセルの軸が点のとき
			s = Clamp01(-segment.Dot(fromBottom) / segmentSq);
		}
		else
		{
			const float segmentDot = segment.Dot(fromBottom);
			const float cross = segment.Dot(axis);
			const float denominator = segmentSq * axisSq - cross * cross;
			// 平行なら、線分のどこでも同じなので s は 0 のまま
			if (denominator > MIN_SEGMENT_LENGTH_SQ)
			{
				s = Clamp01((cross * axisDot - segmentDot * axisSq) / denominator);
			}

			t = (cross * s + axisDot) / axisSq;
			// 軸の外に出たら端に戻し、その端に一番近い s を出し直す
			if (t < 0.0f)
			{
				t = 0.0f;
				s = Clamp01(-segmentDot / segmentSq);
			}
			else if (t > 1.0f)
			{
				t = 1.0f;
				s = Clamp01((cross - segmentDot) / segmentSq);
			}
		}

		const Vector3 onSegment = start + segment * s;
		const Vector3 onAxis = capsuleBottom + axis * t;
		const Vector3 diff = onSegment - onAxis;
		if (diff.LengthSq() > radius * radius)
		{
			return false;
		}

		outPoint = onSegment;
		return true;
	}
}
