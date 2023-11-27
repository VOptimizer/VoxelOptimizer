tool
extends MeshInstance

export(Vector3) var size = Vector3(5, 5, 5) setget _set_size
export(NodePath) var camera

onready var _Camera : Camera = get_node(camera)

func _ready() -> void:
	_set_shader_size()

func _set_size(value : Vector3) -> void:
	size = value 
	_generate_mesh()
	_set_shader_size()
	
func _set_shader_size() -> void:
	if material_override:
		if material_override is ShaderMaterial:
			material_override.set_shader_param("grid_size", size)
			
func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion or event is InputEventMouseButton:
		var from = _Camera.project_ray_origin(event.position)
		var to = _Camera.project_ray_normal(event.position)
		
		pass
	
func _generate_mesh() -> void:
	var halfsize : Vector3 = size * 0.5
	var st := SurfaceTool.new()
	
	st.begin(Mesh.PRIMITIVE_TRIANGLES)
	
	# Front
	st.add_uv(Vector2(0, 0))
	st.add_vertex(Vector3(-halfsize.x, -halfsize.y, halfsize.z))
	
	st.add_uv(Vector2(0, 1))
	st.add_vertex(Vector3(-halfsize.x, halfsize.y, halfsize.z))
	
	st.add_uv(Vector2(1, 1))
	st.add_vertex(Vector3(halfsize.x, halfsize.y, halfsize.z))
	
	st.add_uv(Vector2(1, 0))
	st.add_vertex(Vector3(halfsize.x, -halfsize.y, halfsize.z))
	
	st.add_index(2)
	st.add_index(1)
	st.add_index(0)
	st.add_index(3)
	st.add_index(2)
	st.add_index(0)
	
	# Back
	
	st.add_uv(Vector2(0, 0))
	st.add_vertex(Vector3(-halfsize.x, -halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(0, 1))
	st.add_vertex(Vector3(-halfsize.x, halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(1, 1))
	st.add_vertex(Vector3(halfsize.x, halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(1, 0))
	st.add_vertex(Vector3(halfsize.x, -halfsize.y, -halfsize.z))
	
	st.add_index(4)
	st.add_index(5)
	st.add_index(6)
	st.add_index(4)
	st.add_index(6)
	st.add_index(7)
	
	# Left
	
	st.add_uv(Vector2(0, 0))
	st.add_vertex(Vector3(-halfsize.x, -halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(0, 1))
	st.add_vertex(Vector3(-halfsize.x, halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(1, 1))
	st.add_vertex(Vector3(-halfsize.x, halfsize.y, halfsize.z))
	
	st.add_uv(Vector2(1, 0))
	st.add_vertex(Vector3(-halfsize.x, -halfsize.y, halfsize.z))
	
	st.add_index(10)
	st.add_index(9)
	st.add_index(8)
	st.add_index(11)
	st.add_index(10)
	st.add_index(8)
	
	# Right
	
	st.add_uv(Vector2(0, 0))
	st.add_vertex(Vector3(halfsize.x, -halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(0, 1))
	st.add_vertex(Vector3(halfsize.x, halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(1, 1))
	st.add_vertex(Vector3(halfsize.x, halfsize.y, halfsize.z))
	
	st.add_uv(Vector2(1, 0))
	st.add_vertex(Vector3(halfsize.x, -halfsize.y, halfsize.z))
	
	st.add_index(12)
	st.add_index(13)
	st.add_index(14)
	st.add_index(12)
	st.add_index(14)
	st.add_index(15)
	
	# Top
	
	st.add_uv(Vector2(0, 0))
	st.add_vertex(Vector3(-halfsize.x, halfsize.y, halfsize.z))
	
	st.add_uv(Vector2(0, 1))
	st.add_vertex(Vector3(-halfsize.x, halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(1, 1))
	st.add_vertex(Vector3(halfsize.x, halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(1, 0))
	st.add_vertex(Vector3(halfsize.x, halfsize.y, halfsize.z))
	
	st.add_index(18)
	st.add_index(17)
	st.add_index(16)
	st.add_index(19)
	st.add_index(18)
	st.add_index(16)
	
	# Bottom
	
	st.add_uv(Vector2(0, 0))
	st.add_vertex(Vector3(-halfsize.x, -halfsize.y, halfsize.z))
	
	st.add_uv(Vector2(0, 1))
	st.add_vertex(Vector3(-halfsize.x, -halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(1, 1))
	st.add_vertex(Vector3(halfsize.x, -halfsize.y, -halfsize.z))
	
	st.add_uv(Vector2(1, 0))
	st.add_vertex(Vector3(halfsize.x, -halfsize.y, halfsize.z))
	
	st.add_index(20)
	st.add_index(21)
	st.add_index(22)
	st.add_index(20)
	st.add_index(22)
	st.add_index(23)
	
	st.generate_normals()
	
	mesh = st.commit()
