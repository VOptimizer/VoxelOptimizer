extends ViewportContainer

onready var _Viewport = $Viewport

func _gui_input(event):
	_Viewport.unhandled_input(event)

func _on_ViewportContainer_resized():
	_Viewport.size = rect_size
