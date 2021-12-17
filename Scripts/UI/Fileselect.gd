tool

class_name Fileselect
extends VBoxContainer

signal load_file(file)

export(String) var Caption = "" setget set_caption
export(String) var FilePath = "" setget set_filepath
export(PoolStringArray) var Filters 
export(bool) var SaveDialog = false 

onready var PopupFileDialogScene = preload("res://Scenes/UI/CustomFileDialog.tscn") #preload("res://Scenes/UI/PopupFileDialog.tscn")

onready var _Caption : = $Caption
onready var _Path : = $HBoxContainer/Path
onready var _CurrentFileDlg : CustomFileDialog = null

var _LastPath : String = ""

func _ready():
	set_caption(Caption)
	set_filepath(FilePath)

func set_caption(caption):
	Caption = caption
	
	if _Caption != null:
		_Caption.text = caption

func set_filepath(filepath : String):
	FilePath = filepath
	_LastPath = filepath.replace(filepath.get_file(), "")
	
	if _Path != null:
		_Path.text = filepath
		_Path.caret_position = filepath.length()

func _file_selected(file):
	var recent = _CurrentFileDlg.recent
	var found : bool = false
	
	for r in recent:
		if r == file:
			found = true
			break
	
	if !found:
		recent.append(file)
		
		if SaveDialog:
			SettingManager.set_recent_output(recent)
		else:
			SettingManager.set_recent_input(recent)
	
	set_filepath(file)
	emit_signal("load_file", file)
	
func _popup_hide():
	SettingManager.set_favourites(_CurrentFileDlg.favourites)
	
	remove_child(_CurrentFileDlg)
	_CurrentFileDlg.queue_free()
	_CurrentFileDlg = null

func _on_select_pressed():
	_CurrentFileDlg = PopupFileDialogScene.instance()
	_CurrentFileDlg.popup_exclusive = true
	_CurrentFileDlg.allow_select_all_files = false
	_CurrentFileDlg.connect("on_file_selected", self, "_file_selected")
	_CurrentFileDlg.connect("popup_hide", self, "_popup_hide")
	
	add_child(_CurrentFileDlg)
	_CurrentFileDlg.filters = Filters
	_CurrentFileDlg.access = CustomFileDialog.Access.ACCESS_FILESYSTEM
	_CurrentFileDlg.favourites = SettingManager.get_favourites()
	
	if SaveDialog:
		_CurrentFileDlg.mode = FileDialog.MODE_SAVE_FILE
		_CurrentFileDlg.recent = SettingManager.get_recent_output()
	else:
		_CurrentFileDlg.mode = FileDialog.MODE_OPEN_FILE
		_CurrentFileDlg.recent = SettingManager.get_recent_input()
	
	_CurrentFileDlg.current_dir = _LastPath
	_CurrentFileDlg.popup_centered()

func _on_Path_text_changed(new_text):
	FilePath = new_text
	_LastPath = new_text.replace(new_text.get_file(), "")
