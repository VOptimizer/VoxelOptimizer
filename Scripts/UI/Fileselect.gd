tool

class_name Fileselect
extends VBoxContainer

signal load_file(file)

export(String) var Caption = "" setget set_caption
export(String) var FilePath = "" setget set_filepath
export(PoolStringArray) var Filters 
export(bool) var SaveDialog = false 

onready var PopupFileDialogScene = preload("res://Scenes/UI/PopupFileDialog.tscn")

onready var _Caption : = $Caption
onready var _Path : = $HBoxContainer/Path
onready var _CurrentFileDlg : PopupFileDialog = null

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
	set_filepath(file)
	emit_signal("load_file", file)
	
func _popup_hide():
	remove_child(_CurrentFileDlg)
	_CurrentFileDlg.queue_free()
	_CurrentFileDlg = null

func _on_select_pressed():
	_CurrentFileDlg = PopupFileDialogScene.instance()
	_CurrentFileDlg.connect("file_selected", self, "_file_selected")
	_CurrentFileDlg.connect("popup_hide", self, "_popup_hide")
	
	add_child(_CurrentFileDlg)
	_CurrentFileDlg.Filters = Filters
	_CurrentFileDlg.SaveDialog = SaveDialog
	_CurrentFileDlg.Path = _LastPath
	_CurrentFileDlg.popup()


func _on_Path_text_changed(new_text):
	FilePath = new_text
	_LastPath = new_text.replace(new_text.get_file(), "")
