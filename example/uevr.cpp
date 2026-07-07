//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "imgui.h"

#include "uevr.h"


//
//	UEVRDemo::UEVRDemo
//

UEVRDemo::UEVRDemo() {
	blueprint.SetBlueprint("haptic_jump", "UEVR");
	BlueprintLua::SetupUEVRRegistry(blueprint);

	blueprint.AddVariable("Rumble Amplitude", BlueprintEditor::PinType(BlueprintEditor::PinKind::Float), "300");
	blueprint.AddVariable("God Mode", BlueprintEditor::PinType(BlueprintEditor::PinKind::Boolean), "true");

	buildSampleGraph();

	luaView.SetLanguage(TextEditor::Language::Lua());
	luaView.SetReadOnlyEnabled(true);
	luaView.SetShowLineNumbersEnabled(true);
	regenerate();
}


//
//	UEVRDemo::buildSampleGraph
//

void UEVRDemo::buildSampleGraph() {
	// XInput Get State -> A button pressed? -> haptic pulse + pawn:Jump()
	auto xinput = blueprint.AddEventNode("UEVR", "XInput Get State", ImVec2(0.0f, 0.0f));
	auto pressed = blueprint.AddCallFunctionNode("XInput", "Is Button Pressed", ImVec2(240.0f, 140.0f));
	auto branch = blueprint.AddFlowControlNode("Branch", ImVec2(300.0f, 0.0f));
	auto haptic = blueprint.AddCallFunctionNode("UEVR_VRData", "Trigger Haptic Vibration", ImVec2(520.0f, 0.0f));
	auto rightIndex = blueprint.AddCallFunctionNode("UEVR_VRData", "Get Right Controller Index", ImVec2(240.0f, 360.0f));
	auto getAmplitude = blueprint.AddVariableGetNode("Rumble Amplitude", ImVec2(260.0f, 300.0f));
	auto pawn = blueprint.AddCallFunctionNode("UEVR_API", "Get Local Pawn", ImVec2(620.0f, 300.0f));
	auto jump = blueprint.AddCallFunctionNode("UObject", "Call (No Args)", ImVec2(900.0f, 0.0f));

	blueprint.AddLink(blueprint.FindPinID(xinput, "", true), blueprint.FindPinID(branch, "", false));
	blueprint.AddLink(blueprint.FindPinID(xinput, "State", true), blueprint.FindPinID(pressed, "State", false));
	blueprint.AddLink(blueprint.FindPinID(pressed, "Return Value", true), blueprint.FindPinID(branch, "Condition", false));
	blueprint.AddLink(blueprint.FindPinID(branch, "True", true), blueprint.FindPinID(haptic, "", false));
	blueprint.AddLink(blueprint.FindPinID(getAmplitude, "", true), blueprint.FindPinID(haptic, "Amplitude", false));
	blueprint.AddLink(blueprint.FindPinID(rightIndex, "Return Value", true), blueprint.FindPinID(haptic, "Source", false));
	blueprint.AddLink(blueprint.FindPinID(haptic, "", true), blueprint.FindPinID(jump, "", false));
	blueprint.AddLink(blueprint.FindPinID(pawn, "Return Value", true), blueprint.FindPinID(jump, "Target", false));

	blueprint.AddCommentNode("Haptic pulse and jump when the A button is pressed", ImVec2(-40.0f, -70.0f), ImVec2(1160.0f, 560.0f));

	// Pre Engine Tick -> Do Once -> make the pawn invulnerable
	auto tick = blueprint.AddEventNode("UEVR", "Pre Engine Tick", ImVec2(0.0f, 600.0f));
	auto once = blueprint.AddFlowControlNode("Do Once", ImVec2(300.0f, 600.0f));
	auto branch2 = blueprint.AddFlowControlNode("Branch", ImVec2(520.0f, 600.0f));
	auto getGodMode = blueprint.AddVariableGetNode("God Mode", ImVec2(480.0f, 760.0f));
	auto setProp = blueprint.AddPropertySetNode("AActor", "bCanBeDamaged", ImVec2(760.0f, 600.0f));
	auto pawn2 = blueprint.AddCallFunctionNode("UEVR_API", "Get Local Pawn", ImVec2(760.0f, 800.0f));
	auto log = blueprint.AddCallFunctionNode("UEVR_API", "Print", ImVec2(1060.0f, 600.0f));

	blueprint.AddLink(blueprint.FindPinID(tick, "", true), blueprint.FindPinID(once, "", false));
	blueprint.AddLink(blueprint.FindPinID(once, "Completed", true), blueprint.FindPinID(branch2, "", false));
	blueprint.AddLink(blueprint.FindPinID(getGodMode, "", true), blueprint.FindPinID(branch2, "Condition", false));
	blueprint.AddLink(blueprint.FindPinID(branch2, "True", true), blueprint.FindPinID(setProp, "", false));
	blueprint.AddLink(blueprint.FindPinID(pawn2, "Return Value", true), blueprint.FindPinID(setProp, "Target", false));
	blueprint.AddLink(blueprint.FindPinID(setProp, "", true), blueprint.FindPinID(log, "", false));

	blueprint.AddCommentNode("Disable damage once when the script starts", ImVec2(-40.0f, 540.0f), ImVec2(1360.0f, 400.0f));

	blueprint.ClearDirty();
	blueprint.ZoomToFit();
}


//
//	UEVRDemo::regenerate
//

void UEVRDemo::regenerate() {
	luaView.SetText(BlueprintLua::GenerateScript(blueprint));
}


//
//	UEVRDemo::render
//

void UEVRDemo::render(bool* open) {
	ImGui::SetNextWindowSize(ImVec2(1200.0f, 760.0f), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("UEVR Lua Editor", open, ImGuiWindowFlags_MenuBar)) {
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Graph")) {
				if (ImGui::MenuItem("Rebuild Sample Graph")) { blueprint.ClearGraph(); buildSampleGraph(); regenerate(); }
				if (ImGui::MenuItem("Clear Graph")) { blueprint.ClearGraph(); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, blueprint.CanUndo())) { blueprint.Undo(); }
				if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", nullptr, blueprint.CanRedo())) { blueprint.Redo(); }
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "Ctrl+X", nullptr, blueprint.HasSelection())) { blueprint.Cut(); }
				if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, blueprint.HasSelection())) { blueprint.Copy(); }
				if (ImGui::MenuItem("Paste", "Ctrl+V")) { blueprint.Paste(); }
				if (ImGui::MenuItem("Duplicate", "Ctrl+D", nullptr, blueprint.HasSelection())) { blueprint.Duplicate(); }
				if (ImGui::MenuItem("Delete", "Del", nullptr, blueprint.HasSelection())) { blueprint.DeleteSelected(); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				if (ImGui::MenuItem("Zoom to Fit", "Home")) { blueprint.ZoomToFit(); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Lua")) {
				if (ImGui::MenuItem("Copy Script to Clipboard")) { ImGui::SetClipboardText(luaView.GetText().c_str()); }
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// regenerate the script whenever the graph changes
		if (blueprint.IsDirty()) {
			regenerate();
			blueprint.ClearDirty();
		}

		// blueprint canvas on top, generated Lua below
		auto area = ImGui::GetContentRegionAvail();
		blueprint.Render("UEVRBlueprintCanvas", ImVec2(0.0f, area.y * 0.6f));

		ImGui::Spacing();
		ImGui::TextUnformatted("Generated UEVR Lua script (read-only, updates live):");
		luaView.Render("GeneratedLua");
	}

	ImGui::End();
}
