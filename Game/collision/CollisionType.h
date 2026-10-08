#pragma once

/** 当たったものの種類（Bullet の userIndex に入れる） */
enum class EnCollisionType : int {
    Wall = enCollisionAttr_User,            // ウォールランできる壁
    ElectricFence,                          // 電気柵
    DashPanel,                              // ダッシュ板
};
