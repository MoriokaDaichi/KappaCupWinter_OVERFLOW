#pragma once
#include "state/IState.h"

/**
 * 状態の切り替え役。TOwner は持ち主、TStateIdは状態の番号の enum class（最後に Num）
 * TStateBase は状態の親クラス（ふつうは IState<TOwner>。持ち主が状態に独自の関数を呼びたいときは、その親クラスを渡す）
 */
template<typename TOwner, typename TStateId, typename TStateBase = IState<TOwner>>
class StateMachine
{
public:
	explicit StateMachine(TOwner* owner) : m_owner(owner){}

	/** 今の状態の番号を返す。まだ状態がなければ Num */
	TStateId GetCurrentState() const
	{
		return m_currentId;
	}

	/** 今の状態が id かどうかを返す。まだ状態がなければ、どの id でも false */
	bool IsCurrentState(TStateId id) const
	{
		return m_currentId == id;
	}

	/** 今の状態の Update を呼ぶ。持ち主の Update から毎フレーム呼ぶ */
	void Update()
	{
		// ChangeState を一度も呼ぶ前に Update が呼ばれても落ちないようにする
		if (m_currentState == nullptr)
		{
			return;
		}
		m_currentState->Update();
	}

	/** 今の状態そのものを返す（持ち主が状態に聞きたいことがあるとき）。まだ状態がなければ nullptr */
	TStateBase* GetCurrentStateObject() const
	{
		return m_currentState;
	}

	/** TState を作って id の場所に登録する（TState は TStateBase を継承したクラス）。ChangeState より前に、全部の状態を登録しておく */
	template<typename TState>
	void AddState(TStateId id)
	{
		static_assert(std::is_base_of_v<TStateBase, TState>, "TStateは TStateBase を継承してください");
		m_stateList[ToIndex(id)] = std::make_unique<TState>(m_owner);
	}

	/** 状態を id に切り替える。今の状態の Exit → 次の状態の Enter の順に呼ぶ。同じ状態なら何もしない */
	void ChangeState(TStateId id)
	{
		if (m_currentState != nullptr && m_currentId == id)
		{
			return;
		}

		TStateBase* nextState = m_stateList[ToIndex(id)].get();
		K2_ASSERT(nextState != nullptr, "m_stateList[ToIndex(id)]がnullptrです。AddStateしてください");

		if (m_currentState != nullptr)
		{
			m_currentState->Exit();
		}

		m_currentState = nextState;
		m_currentId = id;

		m_currentState->Enter();
	}

	/** id の状態が登録されているか（AddState したか） */
	bool HasState(const TStateId id) const
	{
		return m_stateList[ToIndex(id)] != nullptr;
	}

private:
	/** 状態の数（配列の大きさに使う） */
	static constexpr int STATE_NUM = static_cast<int>(TStateId::Num);

	std::unique_ptr<TStateBase> m_stateList[STATE_NUM];			 ///< 全部の状態。AddState で登録する。登録していない番号は nullptr
	TStateBase* m_currentState = nullptr;						///< 今の状態。最初の ChangeState までは nullptr
	TStateId m_currentId = TStateId::Num;						///< 今の状態の番号。まだ状態がなければ Num
	TOwner* m_owner = nullptr;									///< 持ち主（BattleCharacter など）。状態を作るときに渡す

	/**	状態の番号を配列の添え字に変える */
	static int ToIndex(TStateId id)
	{
		const int i = static_cast<int>(id);
		K2_ASSERT(i >= 0 && i < STATE_NUM, "TStateId の範囲外です");
		return i;
	}

};
