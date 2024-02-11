extends VBoxContainer

onready var _SelectBox : = $OptionButton

func _ready():
	_SelectBox.add_item("Simple")
	_SelectBox.add_item("Greedy")
	_SelectBox.add_item("Marching cubes")
	_SelectBox.add_item("Greedy Chunked")
	_SelectBox.add_item("Greedy Textured (Beta)")
	#_SelectBox.add_item("Flood")
	
	_SelectBox.select(0)
