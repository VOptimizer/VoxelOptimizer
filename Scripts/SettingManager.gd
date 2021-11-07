extends Node

const SETTINGS_DIR : String = "user://config"

onready var _FileDialogConfig : FileDialogConfig = FileDialogConfig.new()

func _ready() -> void:
	var filedialogConfig : String = SETTINGS_DIR.plus_file("filedialog.res")
	var file : File = File.new()
	if file.file_exists(filedialogConfig):
		_FileDialogConfig = load(filedialogConfig)

func _exit_tree() -> void:
	var dir := Directory.new()
	if !dir.dir_exists(SETTINGS_DIR):
		dir.make_dir_recursive(SETTINGS_DIR)
		
	var filedialogConfig : String = SETTINGS_DIR.plus_file("filedialog.res")
	ResourceSaver.save(filedialogConfig, _FileDialogConfig)

func set_favourites(fav : Array) -> void:
	_FileDialogConfig.favourites = fav

func set_recent_input(recent : Array) -> void:
	_FileDialogConfig.recent_input = recent
	
func set_recent_output(recent : Array) -> void:
	_FileDialogConfig.recent_output = recent

func get_favourites() -> Array:
	return _FileDialogConfig.favourites

func get_recent_input() -> Array:
	return _FileDialogConfig.recent_input
	
func get_recent_output() -> Array:
	return _FileDialogConfig.recent_output
