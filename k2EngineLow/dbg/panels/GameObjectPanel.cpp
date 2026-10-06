/*!
 *@brief	ゲームオブジェクトの一覧を表示するデバッグパネル。
 */

#include "k2EngineLowPreCompile.h"
#include "GameObjectPanel.h"

namespace nsK2EngineLow {
	namespace nsDbg {

#ifdef K2_ENABLE_DEBUG_GUI

		namespace {
			/// <summary>
			/// 一覧に表示する1行分の情報。
			/// </summary>
			struct GameObjectInfo {
				int priority = 0;				// 優先度。
				IGameObject* gameObject = nullptr;	// ゲームオブジェクト。
			};

			// 表示のフィルタ。
			ImGuiTextFilter	g_nameFilter;
			// 死亡済みのオブジェクトも表示するか。
			bool			g_isShowDead = true;
			// 優先度ごとにまとめて表示するか。
			bool			g_isGroupByPriority = true;

			/// <summary>
			/// 1行分を描画する。
			/// </summary>
			void DrawRow(const GameObjectInfo& info)
			{
				IGameObject* gameObject = info.gameObject;
				const bool isDead = gameObject->IsDead();

				ImGui::TableNextRow();
				ImGui::PushID((int)gameObject->GetInstanceID());

				// インスタンスID。
				ImGui::TableNextColumn();
				ImGui::Text("%llu", (unsigned long long)gameObject->GetInstanceID());

				// 名前。死亡済みは赤く表示する。
				ImGui::TableNextColumn();
				if (isDead) {
					ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", gameObject->GetName());
				}
				else {
					ImGui::TextUnformatted(gameObject->GetName());
				}

				// 優先度。
				ImGui::TableNextColumn();
				ImGui::Text("%d", info.priority);

				// 状態。
				ImGui::TableNextColumn();
				if (isDead) {
					// DeleteGO済み。次のフレームの頭で本当に破棄される。
					ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "DEAD");
				}
				else if (gameObject->IsActive() == false) {
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Inactive");
				}
				else if (gameObject->IsStart() == false) {
					ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.4f, 1.0f), "Starting");
				}
				else {
					ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.5f, 1.0f), "Alive");
				}

				// 操作ボタン。
				ImGui::TableNextColumn();
				if (isDead) {
					ImGui::TextDisabled("-");
				}
				else {
					if (gameObject->IsActive()) {
						if (ImGui::SmallButton("Deactivate")) {
							gameObject->Deactivate();
						}
					}
					else {
						if (ImGui::SmallButton("Activate  ")) {
							gameObject->Activate();
						}
					}
					ImGui::SameLine();
					if (ImGui::SmallButton("DeleteGO")) {
						DeleteGO(gameObject);
					}
				}
				ImGui::PopID();
			}

			/// <summary>
			/// 表を描画する。
			/// </summary>
			void DrawTable(const char* tableId, const std::vector<GameObjectInfo>& infoList)
			{
				const ImGuiTableFlags flags =
					ImGuiTableFlags_Borders
					| ImGuiTableFlags_RowBg
					| ImGuiTableFlags_SizingStretchProp;
				if (ImGui::BeginTable(tableId, 5, flags) == false) {
					return;
				}
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 44.0f);
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Prio", ImGuiTableColumnFlags_WidthFixed, 36.0f);
				ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 60.0f);
				ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableHeadersRow();
				for (const auto& info : infoList) {
					DrawRow(info);
				}
				ImGui::EndTable();
			}

			/// <summary>
			/// パネルの中身を描画する。
			/// </summary>
			void DrawGameObjectPanel()
			{
				auto gameObjectManager = GameObjectManager::GetInstance();
				if (gameObjectManager == nullptr) {
					ImGui::TextUnformatted("GameObjectManager is not created.");
					return;
				}

				// 一覧を集める。
				std::vector<GameObjectInfo> infoList;
				int aliveCount = 0;
				int deadCount = 0;
				gameObjectManager->QueryAllGameObjects(
					[&](int priority, IGameObject* gameObject) {
						if (gameObject->IsDead()) {
							deadCount++;
						}
						else {
							aliveCount++;
						}
						GameObjectInfo info;
						info.priority = priority;
						info.gameObject = gameObject;
						infoList.push_back(info);
					}
				);

				// 総数の表示。
				ImGui::Text("Alive : %d", aliveCount);
				ImGui::SameLine();
				if (deadCount > 0) {
					ImGui::TextColored(
						ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "    Dead(waiting) : %d", deadCount);
				}
				else {
					ImGui::TextDisabled("    Dead(waiting) : 0");
				}
				ImGui::SetItemTooltip("%s", U8(
					"DeleteGO()されたオブジェクトは、すぐには消えない。\n"
					"次のフレームのUpdateの先頭でまとめて破棄される。\n"
					"この「まだ生きているように見えるが死んでいる」状態が、\n"
					"削除済みオブジェクトへのアクセスの原因になる。"));

				// フィルタ。
				g_nameFilter.Draw("Filter", 160.0f);
				ImGui::SameLine();
				ImGui::Checkbox("Show dead", &g_isShowDead);
				ImGui::SameLine();
				ImGui::Checkbox("Group by prio", &g_isGroupByPriority);
				ImGui::Separator();

				// フィルタをかける。
				std::vector<GameObjectInfo> filteredList;
				for (const auto& info : infoList) {
					if (g_isShowDead == false && info.gameObject->IsDead()) {
						continue;
					}
					if (g_nameFilter.PassFilter(info.gameObject->GetName()) == false) {
						continue;
					}
					filteredList.push_back(info);
				}

				if (filteredList.empty()) {
					ImGui::TextDisabled("(no game object)");
					return;
				}

				if (g_isGroupByPriority == false) {
					DrawTable("##gameObjectsAll", filteredList);
					return;
				}

				// 優先度ごとにまとめて表示する。
				// filteredListはQueryAllGameObjectsが優先度順に回してくれているので、
				// すでに優先度の昇順に並んでいる。
				size_t index = 0;
				while (index < filteredList.size()) {
					const int priority = filteredList[index].priority;
					std::vector<GameObjectInfo> group;
					while (index < filteredList.size()
						&& filteredList[index].priority == priority
					) {
						group.push_back(filteredList[index]);
						index++;
					}
					char header[64];
					sprintf_s(header, "priority %d  (%d)", priority, (int)group.size());
					if (ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_DefaultOpen)) {
						char tableId[64];
						sprintf_s(tableId, "##gameObjects%d", priority);
						DrawTable(tableId, group);
					}
				}
			}
		}

		void RegisterGameObjectPanel()
		{
			DebugGui::RegisterPanel(
				"GameObject",
				"GameObject List",
				DrawGameObjectPanel,
				/*isDefaultVisible=*/false
			);
		}

#else // K2_ENABLE_DEBUG_GUI

		void RegisterGameObjectPanel() {}

#endif // K2_ENABLE_DEBUG_GUI
	}
}
