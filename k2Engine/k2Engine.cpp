#include "k2EnginePreCompile.h"
#include "k2Engine.h"
#include "dbg/dbgVectorRenderer.h"

namespace nsK2Engine {
	K2Engine* K2Engine::m_instance = nullptr;
	SceneLight* g_sceneLight = nullptr;
	RenderingEngine* g_renderingEngine = nullptr;
	CollisionObjectManager* g_collisionObjectManager = nullptr;
	K2Engine* g_k2Engine = nullptr;
	void K2Engine::Init(const InitData& initData)
	{
		g_k2Engine = this;
		g_engine = &m_k2EngineLow;
		g_collisionObjectManager = &m_collisionObjectManager;
		g_renderingEngine = &m_renderingEngine;

		raytracing::InitData raytracintInitData;
		raytracintInitData.m_expandShaderResource = &m_renderingEngine.GetRaytracingLightData();
		raytracintInitData.m_expandShaderResourceSize = sizeof(m_renderingEngine.GetRaytracingLightData());
#ifdef COPY_RAYTRACING_FRAMEBUFFER
		// フレームバッファにコピーする場合は、レイトレの出力先のカラーバッファのフォーマットを
		// フレームバッファと同じにする。
		raytracintInitData.m_outputColorBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif // #ifdef COPY_RAYTRACING_FRAMEBUFFER
		m_k2EngineLow.Init(
			initData.hwnd, 
			initData.frameBufferWidth, 
			initData.frameBufferHeight,
			raytracintInitData
		);
		m_renderingEngine.Init(initData.isSoftShadow);
		//ライトとポストエフェクトのデバッグパネルを登録する。
		nsDbg::RegisterRenderPanels();
		g_camera3D->SetPosition({ 0.0f, 100.0f, -200.0f });
		g_camera3D->SetTarget({ 0.0f, 50.0f, 0.0f });
#ifdef K2_DEBUG
		// デバッグベクトルレンダラーを作成する。
		m_vectorRenderer = NewGO<nsDbg::VectorRenderer>(0);
#endif // #ifdef K2_DEBUG
	}
	K2Engine::~K2Engine()
	{
#ifdef K2_DEBUG
		DeleteGO(m_vectorRenderer);
#endif
		g_renderingEngine = nullptr;
		g_collisionObjectManager = nullptr;
		g_engine = nullptr;
	}

	void K2Engine::Execute()
	{
		auto& renderContext = g_graphicsEngine->GetRenderContext();

		//処理負荷の計測を開始。
		nsK2EngineLow::nsDbg::DebugProfiler::BeginFrame();

		g_engine->BeginFrame();

		{
			K2_PROFILE_SCOPE("Update");
			g_engine->ExecuteUpdate();
			// レンダリングエンジンの更新。
			m_renderingEngine.Update();
		}

		{
			K2_PROFILE_SCOPE("Render(CPU)");
			g_engine->ExecuteRender();
			//レンダリングエンジンを実行。
			m_renderingEngine.Execute(renderContext);

			//当たり判定描画。
			g_engine->DebubDrawWorld();
		}

		//////////////////////////////////////
		//絵を描くコードを書くのはここまで！！！
		//////////////////////////////////////
		{
			//Presentと、FPS制限のための待ち時間。
			K2_PROFILE_SCOPE("EndFrame(Present/Wait)");
			g_engine->EndFrame();
		}

		//処理負荷の計測を終了。
		nsK2EngineLow::nsDbg::DebugProfiler::EndFrame();
	}
#ifdef K2_DEBUG
	void K2Engine::DrawVector(const Vector3& vector, const Vector3& origin, const char* name)
	{
		nsDbg::VectorRenderer::SRenderData renderData;
		renderData.origin = origin;
		renderData.vector = vector;
		renderData.name = name;
		m_vectorRenderer->AddVector(renderData);
	}
	void K2Engine::SetDrawVectorEnable()
	{
		m_vectorRenderer->EnableRender();
	}
	void K2Engine::SetDrawVectorDisable()
	{
		m_vectorRenderer->DisableRender();
	}
#endif // #ifdef K2_DEBUG
}