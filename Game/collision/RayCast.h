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
    /**
     * 線分 start〜end が、カプセル（線分 capsuleBottom〜capsuleTop のまわり、半径 radius）に当たるかを調べる
     * 当たったら、線分 start〜end の上でカプセルの軸に一番近い点を outPoint に入れる。start と end が同じなら、球とカプセルの判定になる
     */
    bool SegmentHitsCapsule(const Vector3& start, const Vector3& end, const Vector3& capsuleBottom, const Vector3& capsuleTop, const float radius, Vector3& outPoint);
}
