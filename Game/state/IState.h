#pragma once

/** 状態の土台。TOwnerは「誰の状態か」（Player や Enemy）*/
template<typename TOwner>
class IState
{
public:
	explicit IState(TOwner* owner) : m_owner(owner){}
	virtual ~IState() = default;

	/** 状態に入った瞬間に1回だけ呼ばれる */
	virtual void Enter() {}
	/** その状態の間、毎フレーム呼ばれる */
	virtual void Update() = 0;
	/** 状態から出るときに1回呼ばれる */
	virtual void Exit() {}

protected:
	TOwner* m_owner = nullptr;		///< この状態の持ち主
};
