/*!
 *@brief	ゲームオブジェクトを安全に参照するためのハンドル。
 */

#pragma once

#include "gameObject/GameObjectManager.h"
#include <cstdint>

namespace nsK2EngineLow {

	/// <summary>
	/// ゲームオブジェクトへの「安全な参照」。
	/// </summary>
	/// <remarks>
	/// 【なぜこれが必要なのか】
	///
	/// 他のゲームオブジェクトを覚えておきたいとき、
	/// 生のポインタをメンバ変数に持つと、次のような事故が起きる。
	///
	///     class Missile : public IGameObject {
	///         Enemy* m_target;    // 生ポインタで敵を覚えておく
	///     };
	///
	///     // どこかで敵が死ぬ
	///     DeleteGO(enemy);
	///
	///     // 次のフレーム
	///     void Missile::Update()
	///     {
	///         m_target->GetPosition();   // 削除済みのメモリにアクセス
	///     }
	///
	/// このバグのやっかいなところは、
	/// 「たまたま動いてしまうことがある」という点。
	/// 解放されたメモリがまだ書き換えられていなければ、
	/// 何事もなかったかのように動いてしまう。
	/// そして忘れたころに、別の場所でクラッシュする。
	///
	/// 【GOHandleを使うとどうなるか】
	///
	///     class Missile : public IGameObject {
	///         GOHandle<Enemy> m_target;   // ハンドルで敵を覚えておく
	///     };
	///
	///     void Missile::Update()
	///     {
	///         Enemy* target = m_target.Get();
	///         if (target == nullptr) {
	///             // 敵はもういない。
	///             DeleteGO(this);
	///             return;
	///         }
	///         HomingTo(target->GetPosition());
	///     }
	///
	/// GOHandleはポインタではなく「インスタンスID」を覚えている。
	/// Get()を呼ぶたびにGameObjectManagerへ問い合わせるので、
	/// すでに死んでいるオブジェクトに対しては必ずnullptrが返ってくる。
	///
	/// 【なぜ -> 演算子が無いのか】
	///
	/// このクラスはわざと operator-> を用意していない。
	/// m_target->Update() と書けてしまうと、
	/// 結局nullptrチェックを忘れてしまうから。
	/// 必ず Get() でポインタを受け取り、nullptrかどうかを確認すること。
	/// 「面倒くさい」と感じる書き方が、バグを防いでくれる。
	/// </remarks>
	template<class T>
	class GOHandle {
	public:
		/// <summary>
		/// コンストラクタ。空のハンドルを作る。
		/// </summary>
		GOHandle() = default;
		/// <summary>
		/// コンストラクタ。
		/// </summary>
		/// <param name="gameObject">
		/// 覚えておきたいゲームオブジェクト。nullptrでもよい。
		/// </param>
		explicit GOHandle(T* gameObject)
		{
			Reset(gameObject);
		}
		/// <summary>
		/// 参照するゲームオブジェクトを設定する。
		/// </summary>
		void Reset(T* gameObject)
		{
			m_instanceId = (gameObject != nullptr) ? gameObject->GetInstanceID() : 0;
		}
		/// <summary>
		/// 参照を空にする。
		/// </summary>
		void Clear()
		{
			m_instanceId = 0;
		}
		/// <summary>
		/// ゲームオブジェクトを取得する。
		/// </summary>
		/// <returns>
		/// 生きている場合はそのアドレス。
		/// すでに死んでいる場合と、何も設定されていない場合はnullptr。
		/// </returns>
		T* Get() const
		{
			if (m_instanceId == 0) {
				return nullptr;
			}
			auto gameObjectManager = GameObjectManager::GetInstance();
			if (gameObjectManager == nullptr) {
				return nullptr;
			}
			IGameObject* gameObject =
				gameObjectManager->FindGameObjectByInstanceID(m_instanceId);
			if (gameObject == nullptr) {
				return nullptr;
			}
			// インスタンスIDは使い回されないので、
			// このIDのオブジェクトは必ずReset()で渡されたオブジェクトそのもの。
			// なのでdynamic_castではなくstatic_castで安全にキャストできる。
			return static_cast<T*>(gameObject);
		}
		/// <summary>
		/// 参照先が生きているか判定する。
		/// </summary>
		bool IsAlive() const
		{
			return Get() != nullptr;
		}
		/// <summary>
		/// 参照しているインスタンスIDを取得する。
		/// </summary>
		uint64_t GetInstanceID() const
		{
			return m_instanceId;
		}
	private:
		uint64_t m_instanceId = 0;	//!<参照しているゲームオブジェクトのインスタンスID。0なら未設定。
	};
}
