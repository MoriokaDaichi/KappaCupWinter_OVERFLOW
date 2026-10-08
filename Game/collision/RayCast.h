#pragma once
#include "collision/CollisionType.h"

/** レイキャストの結果 */
struct RayHitInfo {
    bool isHit = false;                 ///< 何かにあたったらtrue
    Vector3 position = Vector3::Zero;  ///<当たった場所(ワールド座標)
    Vector3 normal = Vector3::Zero;    ///< 当たった面の向き ← ウォールランで使う
    int type = -1;                      ///< 当たったものの種類。enCollisionAttr_Ground か EnCollisionType の値。当たらなければ -1
};

namespace nsCollision
{
    /** start から end へレイを飛ばし、一番近い当たりを返す。プレイヤー自身と見えない判定の箱は無視する */
    RayHitInfo RayCast(const Vector3& start, const Vector3& end);
}
