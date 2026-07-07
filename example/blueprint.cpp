//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "imgui.h"

#include "blueprint.h"


//
//	BlueprintDemo::BlueprintDemo
//

BlueprintDemo::BlueprintDemo() {
	// populate the UObject-style reflection registry with the sample hierarchy
	blueprint.SetupDefaultRegistry();
	blueprint.SetBlueprint("BP_DemoCharacter", "ACharacter");

	// add some blueprint member variables
	blueprint.AddVariable("Health", BlueprintEditor::PinType(BlueprintEditor::PinKind::Float), "100");
	blueprint.AddVariable("Max Health", BlueprintEditor::PinType(BlueprintEditor::PinKind::Float), "100");
	blueprint.AddVariable("Score", BlueprintEditor::PinType(BlueprintEditor::PinKind::Integer), "0");
	blueprint.AddVariable("Player Name", BlueprintEditor::PinType(BlueprintEditor::PinKind::String), "Player One");
	blueprint.AddVariable("Is Alive", BlueprintEditor::PinType(BlueprintEditor::PinKind::Boolean), "true");
	blueprint.AddVariable("Spawn Point", BlueprintEditor::PinType(BlueprintEditor::PinKind::Vector), "0,0,100");

	buildSampleGraph();
}


//
//	BlueprintDemo::buildSampleGraph
//

void BlueprintDemo::buildSampleGraph() {
	// Event BeginPlay -> Print String
	auto beginPlay = blueprint.AddEventNode("AActor", "BeginPlay", ImVec2(0.0f, 0.0f));
	auto print = blueprint.AddCallFunctionNode("UKismetSystemLibrary", "Print String", ImVec2(320.0f, 0.0f));
	blueprint.AddLink(blueprint.FindPinID(beginPlay, "", true), blueprint.FindPinID(print, "", false));

	// Event Tick -> drain health -> branch on death -> update state
	auto tick = blueprint.AddEventNode("AActor", "Tick", ImVec2(0.0f, 260.0f));
	auto getHealth = blueprint.AddVariableGetNode("Health", ImVec2(60.0f, 420.0f));
	auto multiply = blueprint.AddCallFunctionNode("UKismetMathLibrary", "Multiply (Float)", ImVec2(240.0f, 360.0f));
	auto subtract = blueprint.AddCallFunctionNode("UKismetMathLibrary", "Subtract (Float)", ImVec2(460.0f, 400.0f));
	auto setHealth = blueprint.AddVariableSetNode("Health", ImVec2(340.0f, 240.0f));
	auto less = blueprint.AddCallFunctionNode("UKismetMathLibrary", "Less (Float)", ImVec2(620.0f, 380.0f));
	auto branch = blueprint.AddFlowControlNode("Branch", ImVec2(640.0f, 240.0f));
	auto setAlive = blueprint.AddVariableSetNode("Is Alive", ImVec2(880.0f, 240.0f));
	auto died = blueprint.AddCallFunctionNode("UKismetSystemLibrary", "Print String", ImVec2(1140.0f, 240.0f));

	// exec flow
	blueprint.AddLink(blueprint.FindPinID(tick, "", true), blueprint.FindPinID(setHealth, "", false));
	blueprint.AddLink(blueprint.FindPinID(setHealth, "", true), blueprint.FindPinID(branch, "", false));
	blueprint.AddLink(blueprint.FindPinID(branch, "True", true), blueprint.FindPinID(setAlive, "", false));
	blueprint.AddLink(blueprint.FindPinID(setAlive, "", true), blueprint.FindPinID(died, "", false));

	// data flow: Health - (DeltaSeconds * 5) -> Health, Health < 0 -> Branch
	blueprint.AddLink(blueprint.FindPinID(tick, "Delta Seconds", true), blueprint.FindPinID(multiply, "A", false));
	blueprint.AddLink(blueprint.FindPinID(getHealth, "", true), blueprint.FindPinID(subtract, "A", false));
	blueprint.AddLink(blueprint.FindPinID(multiply, "Return Value", true), blueprint.FindPinID(subtract, "B", false));
	blueprint.AddLink(blueprint.FindPinID(subtract, "Return Value", true), blueprint.FindPinID(setHealth, "Health", false));
	blueprint.AddLink(blueprint.FindPinID(setHealth, "Health", true), blueprint.FindPinID(less, "A", false));
	blueprint.AddLink(blueprint.FindPinID(less, "Return Value", true), blueprint.FindPinID(branch, "Condition", false));

	blueprint.AddCommentNode("Health drain", ImVec2(-40.0f, 180.0f), ImVec2(880.0f, 340.0f));

	// a pure math island: distance to the player
	auto getPawn = blueprint.AddCallFunctionNode("UGameplayStatics", "Get Player Pawn", ImVec2(0.0f, 640.0f));
	auto distance = blueprint.AddCallFunctionNode("AActor", "Get Distance To", ImVec2(300.0f, 640.0f));
	blueprint.AddLink(blueprint.FindPinID(getPawn, "Return Value", true), blueprint.FindPinID(distance, "Target", false));

	// mark the freshly built graph as the clean baseline
	blueprint.ClearDirty();
	blueprint.ZoomToFit();
}


//
//	BlueprintDemo::renderMenuBar
//

void BlueprintDemo::renderMenuBar() {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Graph")) {
			if (ImGui::MenuItem("Save Snapshot")) { savedState = blueprint.SaveToString(); }
			if (ImGui::MenuItem("Load Snapshot", nullptr, nullptr, !savedState.empty())) { blueprint.LoadFromString(savedState); }
			ImGui::Separator();
			if (ImGui::MenuItem("Rebuild Sample Graph")) { blueprint.ClearGraph(); buildSampleGraph(); }
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
			ImGui::Separator();
			if (ImGui::MenuItem("Select All", "Ctrl+A")) { blueprint.SelectAll(); }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Zoom to Fit", "Home")) { blueprint.ZoomToFit(); }
			bool flag = blueprint.IsShowingGrid();
			if (ImGui::MenuItem("Show Grid", nullptr, &flag)) { blueprint.SetShowGrid(flag); }
			flag = blueprint.IsContextSensitive();
			if (ImGui::MenuItem("Context Sensitive Menu", nullptr, &flag)) { blueprint.SetContextSensitive(flag); }
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}


//
//	BlueprintDemo::render
//

void BlueprintDemo::render(bool* open) {
	ImGui::SetNextWindowSize(ImVec2(1100.0f, 680.0f), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Blueprint Editor", open, ImGuiWindowFlags_MenuBar)) {
		renderMenuBar();
		blueprint.Render("BlueprintCanvas");
	}

	ImGui::End();
}
