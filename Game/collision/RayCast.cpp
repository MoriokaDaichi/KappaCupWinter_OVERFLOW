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

		// 何にも当たらなければ、初期値（isHit = false, type = -1）のまま返す
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
}
