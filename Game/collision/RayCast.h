#pragma once
#include "collision/CollisionType.h"

/** レイキャストの結果 */
struct RayHitInfo {
    static constexpr int INVALID_TYPE = -1;     ///< 当たらなかったとき、または当たった物に種類が設定されていないとき
    bool isHit = false;                 ///< 何かにあたったらtrue
    Vector3 position = Vector3::Zero;  ///<当たった場所(ワールド座標)
    Vector3 normal = Vector3::Zero;    ///< 当たった面の向き ← ウォールランで使う
    int type = INVALID_TYPE;                      ///< 当たったものの種類。enCollisionAttr_Ground か EnCollisionType の値。当たらなければ INVALID_TYPE
};

namespace nsCollision
{
    /** start から end へレイを飛ばし、一番近い当たりを返す。プレイヤー自身と見えない判定の箱は無視する */
    RayHitInfo RayCast(const Vector3& start, const Vector3& end);
}
