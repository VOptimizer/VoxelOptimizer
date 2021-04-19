extends Control

onready var _VoxelOptimizer : = CGodotVoxelOptimizer.new()
onready var _MeshInstance : = $HBoxContainer/ViewportContainer/Viewport/Spatial/MeshInstance
onready var _Wireframe : = $HBoxContainer/ViewportContainer/Viewport/Spatial/Wireframe
onready var _OutputFile : = $HBoxContainer/VBoxContainer/CenterContainer/PanelContainer/MarginContainer/VBoxContainer/Outputfile
onready var _InputFile : = $HBoxContainer/VBoxContainer/CenterContainer/PanelContainer/MarginContainer/VBoxContainer/Inputfile

var _Selected = 0

func _on_Fileselect_load_file(file : String):
	var f = File.new()
	if f.file_exists(file):
		var err = _VoxelOptimizer.load(file)
		if err == OK:
			var mesh : ArrayMesh = _VoxelOptimizer.get_mesh(_Selected)
			var mesh2 : ArrayMesh = ArrayMesh.new()
			
			for i in mesh.get_surface_count():
				mesh2.add_surface_from_arrays(Mesh.PRIMITIVE_LINES, mesh.surface_get_arrays(i))
			
			_MeshInstance.mesh = mesh#_VoxelOptimizer.get_mesh(_Selected)
			_Wireframe.mesh = mesh2
			
	_OutputFile.FilePath = file.replace(file.get_extension(), "obj")


func _on_Button_pressed():
	_VoxelOptimizer.save(_OutputFile.FilePath)


func _on_OptionButton_item_selected(index):	
	_Selected = index
	
	if _InputFile.FilePath != "":
		_on_Fileselect_load_file(_InputFile.FilePath)


func _on_Button_toggled(button_pressed):
	_Wireframe.visible = button_pressed
