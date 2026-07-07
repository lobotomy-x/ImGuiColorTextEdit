//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "../BlueprintEditor.h"
#include "../BlueprintLua.h"
#include "../TextEditor.h"


//
//	UEVRDemo
//

class UEVRDemo {
public:
	// constructor
	UEVRDemo();

	// render the demo window
	void render(bool* open);

private:
	// private functions
	void buildSampleGraph();
	void regenerate();

	// properties
	BlueprintEditor blueprint;
	TextEditor luaView;
};
