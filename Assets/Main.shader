shader_type spatial;

varying vec3 byc;

void vertex()
{
	int Bitval = int(UV2.x);
	byc = vec3(float(Bitval & 1), float(Bitval & (1 << 1)), float(Bitval & (1 << 2)));
}

void fragment()
{
	if(any(lessThan(byc, vec3(0.2)))){
	    ALBEDO = vec3(0.0, 0.0, 0.0);
	}
	else{
	    ALBEDO = vec3(0.5, 0.5, 0.5);
	}
}