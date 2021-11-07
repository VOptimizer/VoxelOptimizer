extends Control

onready var _VoxelOptimizer : = CGodotVoxelOptimizer.new()
onready var _OutputFile : = $HBoxContainer/VBoxContainer/CenterContainer/PanelContainer/MarginContainer/VBoxContainer/Outputfile
onready var _InputFile : = $HBoxContainer/VBoxContainer/CenterContainer/PanelContainer/MarginContainer/VBoxContainer/Inputfile
onready var _Statistics : = $HBoxContainer/ViewportContainer/Statistics
onready var _StatisticsLabel : = $HBoxContainer/ViewportContainer/Statistics/MarginContainer/VBoxContainer/StatisticsLabel
onready var _AboutDialog : = $AboutDialog
onready var _Spatial : = $HBoxContainer/ViewportContainer/Viewport/Spatial
onready var _WireframeButton : = $HBoxContainer/PanelContainer/MarginContainer/VBoxContainer/Wireframe
onready var _WorldspaceToggle : = $HBoxContainer/VBoxContainer/CenterContainer/PanelContainer/MarginContainer/VBoxContainer/CheckButton

onready var _UnshadedMaterial = preload("res://Assets/Unshaded.material")
onready var _FileDialogConfig : FileDialogConfig = FileDialogConfig.new()

var _Selected = 0
var _Meshes = []
var _Wireframes = []

var _Transforms = []

func _on_Fileselect_load_file(file : String):
	var f = File.new()
	if f.file_exists(file):
		var err = _VoxelOptimizer.load(file)
		if err == OK:
			_generate_mesh()
			
	_OutputFile.FilePath = file.get_basename() + ".obj"

func _generate_mesh():
	for m in _Meshes:
		m.queue_free()
		
	for w in _Wireframes:
		w.queue_free()
	_Wireframes.clear()
	
	_Meshes = _VoxelOptimizer.get_meshes(_Selected)
	for m in _Meshes:
		_Spatial.add_child(m)
		
		var instance : = MeshInstance.new()
		_Spatial.add_child(instance)
		
		var mesh : ArrayMesh = ArrayMesh.new()
		
		var tmp = m.mesh.get_surface_count()
		
		for i in m.mesh.get_surface_count():
			var tmp1 = m.mesh.surface_get_arrays(i)[0].size()
			
			mesh.add_surface_from_arrays(Mesh.PRIMITIVE_LINES, m.mesh.surface_get_arrays(i))
	
		instance.mesh = mesh
		instance.transform = m.transform
		instance.visible = _WireframeButton.pressed
		instance.material_override = _UnshadedMaterial
		_Wireframes.append(instance)
	
	_set_statistics()

func _on_Button_pressed():
	if _OutputFile.FilePath.get_extension() != "png":
		_VoxelOptimizer.save(_OutputFile.FilePath, _WorldspaceToggle.pressed)
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
	for w in _Wireframes:
		w.visible = button_pressed
	
func _on_Button2_toggled(button_pressed):
	_set_statistics()
	_Statistics.visible = button_pressed

func _on_Button3_pressed():
	_AboutDialog.popup()

func _on_worldspace_toggled(button_pressed: bool) -> void:
	if button_pressed:
		for i in range(0, _Meshes.size()):
			_Meshes[i].transform = _Transforms[i]
			_Wireframes[i].transform = _Transforms[i]
	else:
		_Transforms.clear()
		for i in range(0, _Meshes.size()):
			_Transforms.append(_Meshes[i].transform)
			
			_Meshes[i].transform = Transform()
			_Wireframes[i].transform = Transform()
