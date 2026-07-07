//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <string>

#include "../BlueprintEditor.h"


//
//	BlueprintDemo
//

class BlueprintDemo {
public:
	// constructor
	BlueprintDemo();

	// render the demo window
	void render(bool* open);

private:
	// private functions
	void buildSampleGraph();
	void renderMenuBar();

	// properties
	BlueprintEditor blueprint;
	std::string savedState;
};
