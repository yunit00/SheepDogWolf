/* The launcher's Installation Settings popup (launcher_state.h): the game's data, a folder or a disc image extracted
 * into the setup's install directory (outputDir), and resetting the save files */
#include "launcher_state.h"
#include "imgui.h"

static const char *const s_id = "Файлы игры";
static const char *const s_resetId = "Удаление сохранений";

void PopupInstall_Open() {
    ImGui::OpenPopup(s_id);
}

/* the game folder in use, and the buttons that change it */
static void Install_GameData(LauncherState &s) {
    Launcher_Warning("Поддержка дисков PSX экспериментальная.");
    ImGui::Spacing();
    std::string dir = s.options.exeDir[0] ? s.options.exeDir : Launcher_DefaultGameDir();
    ImGui::TextDisabled("Папка игры");
    ImGui::TextUnformatted(Launcher_FitLeft(dir.c_str(), ImGui::GetContentRegionAvail().x).c_str());
    if (!s.hasGameData)
        ImGui::TextColored(ImVec4(1, 0.75f, 0.3f, 1), "Папка Levels не найдена.");
    ImGui::Spacing();

    /* a folder becomes the game folder at once; a disc image is extracted first (below) */
    const float width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    ImGui::BeginDisabled(s.picking || Extract_Running());
    if (ImGui::Button("Папка...", ImVec2(width, 0)))
        Launcher_PickFolder(s);
    ImGui::SameLine();
    if (ImGui::Button("Образ диска...", ImVec2(width, 0)))
        Launcher_PickDiscImage(s);
    ImGui::EndDisabled();
}

/* a disc image picked here: its extraction into outputDir, the setup's install directory */
static void Install_DiscImage(LauncherState &s) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextDisabled(s.sourceKind == GAME_SOURCE_ISO ? "Образ ISO" : "Образ CUE/BIN");
    ImGui::TextUnformatted(Launcher_FitLeft(s.sourcePath.c_str(), ImGui::GetContentRegionAvail().x).c_str());
    if (!s.sourceValid) {
        ImGui::TextColored(ImVec4(1, 0.75f, 0.3f, 1), "Этот образ диска не подходит");
        return;
    }
    if (Extract_StartedFor(s.sourcePath)) {
        Extract_Draw(s);
        return;
    }
    ImGui::TextDisabled("Установка в");
    ImGui::TextUnformatted(Launcher_FitLeft(s.outputDir.c_str(), ImGui::GetContentRegionAvail().x).c_str());
    ImGui::Spacing();
    if (ImGui::Button("Установить", ImVec2(-1, 0)))
        Extract_Start(s);
}

/* the confirmation before deleting the saves, over this popup */
static void Install_ResetSaves() {
    if (!Popup_Begin(s_resetId))
        return;
    ImGui::TextWrapped("Удалить сохранения и настройки игры? Это действие нельзя отменить.");
    ImGui::Spacing();
    const float width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    if (ImGui::Button("Отмена", ImVec2(width, 0)))
        ImGui::CloseCurrentPopup();
    ImGui::SameLine();
    if (ImGui::Button("Удалить", ImVec2(width, 0))) {
        Launcher_ResetSaves();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void PopupInstall_Draw(LauncherState &s) {
    const bool canClose = !Extract_Running(); /* an extraction runs to its end, or until Cancel */
    if (!Popup_Begin(s_id, canClose))
        return;

    Install_GameData(s);
    if (s.sourceKind != GAME_SOURCE_FOLDER)
        Install_DiscImage(s);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::BeginDisabled(!canClose);
    if (ImGui::Button("Удалить сохранения...", ImVec2(-1, 0)))
        ImGui::OpenPopup(s_resetId);
    ImGui::EndDisabled();
    Install_ResetSaves();

    Popup_End(canClose);
}
