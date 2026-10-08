#pragma once
#include "state/IState.h"

/** 状態の切り替え役。TOwner は持ち主、TStateIdは状態の番号の enum class（最後に Num） */
template<typename TOwner, typename TStateId>
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

	/** TState を作って id の場所に登録する。ChangeState より前に、全部の状態を登録しておく */
	template<typename TState>
	void AddState(TStateId id)
	{
		static_assert(std::is_base_of_v<IState<TOwner>, TState>, "TStateは IState<TOwner>を継承してください");
		m_stateList[ToIndex(id)] = std::make_unique<TState>(m_owner);
	}

	/** 状態を id に切り替える。今の状態の Exit → 次の状態の Enter の順に呼ぶ。同じ状態なら何もしない */
	void ChangeState(TStateId id)
	{
		if (m_currentState != nullptr && m_currentId == id)
		{
			return;
		}

		IState<TOwner>* nextState = m_stateList[ToIndex(id)].get();
		K2_ASSERT(nextState != nullptr, "m_stateList[ToIndex(id)]がnullptrです。AddStateしてください");

		if (m_currentState != nullptr)
		{
			m_currentState->Exit();
		}

		m_currentState = nextState;
		m_currentId = id;

		m_currentState->Enter();
	}

private:
	/** 状態の数（配列の大きさに使う） */
	static constexpr int STATE_NUM = static_cast<int>(TStateId::Num);

	std::unique_ptr<IState<TOwner>> m_stateList[STATE_NUM];		 ///< 全部の状態。AddState で登録する。登録していない番号は nullptr
	IState<TOwner>* m_currentState = nullptr;					///< 今の状態。最初の ChangeState までは nullptr
	TStateId m_currentId = TStateId::Num;						///< 今の状態の番号。まだ状態がなければ Num
	TOwner* m_owner = nullptr;									///< 持ち主（Player や Enemy）。状態を作るときに渡す

	/**	状態の番号を配列の添え字に変える */
	static int ToIndex(TStateId id)
	{
		const int i = static_cast<int>(id);
		K2_ASSERT(i >= 0 && i < STATE_NUM, "TStateId の範囲外です");
		return i;
	}

};
