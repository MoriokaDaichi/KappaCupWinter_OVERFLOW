#include "stdafx.h"
#include "stage/Stage.h"

#include "gimmick/Anchor.h"
#include "param/GameParam.h"
#include "gimmick/DashPad.h"
#include "gimmick/ElectricFence.h"
#include "gimmick/LaserWall.h"

void Stage::InitRenderer()
{
	m_renderer.Init();
}

void Stage::InitParticle()
{
	m_particle.Init();
}

void Stage::InitProp(const int type)
{
	const std::string path = std::string("Assets/modelData/stage/stage_") + StageParam::PROP_TYPES[type].name + ".tkm";
	m_props[type].Init(path.c_str(), StageParam::PROP_TYPES[type].tint);
}
void Stage::Update()
{
	if (m_reloadCount != nsGameParam::GetReloadCount())
	{
		Build();
	}

	if (m_player == nullptr)
	{
		return;
	}

	for (std::unique_ptr<Gimmick>& gimmick : m_gimmicks)
	{
		gimmick->Update(*m_player);
	}

}

void Stage::DeleteAll()
{
	for (Anchor* anchor : m_anchors)
	{
		DeleteGO(anchor);
	}
}

void Stage::Draw(RenderContext& rc)
{
	m_renderer.Draw();
	for (StageProp& prop : m_props)
	{
		prop.Draw();
	}

	for (std::unique_ptr<Gimmick>& gimmick : m_gimmicks)
	{
		gimmick->Draw(rc);
	}

	// 粒は半透明の加算なので、不透明な物のあとに描く
	m_particle.Draw();
}

void Stage::Build()
{
	BeginBuild();
	// 全部の区間を一気に置く（F5 で JSON を読み直したとき用。少しずつ置くのは読み込みのときだけ）
	while (!BuildNextSection())
	{
	}
}

void Stage::BeginBuild()
{
	m_gimmicks.clear();
	m_reloadCount = nsGameParam::GetReloadCount();
	m_blocks.clear();
	m_renderer.ClearBlocks();
	for (StageProp& prop : m_props)
	{
		prop.Clear();
	}
	m_buildSectionIndex = 0;
	m_buildAnchorCount = 0;
}

bool Stage::BuildNextSection()
{
	const std::vector<StageParam::Section>& sections = nsGameParam::Get().stage.sections;
	if (m_buildSectionIndex < static_cast<int>(sections.size()))
	{
		const StageParam::Section& section = sections[m_buildSectionIndex];

		AddBlocks(section.blocks, StageRenderer::Kind::Block);
		AddBlocks(section.panels, StageRenderer::Kind::Panel);

		for (const std::array<float, 3>&position : section.anchors)
		{
			PlaceAnchor(m_buildAnchorCount, Vector3(position[0], position[1], position[2]));
			m_buildAnchorCount++;
		}

		AddDecorations(section.towers, StageRenderer::Kind::Tower);
		AddDecorations(section.hazards, StageRenderer::Kind::Hazard);

		for (const StageParam::Prop& prop : section.props)
		{
			AddProp(prop);
		}

		for (const StageParam::Block& block : section.dashPads)
		{
			std::unique_ptr<Gimmick> gimmick = std::make_unique<DashPad>();
			gimmick->Init(block);
			m_gimmicks.push_back(std::move(gimmick));
		}

		for (const StageParam::WallPad& pad : section.wallPads)
		{
			StageParam::Block block;
			block.position = pad.position;
			block.size = pad.size;
			std::unique_ptr<Gimmick> gimmick = std::make_unique<DashPad>(pad.side);
			gimmick->Init(block);
			m_gimmicks.push_back(std::move(gimmick));
		}

		for (const StageParam::Block& block : section.fences)
		{
			std::unique_ptr<Gimmick> gimmick = std::make_unique<ElectricFence>();
			gimmick->Init(block);
			m_gimmicks.push_back(std::move(gimmick));
		}

		for (const StageParam::Block& block : section.lasers)
		{
			std::unique_ptr<Gimmick> gimmick = std::make_unique<LaserWall>();
			gimmick->Init(block);
			m_gimmicks.push_back(std::move(gimmick));
		}

		m_buildSectionIndex++;
		// まだ区間が残っていれば、続きは次の呼び出しで（読み込みでは次のフレーム）
		if (m_buildSectionIndex < static_cast<int>(sections.size()))
		{
			return false;
		}
	}

	// 全部の区間を置き終えた。使わなかったアンカーは、ロックされない高さへ隠す
	for (int i = m_buildAnchorCount; i < static_cast<int>(m_anchors.size()); i++)
	{
		m_anchors[i]->SetPosition(Vector3(0.0f, HIDDEN_HEIGHT, 0.0f));
	}
	return true;
}

void Stage::AddBlocks(const std::vector<StageParam::Block>& blocks, const StageRenderer::Kind kind)
{
	for (const StageParam::Block& block : blocks)
	{
		const Vector3 position(block.position[0], block.position[1], block.position[2]);
		const Vector3 size(block.size[0], block.size[1], block.size[2]);
		AddCollider(position, size);
		m_renderer.AddBlock(position, size, kind);
	}
}

void Stage::AddCollider(const Vector3& position, const Vector3& size)
{
	std::unique_ptr<Block> block = std::make_unique<Block>();
	block->collider.Create(size);

	RigidBodyInitData data;
	data.pos = position;
	data.collider = &block->collider;
	block->body.Init(data);

	m_blocks.push_back(std::move(block));
}

void Stage::AddProp(const StageParam::Prop& prop)
{
	const int type = StageParam::FindPropType(prop.model);
	m_props[type].Add(prop);

	const StageParam::PropType& propType = StageParam::PROP_TYPES[type];
	if (propType.colliderWidth <= 0.0f)
	{
		return;
	}

	// 当たり判定を付ける小物は回さないので、向きは気にしなくてよい
	AddCollider(
		Vector3(prop.position[0], prop.position[1] + propType.colliderCenterY * prop.scale[1], prop.position[2]),
		Vector3(propType.colliderWidth * prop.scale[0], propType.colliderHeight * prop.scale[1], propType.colliderDepth * prop.scale[2]));
}

void Stage::AddDecorations(const std::vector<StageParam::Block>& blocks, const StageRenderer::Kind kind)
{
	for (const StageParam::Block& block : blocks)
	{
		m_renderer.AddBlock(
			Vector3(block.position[0], block.position[1], block.position[2]),
			Vector3(block.size[0], block.size[1], block.size[2]),
			kind);
	}
}

void Stage::PlaceAnchor(const int index, const Vector3& position)
{
	if (index >= static_cast<int>(m_anchors.size()))
	{
		m_anchors.push_back(NewGO<Anchor>(0, "anchor"));
	}

	m_anchors[index]->SetPosition(position);
}
