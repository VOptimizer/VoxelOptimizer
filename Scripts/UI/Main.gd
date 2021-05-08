extends Control

onready var _VoxelOptimizer : = CGodotVoxelOptimizer.new()
onready var _MeshInstance : = $HBoxContainer/ViewportContainer/Viewport/Spatial/MeshInstance
onready var _Wireframe : = $HBoxContainer/ViewportContainer/Viewport/Spatial/Wireframe
onready var _OutputFile : = $HBoxContainer/VBoxContainer/CenterContainer/PanelContainer/MarginContainer/VBoxContainer/Outputfile
onready var _InputFile : = $HBoxContainer/VBoxContainer/CenterContainer/PanelContainer/MarginContainer/VBoxContainer/Inputfile
onready var _Statistics : = $HBoxContainer/ViewportContainer/Statistics
onready var _StatisticsLabel : = $HBoxContainer/ViewportContainer/Statistics/MarginContainer/VBoxContainer/StatisticsLabel
onready var _AboutDialog : = $AboutDialog

var _Selected = 0

func _on_Fileselect_load_file(file : String):
	var f = File.new()
	if f.file_exists(file):
		var err = _VoxelOptimizer.load(file)
		if err == OK:
			_generate_mesh()
			
	_OutputFile.FilePath = file.replace(file.get_extension(), "obj")

func _generate_mesh():
	var mesh : ArrayMesh = _VoxelOptimizer.get_mesh(_Selected)
	var mesh2 : ArrayMesh = ArrayMesh.new()
	
	for i in mesh.get_surface_count():
		mesh2.add_surface_from_arrays(Mesh.PRIMITIVE_LINES, mesh.surface_get_arrays(i))
	
	_MeshInstance.mesh = mesh#_VoxelOptimizer.get_mesh(_Selected)
	_Wireframe.mesh = mesh2
	_set_statistics()

func _on_Button_pressed():
	if _OutputFile.FilePath.get_extension() != "png":
		_VoxelOptimizer.save(_OutputFile.FilePath)
	else:
		_VoxelOptimizer.save_slices(_OutputFile.FilePath)

func _on_OptionButton_item_selected(index):	
	_Selected = index
	
	if _InputFile.FilePath != "":
		_generate_mesh()

func _set_statistics():
	var stats = _VoxelOptimizer.get_statistics()
	_StatisticsLabel.text = "Blockcount: " + str(stats.blocks)
	_StatisticsLabel.text += "\nVertices: " + str(stats.vertices)
	_StatisticsLabel.text += "\nFaces: " + str(stats.faces)

func _on_Button_toggled(button_pressed):
	_Wireframe.visible = button_pressed
	
func _on_Button2_toggled(button_pressed):
	_set_statistics()
	_Statistics.visible = button_pressed

func _on_Button3_pressed():
	_AboutDialog.popup()
