tool
extends Label

func _ready():
	text = "Version: " + ProjectSettings.get_setting("application/config/version")
