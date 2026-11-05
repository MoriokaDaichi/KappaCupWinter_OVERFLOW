#include "stdafx.h"
#include "stage/Stage.h"

#include "gimmick/Anchor.h"
#include "param/GameParam.h"

void Stage::Init()
{
	m_renderer.Init();
	m_particle.Init();
	for (int i = 0; i < StageParam::PROP_TYPE_COUNT; i++)
	{
		const std::string path = std::string("Assets/modelData/stage/stage_") + StageParam::PROP_TYPES[i].name + ".tkm";
		m_props[i].Init(path.c_str(), StageParam::PROP_TYPES[i].tint);
	}
	Build();
}

void Stage::Update()
{
	if (m_reloadCount != nsGameParam::GetReloadCount())
	{
		Build();
	}
}

void Stage::DeleteAll()
{
	for (Anchor* anchor : m_anchors)
	{
		DeleteGO(anchor);
	}
}

void Stage::Draw()
{
	m_renderer.Draw();
	for (StageProp& prop : m_props)
	{
		prop.Draw();
	}
	// 粒は半透明の加算なので、不透明な物のあとに描く
	m_particle.Draw();
}

void Stage::Build()
{
	m_reloadCount = nsGameParam::GetReloadCount();
	m_blocks.clear();
	m_renderer.ClearBlocks();
	for (StageProp& prop : m_props)
	{
		prop.Clear();
	}

	int anchorCount = 0;
	for (const StageParam::Section& section : nsGameParam::Get().stage.sections)
	{
		for (const StageParam::Block& block : section.blocks)
		{
			AddBlock(
				Vector3(block.position[0], block.position[1], block.position[2]),
				Vector3(block.size[0], block.size[1], block.size[2]));
		}

		for (const std::array<float, 3>& position : section.anchors)
		{
			PlaceAnchor(anchorCount, Vector3(position[0], position[1], position[2]));
			anchorCount++;
		}

		AddDecorations(section.towers, StageRenderer::Kind::Tower);
		AddDecorations(section.hazards, StageRenderer::Kind::Hazard);

		for (const StageParam::Prop& prop : section.props)
		{
			AddProp(prop);
		}
	}

	for (int i = anchorCount; i < static_cast<int>(m_anchors.size()); i++)
	{
		m_anchors[i]->SetPosition(Vector3(0.0f, HIDDEN_HEIGHT, 0.0f));
	}
}

void Stage::AddBlock(const Vector3& position, const Vector3& size)
{
	AddCollider(position, size);
	m_renderer.AddBlock(position, size);
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
