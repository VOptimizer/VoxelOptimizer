extends VBoxContainer

onready var _SelectBox : = $OptionButton

func _ready():
	_SelectBox.add_item("Simple")
	_SelectBox.add_item("Greedy")
	_SelectBox.add_item("Marching cubes")
	
	_SelectBox.select(0)
