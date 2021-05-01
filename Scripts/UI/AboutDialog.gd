extends PopupPanel

onready var _Dlg : = $WindowDialog

func popup(bounds: Rect2 = Rect2( 0, 0, 0, 0 )):
	.popup(bounds)
	
	_Dlg.popup_centered()


func _on_Button_pressed():
	_Dlg.hide()

func _on_RichTextLabel_meta_clicked(meta):
	OS.shell_open(meta)

func _on_WindowDialog_popup_hide():
	hide()
