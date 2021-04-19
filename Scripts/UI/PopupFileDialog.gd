class_name PopupFileDialog
extends PopupPanel

signal file_selected(path)

export(PoolStringArray) var Filters setget set_filters
export(bool) var SaveDialog = false setget set_save_dialog
export(String) var Path = "" setget set_path

onready var _Dlg : = $FileDialog

func popup(bounds: Rect2 = Rect2( 0, 0, 0, 0 )):
	.popup(bounds)
	
	_Dlg.popup_centered(Vector2(400, 300))

func _on_FileDialog_popup_hide():
	hide()

func _on_FileDialog_file_selected(path):
	emit_signal("file_selected", path)

func set_filters(arr : PoolStringArray):
	_Dlg.set_filters(arr)
	
func set_save_dialog(val : bool):
	if val:
		_Dlg.mode = FileDialog.MODE_SAVE_FILE
	else:
		_Dlg.mode = FileDialog.MODE_OPEN_FILE
		
func set_path(path : String):
	_Dlg.current_path = path
