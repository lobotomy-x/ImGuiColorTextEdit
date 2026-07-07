# BlueprintEditor: an Unreal Engine style Blueprint editor for Dear ImGui

`BlueprintEditor` is a self-contained node graph widget (`BlueprintEditor.h` /
`BlueprintEditor.cpp`) that mimics Unreal Engine's Blueprint visual scripting
editor. Like `TextEditor`, it is a single widget with no dependencies beyond
Dear ImGui itself, and all public member functions start with an uppercase
character to be consistent with Dear ImGui.

The widget edits graphs of UObject-style nodes: events, function calls,
property and variable accessors, flow control macros, reroute knots and
comments, connected by exec and data wires with Unreal's familiar pin type
colors.

## The reflection registry

Blueprint nodes are generated from a minimal mirror of Unreal's
`UClass`/`UFunction`/`FProperty` reflection model. In an Unreal Engine build
the registry can be populated from real UObject reflection data; standalone it
is filled by hand with a fluent builder API:

```cpp
BlueprintEditor blueprint;
auto& registry = blueprint.GetRegistry();

auto& actor = registry.AddClass("AActor", "UObject", "An actor in a level");
actor.AddEvent("BeginPlay").Tooltip("Event when play begins for this actor");
actor.AddEvent("Tick").Out("Delta Seconds", {BlueprintEditor::PinKind::Float});

actor.AddFunction("Set Actor Location", "Transformation")
	.In("New Location", {BlueprintEditor::PinKind::Vector})
	.Ret({BlueprintEditor::PinKind::Boolean});

actor.AddFunction("Get Actor Location", "Transformation")
	.Pure()
	.Ret({BlueprintEditor::PinKind::Vector});

actor.AddProperty("Root Component", {BlueprintEditor::PinKind::Object, "USceneComponent"});
```

- **Classes** form an inheritance chain (`IsChildOf` walks parents), which is
  used for object pin compatibility (an `APawn` output connects to an
  `AActor` input, but not vice versa).
- **Functions** are impure by default (they get white exec pins); `.Pure()`
  removes the exec pins and turns the header green; `.Static()` removes the
  `Target` pin (like Unreal's kismet libraries).
- **Events** appear in the palette for the blueprint's parent class chain and
  can only be placed once (like Unreal).
- **Enumerations** registered with `AddEnum` get combo box default value
  editors on enum pins.

`SetupDefaultRegistry()` fills the registry with a sample hierarchy
(`UObject`, `AActor`, `APawn`, `ACharacter`, components) plus kismet-style
math/string/system/gameplay libraries so you can try the editor immediately.

## Pin types

Pins are typed with `PinKind` (Exec, Boolean, Byte, Integer, Float, String,
Name, Vector, Rotator, Transform, Object, Class, Struct, Enum, Delegate,
Wildcard), an optional subtype (the class/struct/enum name) and an array flag.
Colors follow Unreal's conventions (green floats, magenta strings, gold
vectors, blue objects, ...). Connection rules also follow Unreal:

- Exec output pins drive a single target; data input pins have a single
  source; connecting to a full pin replaces the existing wire.
- Object/class pins respect the inheritance chain.
- Byte→Integer→Float promotions are allowed implicitly.
- Data connections that would create a cycle are refused (exec loops are fine).

## Interactive editing

| Interaction | Effect |
| --- | --- |
| Right-click canvas | searchable, context-sensitive "add node" palette |
| Drag from pin | create a wire; drop on empty canvas to spawn-and-connect |
| Alt-click pin or wire | break links |
| Double-click wire | insert a reroute node |
| Right-click node | delete/duplicate/break links/rename/add sequence pin |
| Left drag on canvas | box select (comments require full containment) |
| Right/middle drag | pan; mouse wheel zooms around the cursor |
| Ctrl+A/C/X/V/D | select all, copy, cut, paste, duplicate |
| Ctrl+Z / Ctrl+Shift+Z / Ctrl+Y | undo / redo |
| Del | delete selection, Home: zoom to fit, Esc: clear selection |

Unconnected input pins show inline default value editors (checkboxes, drag
scalars, text fields, vector/rotator triplets, enum and class combos), and
comment boxes drag all fully contained nodes along, just like Unreal.

## Building graphs programmatically

```cpp
blueprint.SetBlueprint("BP_DemoCharacter", "ACharacter");
blueprint.AddVariable("Health", {BlueprintEditor::PinKind::Float}, "100");

auto beginPlay = blueprint.AddEventNode("AActor", "BeginPlay", ImVec2(0.0f, 0.0f));
auto print = blueprint.AddCallFunctionNode("UKismetSystemLibrary", "Print String", ImVec2(320.0f, 0.0f));

blueprint.AddLink(
	blueprint.FindPinID(beginPlay, "", true),   // exec output
	blueprint.FindPinID(print, "", false));     // exec input
```

Other factories: `AddCustomEventNode`, `AddPropertyGetNode`,
`AddPropertySetNode`, `AddVariableGetNode`, `AddVariableSetNode`,
`AddFlowControlNode` ("Branch", "Sequence", "For Loop", "While Loop",
"Do Once", "Do N", "Flip Flop", "Gate"), `AddRerouteNode` and
`AddCommentNode`.

## Serialization and undo

`SaveToString()`/`LoadFromString()` implement a simple line-based text format
that round-trips the whole graph (blueprint identity, variables, nodes, pins,
links). The same format powers the clipboard (`Cut`/`Copy`/`Paste`/
`Duplicate`, using the Dear ImGui clipboard) and the snapshot-based undo
stack. `IsDirty()`/`ClearDirty()` track unsaved changes.

## Rendering

Call `Render` every frame inside a window:

```cpp
blueprint.Render("MyBlueprint", size, border);
```

The widget renders in a child window: an Unreal-style grid, a "BLUEPRINT"
watermark with the blueprint name, bezier wires (thicker white ones for exec),
zoom from 30% to 250% and a status line with node/link counts.

## Demo

The example program contains a full demo (`example/blueprint.cpp`): open
*View → Blueprint Editor* in the menu. It builds a small `ACharacter`
blueprint with a health drain graph, several variables and a comment box, and
provides menus for snapshots, clipboard, undo and view control.
